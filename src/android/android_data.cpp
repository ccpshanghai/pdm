// Copyright © 2026 CCP ehf.

// Android system-inventory backend. Counterpart to src/macos/macos_data.mm and
// src/ios/ios_data.mm. SRC_FILES is flat and each source guards itself, so this file is
// compiled on every target and gates itself the same way.
//
// Android is more restricted than iOS for this kind of data. Notably absent, and left empty
// with a note at each site rather than faked: display information (needs Java or
// SurfaceFlinger), GPU identity (needs a live EGL or Vulkan context), and any stable device
// identifier (ro.serialno became privileged in Android 10, and there is no native equivalent
// of identifierForVendor).
//
// bionic has no sysctlbyname, so none of the macOS/iOS sysctl helpers carry over. System
// properties take their place. GetOSString/GetOSInteger are deliberately not defined here:
// nothing in the Android link needs them.

#if defined( __ANDROID__ )

#include "../../include/pdm.h"
#include "../utilities.h"
#include "../defines.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/system_properties.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>

namespace
{
    // The property API writes at most PROP_VALUE_MAX bytes including the terminator.
    std::string GetSystemProperty( const char* name )
    {
        char buffer[PROP_VALUE_MAX] = { 0 };
        int length = __system_property_get( name, buffer );
        return length > 0 ? std::string( buffer, static_cast<size_t>( length ) ) : std::string();
    }

    std::string ReadFirstLine( const char* path )
    {
        FILE* file = fopen( path, "r" );
        if( !file ) return {};

        char buffer[256] = { 0 };
        char* line = fgets( buffer, sizeof( buffer ), file );
        fclose( file );
        if( !line ) return {};

        std::string result( buffer );
        while( !result.empty() && ( result.back() == 10 || result.back() == 13 ) )
        {
            result.pop_back();
        }
        return result;
    }

    // ro.build.version.release is "14", "13.0" or occasionally "4.4.4". Split on dots and hand
    // back the requested component, or "0" when the release string does not carry one.
    std::string GetReleasePart( size_t part )
    {
        auto release = GetSystemProperty( "ro.build.version.release" );
        size_t start = 0;
        for( size_t i = 0; i <= part; ++i )
        {
            auto dot = release.find( '.', start );
            auto piece = release.substr( start, dot == std::string::npos ? std::string::npos : dot - start );
            if( i == part ) return piece.empty() ? std::string( "0" ) : piece;
            if( dot == std::string::npos ) return "0";
            start = dot + 1;
        }
        return "0";
    }
}

namespace PDM
{
    // PDM::OS has no ANDROID enumerator, and adding one would touch a serialised public enum
    // plus the exhaustive switch in gatherer.cpp. Unlike iOS, which is at least Darwin and can
    // honestly report MACOS, Android resembles none of the existing values -- so UNKNOWN is the
    // truthful answer. GetOSName and GetHardwareModel carry the real identity.
    OS GetOSType()
    {
        return OS::UNKNOWN;
    }

    std::string GetOSName()
    {
        auto release = GetSystemProperty( "ro.build.version.release" );
        auto sdk = GetSystemProperty( "ro.build.version.sdk" );

        std::string name = "Android";
        if( !release.empty() ) name += " " + release;
        if( !sdk.empty() )     name += " (API " + sdk + ")";
        return name;
    }

    std::string GetOSMajorVersion()
    {
        return GetReleasePart( 0 );
    }

    std::string GetOSMinorVersion()
    {
        return GetReleasePart( 1 );
    }

    // The closest analogue to a build number. ro.build.id is the platform build tag; the
    // incremental property is the vendor build counter and is noisier.
    std::string GetOSBuildNumber()
    {
        auto id = GetSystemProperty( "ro.build.id" );
        return id.empty() ? GetSystemProperty( "ro.build.version.incremental" ) : id;
    }

    std::string GetOSKernelVersion()
    {
        struct utsname info;
        if( uname( &info ) != 0 ) return {};
        return info.release;
    }

    std::string GetHardwareModel()
    {
        auto model = GetSystemProperty( "ro.product.model" );
        return model.empty() ? GetSystemProperty( "ro.product.device" ) : model;
    }

    std::string GetMachineName()
    {
        auto device = GetSystemProperty( "ro.product.device" );
        if( !device.empty() ) return device;

        char host[256] = { 0 };
        return gethostname( host, sizeof( host ) - 1 ) == 0 ? std::string( host ) : std::string();
    }

    // Android exposes no user accounts to native code.
    std::string GetUsername()
    {
        return {};
    }

    std::string GetUserLocale()
    {
        auto locale = GetSystemProperty( "persist.sys.locale" );
        return locale.empty() ? GetSystemProperty( "ro.product.locale" ) : locale;
    }

    // There is no device identifier available to native code. ro.serialno became privileged in
    // Android 10, ANDROID_ID needs a JNI round trip through Settings.Secure, and neither is a
    // hardware id in the sense macOS kIOPlatformUUIDKey is. Left empty rather than invented.
    std::string GetMachineUuidString()
    {
        return {};
    }

    // Maximum rather than current frequency, matching what the macOS backend reports. The
    // cpufreq node is absent on some devices and readable without permissions where present.
    uint32_t GetCPUFrequency()
    {
        auto khz = ReadFirstLine( "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq" );
        if( khz.empty() ) return 0;
        return static_cast<uint32_t>( std::atoll( khz.c_str() ) / 1000 );
    }

    // _SC_PHYS_PAGES counts the pages the kernel manages, which is what /proc/meminfo calls
    // MemTotal and is a little under what the hardware holds -- the same relationship
    // ActivityManager.MemoryInfo.totalMem has to the number printed on the box.
    uint64_t GetTotalMemory()
    {
        long pages = sysconf( _SC_PHYS_PAGES );
        long pageSize = sysconf( _SC_PAGE_SIZE );
        if( pages <= 0 || pageSize <= 0 ) return 0;

        return static_cast<uint64_t>( pages ) * static_cast<uint64_t>( pageSize );
    }

    bool IsRemoteSession()
    {
        return false;
    }

    // Rosetta is an Apple technology.
    bool IsRosetta()
    {
        return false;
    }

    // Unlike iOS, where every device has a battery and the answer is a constant, Android also
    // runs on TV boxes, head units and emulators that have none -- so there is a real question
    // here and a real place to ask it. /sys/class/power_supply is world-readable and each entry
    // names its own type; a "Battery" entry is what the framework itself looks for. UNKNOWN is
    // reserved for not being able to read the directory at all, which is a different answer
    // from reading it and finding no battery.
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

    // Metal is an Apple API.
    bool GetMetalSupported()
    {
        return false;
    }

    // Android does support Vulkan, and whether a given device has a driver is exactly what the
    // loader answers. Enumerating devices for properties would need an instance, which is more
    // than this call should cost, so this reports support only.
    VulkanProperties GetVulkanProperties()
    {
        VulkanProperties properties;

        void* loader = dlopen( "libvulkan.so", RTLD_NOW | RTLD_LOCAL );
        if( !loader )
        {
            loader = dlopen( "libvulkan.so.1", RTLD_NOW | RTLD_LOCAL );
        }

        properties.support = loader ? VulkanSupport::SUPPORTED : VulkanSupport::UNSUPPORTED;
        if( loader ) dlclose( loader );

        return properties;
    }

    // Requires a live EGL or Vulkan context to name the adapter, which this call cannot create.
    // The macOS and iOS backends get this from Metal without a context; there is no equivalent
    // on Android.
    std::vector<GPUInfo> GetGPUInfo()
    {
        return {};
    }

    // Deliberately 1 while GetMonitorsInfo below returns nothing, and the two disagree on
    // purpose: a phone or tablet always has its built-in display, so 0 would be a wrong fact
    // in the report, but describing that display needs DisplayManager and a JNI environment.
    // Do not read this as the length of the GetMonitorsInfo vector. External displays over
    // HDMI or Cast are not counted either, for the same reason.
    uint32_t GetMonitorCount()
    {
        return 1;
    }

    // Display metrics live in the Java layer (DisplayManager) or behind SurfaceFlinger; neither
    // is reachable from a plain native library without a JNI environment.
    std::vector<MonitorInfo> GetMonitorsInfo()
    {
        return {};
    }

    // getifaddrs is available from API 24. Note this differs from the iOS backend: Linux carries
    // hardware addresses in AF_PACKET / sockaddr_ll, not AF_LINK / sockaddr_dl.
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

    // The app data directory is the only path guaranteed readable. Its filesystem is the
    // internal one users see as device storage. Type is always flash, but Android exposes no
    // way to distinguish, so it stays UNKNOWN rather than claiming SSD.
    std::vector<HardDriveInfo> GetHardDriveInfo()
    {
        const char* candidates[] = { "/data", "/data/data", "/" };
        for( const char* path : candidates )
        {
            struct statvfs stats;
            if( statvfs( path, &stats ) != 0 ) continue;

            auto total = static_cast<uint64_t>( stats.f_blocks ) * static_cast<uint64_t>( stats.f_frsize );
            if( total == 0 ) continue;

            return {{ "Internal Storage", HardDriveInfo::HardDriveType::UNKNOWN, total }};
        }
        return {};
    }
}

#endif
