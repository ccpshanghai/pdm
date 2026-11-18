// Copyright © 2026 CCP ehf.

// iOS system-inventory backend. Counterpart to src/macos/macos_data.mm, which guards itself
// on TARGET_OS_OSX and therefore contributes nothing here. SRC_FILES is flat and each source
// guards itself, so this file is compiled on every target and must gate itself the same way.
#if defined( __APPLE__ )
#include <TargetConditionals.h>
#endif

#if defined( __APPLE__ ) && TARGET_OS_IPHONE

#include "../../include/pdm.h"
#include "../utilities.h"
#include "../defines.h"

#include <string>
#include <vector>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>

namespace PDM
{
    // Identical to the macOS implementations: sysctlbyname is plain BSD and behaves the same
    // on iOS. Shared code (cpu_extensions.h) calls these, so each backend must define them.
    std::string GetOSString(const char* name)
    {
        char buffer[1024] = { 0 };
        size_t size = sizeof(buffer);
        sysctlbyname(name, buffer, &size, nullptr, 0);

        return buffer;
    }

    uint64_t GetOSInteger(const char* name)
    {
        uint64_t result = 0;
        size_t size = sizeof(result);
        sysctlbyname(name, &result, &size, nullptr, 0);

        return result;
    }

    // PDM::OS has no IOS enumerator, and adding one would touch a serialised public enum plus
    // the exhaustive switch in gatherer.cpp. iOS is Darwin and every consumer already handles
    // MACOS, so this reports MACOS deliberately. Callers that must tell the two apart can use
    // GetHardwareModel, which returns something like "iPhone15,2".
    OS GetOSType()
    {
        return OS::MACOS;
    }

    std::string GetOSName()
    {
        return [[[NSProcessInfo processInfo] operatingSystemVersionString] UTF8String];
    }

    // macOS shells out to sw_vers; popen does not exist on iOS, and NSProcessInfo carries the
    // same three numbers directly.
    std::string GetOSMajorVersion()
    {
        return std::to_string( [[NSProcessInfo processInfo] operatingSystemVersion].majorVersion );
    }

    std::string GetOSMinorVersion()
    {
        return std::to_string( [[NSProcessInfo processInfo] operatingSystemVersion].minorVersion );
    }

    std::string GetOSBuildNumber()
    {
        return std::to_string( [[NSProcessInfo processInfo] operatingSystemVersion].patchVersion );
    }

    std::string GetOSKernelVersion()
    {
        return GetOSString("kern.osrelease");
    }

    // hw.model is a board id on iOS ("D74AP"); hw.machine is the product identifier
    // ("iPhone15,2"), which is what a reader of this field expects.
    std::string GetHardwareModel()
    {
        auto machine = GetOSString("hw.machine");
        return machine.empty() ? GetOSString("hw.model") : machine;
    }

    std::string GetMachineName()
    {
        return [[[NSProcessInfo processInfo] hostName] UTF8String];
    }

    // iOS has no user accounts, and NSProcessInfo.userName is unavailable there.
    std::string GetUsername()
    {
        return {};
    }

    std::string GetUserLocale()
    {
        return [[[NSLocale currentLocale] localeIdentifier] UTF8String];
    }

    uint32_t GetMonitorCount()
    {
        return static_cast<uint32_t>( [[UIScreen screens] count] );
    }

    uint64_t GetTotalMemory()
    {
        return [[NSProcessInfo processInfo] physicalMemory];
    }

    bool IsRemoteSession()
    {
        return false;
    }

    // No Rosetta on iOS.
    bool IsRosetta()
    {
        return false;
    }

    // iOS exposes no CPU frequency: there is no hw.cpufrequency sysctl on Apple silicon, and
    // the macOS path shells out to arch/sysctl, which needs popen.
    uint32_t GetCPUFrequency()
    {
        return 0;
    }

    // Every iOS device has an internal battery. Reading its charge state requires enabling
    // batteryMonitoring, a global side effect on UIDevice, so this reports presence only --
    // which is all BatteryStatus models.
    BatteryStatus GetBatteryStatus()
    {
        return BatteryStatus::DETECTED;
    }

    // iOS exposes no hardware UUID: the IORegistry kIOPlatformUUIDKey path used on macOS needs
    // IOKit APIs apps cannot reach. identifierForVendor is the sanctioned substitute, with two
    // caveats -- it is per-vendor rather than per-device, and it is regenerated once the last
    // app from that vendor is uninstalled. Same choice as BlueSysInfo.mm in the blue fork.
    std::string GetMachineUuidString()
    {
        NSUUID* uuid = [[UIDevice currentDevice] identifierForVendor];
        return uuid ? std::string( [[uuid UUIDString] UTF8String] ) : std::string();
    }

    std::vector<MonitorInfo> GetMonitorsInfo()
    {
        std::vector<MonitorInfo> monitors;
        for( UIScreen* screen in [UIScreen screens] )
        {
            CGRect bounds = [screen nativeBounds];
            MonitorInfo info;
            info.name = "Built-in Display";
            info.width = static_cast<uint32_t>( bounds.size.width );
            info.height = static_cast<uint32_t>( bounds.size.height );
            // iOS reports no per-display bit depth and offers no API to distinguish; every
            // supported device is at least 8 bits per channel.
            info.bitsPerColor = 8;
            info.refreshRate = static_cast<uint32_t>( [screen maximumFramesPerSecond] );
            info.dpiScaling = static_cast<uint32_t>( [screen nativeScale] * 100.0 );
            monitors.push_back( info );
        }
        return monitors;
    }

    std::vector<GPUInfo> GetGPUInfo()
    {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if( !device ) return {};

        GPUInfo info;
        info.description = [[device name] UTF8String];
        // iOS GPUs are integrated and share system memory, so this is the budget Metal
        // reports rather than dedicated VRAM, which does not exist.
        info.memory = [device recommendedMaxWorkingSetSize];
        // vendorID, deviceID, revision and the driver fields are left default: Metal exposes
        // no PCI identity, and the driver ships as part of the OS rather than separately.
        return { info };
    }

    bool GetMetalSupported()
    {
        return MTLCreateSystemDefaultDevice() != nil;
    }

    // Vulkan is not an iOS API. Reaching it would mean shipping MoltenVK, which is a product
    // decision rather than something to detect here.
    VulkanProperties GetVulkanProperties()
    {
        VulkanProperties properties;
        properties.support = VulkanSupport::UNSUPPORTED;
        return properties;
    }

    // macOS walks the IOKit ethernet registry. getifaddrs is plain BSD, available on iOS, and
    // yields the same interface name and hardware address for every link-layer interface.
    std::vector<NetworkAdapterInfo> GetNetworkAdapterInfo()
    {
        struct ifaddrs* interfaces = nullptr;
        if( getifaddrs( &interfaces ) != 0 ) return {};

        std::vector<NetworkAdapterInfo> adapters;
        for( struct ifaddrs* it = interfaces; it != nullptr; it = it->ifa_next )
        {
            if( !it->ifa_addr || it->ifa_addr->sa_family != AF_LINK ) continue;
            if( it->ifa_flags & IFF_LOOPBACK ) continue;

            auto* dl = reinterpret_cast<struct sockaddr_dl*>( it->ifa_addr );
            if( dl->sdl_alen != 6 ) continue;
            auto* mac = reinterpret_cast<const uint8_t*>( LLADDR( dl ) );

            NetworkAdapterInfo adapter;
            adapter.name = it->ifa_name;
            adapter.macAddress.assign( mac, mac + dl->sdl_alen );

            char text[18] = { 0 };
            snprintf( text, sizeof(text), "%02x:%02x:%02x:%02x:%02x:%02x",
                      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
            adapter.macAddressString = text;

            adapters.push_back( adapter );
        }

        freeifaddrs( interfaces );
        return adapters;
    }

    // macOS parses `diskutil info -plist disk0`, which needs popen. NSFileManager reports the
    // same total for the volume the app lives on, and storage is always flash on iOS.
    std::vector<HardDriveInfo> GetHardDriveInfo()
    {
        NSString* home = NSHomeDirectory();
        NSDictionary* attributes = [[NSFileManager defaultManager] attributesOfFileSystemForPath:home error:nil];
        if( !attributes ) return {};

        NSNumber* size = attributes[NSFileSystemSize];
        if( !size ) return {};

        return {{
            "Internal Storage",
            HardDriveInfo::HardDriveType::SSD,
            static_cast<uint64_t>( [size unsignedLongLongValue] )
        }};
    }
}

#endif
