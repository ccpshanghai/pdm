// Copyright © 2026 CCP ehf.

// Desktop Linux system-inventory backend. android_data.cpp is empty unless
// __ANDROID__, and wine.cpp only supplies the Wine stubs outside _WIN32.
// This file fills the symbols gatherer.cpp calls and names the Vulkan
// adapters the loader actually enumerates, with a discrete NVIDIA device
// ordered first.

#if defined( __linux__ ) && !defined( __ANDROID__ )

#include "../../include/pdm.h"
#include "../utilities.h"
#include "../defines.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <dirent.h>
#include <dlfcn.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace
{
    std::string ReadFirstLine( const char* path )
    {
        FILE* file = fopen( path, "r" );
        if( !file ) return {};

        char buffer[512] = { 0 };
        char* line = fgets( buffer, sizeof( buffer ), file );
        fclose( file );
        if( !line ) return {};

        std::string result( buffer );
        while( !result.empty() && ( result.back() == '\n' || result.back() == '\r' ) )
        {
            result.pop_back();
        }
        return result;
    }

    std::string OsRelease( const char* key )
    {
        FILE* file = fopen( "/etc/os-release", "r" );
        if( !file ) return {};

        const std::string prefix = std::string( key ) + "=";
        char line[512] = { 0 };
        std::string value;
        while( fgets( line, sizeof( line ), file ) )
        {
            if( strncmp( line, prefix.c_str(), prefix.size() ) != 0 ) continue;
            value = line + prefix.size();
            while( !value.empty() && ( value.back() == '\n' || value.back() == '\r' ) )
            {
                value.pop_back();
            }
            if( value.size() >= 2 && value.front() == '"' && value.back() == '"' )
            {
                value = value.substr( 1, value.size() - 2 );
            }
            break;
        }

        fclose( file );
        return value;
    }

    std::string VersionPart( const std::string& version, size_t part )
    {
        size_t start = 0;
        for( size_t i = 0; i <= part; ++i )
        {
            auto dot = version.find( '.', start );
            auto piece = version.substr( start, dot == std::string::npos ? std::string::npos : dot - start );
            if( i == part ) return piece.empty() ? std::string( "0" ) : piece;
            if( dot == std::string::npos ) return "0";
            start = dot + 1;
        }
        return "0";
    }

    std::string FormatPackedVersion( uint32_t version )
    {
        char text[64] = { 0 };
        snprintf( text, sizeof( text ), "%u.%u.%u",
                  VK_VERSION_MAJOR( version ),
                  VK_VERSION_MINOR( version ),
                  VK_VERSION_PATCH( version ) );
        return text;
    }

    // NVIDIA packs its driver version differently from the Vulkan api version.
    std::string FormatDriverVersion( uint32_t vendor, uint32_t version )
    {
        if( vendor == 0x10deu )
        {
            char text[64] = { 0 };
            snprintf( text, sizeof( text ), "%u.%u.%u",
                      ( version >> 22 ) & 0x3ffu,
                      ( version >> 12 ) & 0x3ffu,
                      version & 0xfffu );
            return text;
        }
        return FormatPackedVersion( version );
    }

    const char* DriverVendorName( uint32_t vendor, const std::string& description )
    {
        if( vendor == 0x10deu ) return "NVIDIA";
        if( vendor == 0x1002u ) return "AMD";
        if( vendor == 0x8086u ) return "Intel";
        if( description.find( "llvmpipe" ) != std::string::npos ) return "Mesa";
        return "";
    }

    uint32_t PciRevision( uint32_t vendor, uint32_t device )
    {
        DIR* directory = opendir( "/sys/bus/pci/devices" );
        if( !directory ) return 0;

        uint32_t revision = 0;
        while( struct dirent* entry = readdir( directory ) )
        {
            if( entry->d_name[0] == '.' ) continue;

            std::string base = std::string( "/sys/bus/pci/devices/" ) + entry->d_name;
            auto vendorText = ReadFirstLine( ( base + "/vendor" ).c_str() );
            auto deviceText = ReadFirstLine( ( base + "/device" ).c_str() );
            if( strtoul( vendorText.c_str(), nullptr, 0 ) != vendor ) continue;
            if( strtoul( deviceText.c_str(), nullptr, 0 ) != device ) continue;

            revision = static_cast<uint32_t>( strtoul( ReadFirstLine( ( base + "/revision" ).c_str() ).c_str(), nullptr, 0 ) );
            break;
        }

        closedir( directory );
        return revision;
    }

    uint64_t DeviceLocalBytes( PFN_vkGetPhysicalDeviceMemoryProperties getMemory, VkPhysicalDevice device )
    {
        VkPhysicalDeviceMemoryProperties memory{};
        getMemory( device, &memory );

        uint64_t bytes = 0;
        for( uint32_t i = 0; i < memory.memoryHeapCount; ++i )
        {
            if( memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT )
            {
                bytes += memory.memoryHeaps[i].size;
            }
        }
        return bytes;
    }

    int AdapterRank( VkPhysicalDeviceType type, uint32_t vendor )
    {
        const bool nvidia = vendor == 0x10deu;
        const bool discrete = type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if( nvidia && discrete ) return 0;
        if( discrete ) return 1;
        if( nvidia ) return 2;
        return 3;
    }

    struct VulkanSnapshot
    {
        PDM::VulkanProperties properties;
        std::vector<PDM::GPUInfo> gpus;
    };

    VulkanSnapshot QueryVulkan()
    {
        VulkanSnapshot snapshot;
        snapshot.properties.support = PDM::VulkanSupport::UNSUPPORTED;

        void* loader = dlopen( "libvulkan.so.1", RTLD_NOW | RTLD_LOCAL );
        if( !loader )
        {
            loader = dlopen( "libvulkan.so", RTLD_NOW | RTLD_LOCAL );
        }
        if( !loader ) return snapshot;

        auto enumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>( dlsym( loader, "vkEnumerateInstanceVersion" ) );
        auto createInstance = reinterpret_cast<PFN_vkCreateInstance>( dlsym( loader, "vkCreateInstance" ) );
        auto destroyInstance = reinterpret_cast<PFN_vkDestroyInstance>( dlsym( loader, "vkDestroyInstance" ) );
        auto enumerateDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>( dlsym( loader, "vkEnumeratePhysicalDevices" ) );
        auto getProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>( dlsym( loader, "vkGetPhysicalDeviceProperties" ) );
        auto getMemory = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>( dlsym( loader, "vkGetPhysicalDeviceMemoryProperties" ) );

        if( !createInstance || !destroyInstance || !enumerateDevices || !getProperties || !getMemory )
        {
            dlclose( loader );
            return snapshot;
        }

        uint32_t apiVersion = VK_API_VERSION_1_0;
        if( enumerateInstanceVersion )
        {
            enumerateInstanceVersion( &apiVersion );
        }

        VkApplicationInfo application{};
        application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application.pApplicationName = "pdm";
        application.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &application;

        VkInstance instance = VK_NULL_HANDLE;
        if( createInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS || !instance )
        {
            dlclose( loader );
            return snapshot;
        }

        snapshot.properties.support = PDM::VulkanSupport::SUPPORTED;
        snapshot.properties.version = FormatPackedVersion( apiVersion );

        uint32_t deviceCount = 0;
        if( enumerateDevices( instance, &deviceCount, nullptr ) != VK_SUCCESS || deviceCount == 0 )
        {
            destroyInstance( instance, nullptr );
            dlclose( loader );
            return snapshot;
        }

        std::vector<VkPhysicalDevice> devices( deviceCount );
        if( enumerateDevices( instance, &deviceCount, devices.data() ) != VK_SUCCESS )
        {
            destroyInstance( instance, nullptr );
            dlclose( loader );
            return snapshot;
        }

        struct Ranked
        {
            int rank;
            PDM::GPUInfo info;
        };
        std::vector<Ranked> ranked;
        ranked.reserve( deviceCount );

        for( uint32_t i = 0; i < deviceCount; ++i )
        {
            VkPhysicalDeviceProperties properties{};
            getProperties( devices[i], &properties );

            PDM::GPUInfo info;
            info.description = properties.deviceName;
            info.vendorID = properties.vendorID;
            info.deviceID = properties.deviceID;
            info.revision = PciRevision( properties.vendorID, properties.deviceID );
            info.memory = DeviceLocalBytes( getMemory, devices[i] );
            info.driverVersionString = FormatDriverVersion( properties.vendorID, properties.driverVersion );
            info.driverVendor = DriverVendorName( properties.vendorID, info.description );
            ranked.push_back( { AdapterRank( properties.deviceType, properties.vendorID ), info } );
        }

        std::stable_sort( ranked.begin(), ranked.end(), []( const Ranked& a, const Ranked& b )
        {
            return a.rank < b.rank;
        } );

        snapshot.gpus.reserve( ranked.size() );
        for( const Ranked& entry : ranked )
        {
            snapshot.gpus.push_back( entry.info );
        }

        destroyInstance( instance, nullptr );
        dlclose( loader );
        return snapshot;
    }

    bool StartsWith( const std::string& text, const char* prefix )
    {
        return text.compare( 0, strlen( prefix ), prefix ) == 0;
    }
}

namespace PDM
{
    // PDM::OS has no LINUX enumerator. Adding one would change the serialised
    // public enum and the switch in gatherer.cpp. UNKNOWN is the same answer
    // the Android backend gives; GetOSName carries "Linux".
    OS GetOSType()
    {
        return OS::UNKNOWN;
    }

    std::string GetOSName()
    {
        auto pretty = OsRelease( "PRETTY_NAME" );
        if( !pretty.empty() ) return pretty;

        struct utsname info;
        if( uname( &info ) != 0 ) return "Linux";
        return std::string( "Linux " ) + info.release;
    }

    std::string GetOSMajorVersion()
    {
        return VersionPart( OsRelease( "VERSION_ID" ), 0 );
    }

    std::string GetOSMinorVersion()
    {
        return VersionPart( OsRelease( "VERSION_ID" ), 1 );
    }

    std::string GetOSBuildNumber()
    {
        auto build = OsRelease( "BUILD_ID" );
        return build.empty() ? OsRelease( "VERSION_ID" ) : build;
    }

    std::string GetOSKernelVersion()
    {
        struct utsname info;
        if( uname( &info ) != 0 ) return {};
        return info.release;
    }

    std::string GetHardwareModel()
    {
        auto product = ReadFirstLine( "/sys/devices/virtual/dmi/id/product_name" );
        if( !product.empty() ) return product;
        return ReadFirstLine( "/sys/devices/virtual/dmi/id/board_name" );
    }

    std::string GetMachineName()
    {
        char host[256] = { 0 };
        return gethostname( host, sizeof( host ) - 1 ) == 0 ? std::string( host ) : std::string();
    }

    std::string GetUsername()
    {
        if( const passwd* entry = getpwuid( geteuid() ) )
        {
            if( entry->pw_name ) return entry->pw_name;
        }
        const char* user = getenv( "USER" );
        return user ? user : std::string();
    }

    std::string GetUserLocale()
    {
        const char* locale = getenv( "LC_ALL" );
        if( locale && locale[0] ) return locale;
        locale = getenv( "LANG" );
        return locale ? locale : std::string();
    }

    std::string GetMachineUuidString()
    {
        return ReadFirstLine( "/etc/machine-id" );
    }

    uint32_t GetCPUFrequency()
    {
        auto khz = ReadFirstLine( "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq" );
        if( !khz.empty() )
        {
            return static_cast<uint32_t>( std::atoll( khz.c_str() ) / 1000 );
        }

        FILE* file = fopen( "/proc/cpuinfo", "r" );
        if( !file ) return 0;
        char line[256] = { 0 };
        uint32_t mhz = 0;
        while( fgets( line, sizeof( line ), file ) )
        {
            if( strncmp( line, "cpu MHz", 7 ) != 0 ) continue;
            const char* colon = strchr( line, ':' );
            if( !colon ) continue;
            mhz = static_cast<uint32_t>( std::atof( colon + 1 ) );
            break;
        }
        fclose( file );
        return mhz;
    }

    uint64_t GetTotalMemory()
    {
        long pages = sysconf( _SC_PHYS_PAGES );
        long pageSize = sysconf( _SC_PAGE_SIZE );
        if( pages <= 0 || pageSize <= 0 ) return 0;
        return static_cast<uint64_t>( pages ) * static_cast<uint64_t>( pageSize );
    }

    bool IsRemoteSession()
    {
        return getenv( "SSH_CONNECTION" ) != nullptr || getenv( "SSH_CLIENT" ) != nullptr;
    }

    bool IsRosetta()
    {
        return false;
    }

    Bitness GetOSBitnessInternal()
    {
        return sizeof( void* ) == 8 ? Bitness::BITNESS_64 : Bitness::BITNESS_32;
    }

    BatteryStatus GetBatteryStatus()
    {
        DIR* directory = opendir( "/sys/class/power_supply" );
        if( !directory ) return BatteryStatus::UNKNOWN;

        BatteryStatus status = BatteryStatus::NOT_DETECTED;
        while( struct dirent* entry = readdir( directory ) )
        {
            if( entry->d_name[0] == '.' ) continue;

            std::string typePath = std::string( "/sys/class/power_supply/" ) + entry->d_name + "/type";
            if( ReadFirstLine( typePath.c_str() ) == "Battery" )
            {
                status = BatteryStatus::DETECTED;
                break;
            }
        }

        closedir( directory );
        return status;
    }

    bool GetMetalSupported()
    {
        return false;
    }

    VulkanProperties GetVulkanProperties()
    {
        return QueryVulkan().properties;
    }

    std::vector<GPUInfo> GetGPUInfo()
    {
        return QueryVulkan().gpus;
    }

    std::vector<MonitorInfo> GetMonitorsInfo()
    {
        DIR* directory = opendir( "/sys/class/drm" );
        if( !directory ) return {};

        std::vector<MonitorInfo> monitors;
        while( struct dirent* entry = readdir( directory ) )
        {
            std::string name = entry->d_name;
            if( name.find( '-' ) == std::string::npos ) continue;

            std::string base = std::string( "/sys/class/drm/" ) + name;
            if( ReadFirstLine( ( base + "/status" ).c_str() ) != "connected" ) continue;

            MonitorInfo monitor;
            monitor.name = name;
            auto modes = ReadFirstLine( ( base + "/modes" ).c_str() );
            auto split = modes.find( 'x' );
            if( split != std::string::npos )
            {
                monitor.width = static_cast<uint32_t>( std::atoi( modes.c_str() ) );
                monitor.height = static_cast<uint32_t>( std::atoi( modes.c_str() + split + 1 ) );
            }
            monitors.push_back( monitor );
        }

        closedir( directory );
        return monitors;
    }

    uint32_t GetMonitorCount()
    {
        return static_cast<uint32_t>( GetMonitorsInfo().size() );
    }

    std::vector<NetworkAdapterInfo> GetNetworkAdapterInfo()
    {
        struct ifaddrs* interfaces = nullptr;
        if( getifaddrs( &interfaces ) != 0 ) return {};

        std::vector<NetworkAdapterInfo> adapters;
        for( struct ifaddrs* it = interfaces; it != nullptr; it = it->ifa_next )
        {
            if( !it->ifa_addr || it->ifa_addr->sa_family != AF_PACKET ) continue;
            if( it->ifa_flags & IFF_LOOPBACK ) continue;

            auto* ll = reinterpret_cast<struct sockaddr_ll*>( it->ifa_addr );
            if( ll->sll_halen != 6 ) continue;

            NetworkAdapterInfo adapter;
            adapter.name = it->ifa_name;
            adapter.macAddress.assign( ll->sll_addr, ll->sll_addr + ll->sll_halen );

            char text[18] = { 0 };
            snprintf( text, sizeof( text ), "%02x:%02x:%02x:%02x:%02x:%02x",
                      ll->sll_addr[0], ll->sll_addr[1], ll->sll_addr[2],
                      ll->sll_addr[3], ll->sll_addr[4], ll->sll_addr[5] );
            adapter.macAddressString = text;
            adapters.push_back( adapter );
        }

        freeifaddrs( interfaces );
        return adapters;
    }

    std::vector<HardDriveInfo> GetHardDriveInfo()
    {
        std::vector<HardDriveInfo> drives;
        DIR* directory = opendir( "/sys/block" );
        if( directory )
        {
            while( struct dirent* entry = readdir( directory ) )
            {
                std::string name = entry->d_name;
                if( name.empty() || name[0] == '.' ) continue;
                if( StartsWith( name, "loop" ) || StartsWith( name, "ram" ) || StartsWith( name, "zram" ) ||
                    StartsWith( name, "fd" ) || StartsWith( name, "dm-" ) || StartsWith( name, "sr" ) ||
                    StartsWith( name, "nbd" ) )
                {
                    continue;
                }

                std::string base = std::string( "/sys/block/" ) + name;
                auto sectors = ReadFirstLine( ( base + "/size" ).c_str() );
                if( sectors.empty() ) continue;

                auto bytes = static_cast<uint64_t>( std::atoll( sectors.c_str() ) ) * 512ull;
                if( bytes == 0 ) continue;

                auto rotational = ReadFirstLine( ( base + "/queue/rotational" ).c_str() );
                auto type = rotational == "0" ? HardDriveInfo::HardDriveType::SSD
                            : rotational == "1" ? HardDriveInfo::HardDriveType::HDD
                            : HardDriveInfo::HardDriveType::UNKNOWN;
                drives.push_back( { name, type, bytes } );
            }
            closedir( directory );
        }

        if( !drives.empty() ) return drives;

        struct statvfs stats;
        if( statvfs( "/", &stats ) != 0 ) return {};
        auto total = static_cast<uint64_t>( stats.f_blocks ) * static_cast<uint64_t>( stats.f_frsize );
        if( total == 0 ) return {};
        return {{ "root", HardDriveInfo::HardDriveType::UNKNOWN, total }};
    }
}

#endif
