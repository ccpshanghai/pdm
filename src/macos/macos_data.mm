#include "macos_data.h"

#if __APPLE__

#include <sys/sysctl.h>
#include <sys/utsname.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

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

namespace PDM
{
	Bitness GetOSBitnessInternal()
	{
		struct utsname un;
		int res = uname(&un);
		if (res >= 0)
		{
			std::string machine{un.machine};
			if (machine == "x86_64") return Bitness::BITNESS_64;
			if (machine == "i386"  ) return Bitness::BITNESS_32;
		}
		
		return Bitness::BITNESS_UNKNOWN;
	}

	OS GetOSType()
	{
		return OS::MACOS;
	}

	std::string GetOSName()
	{
		return [[[NSProcessInfo processInfo] operatingSystemVersionString] UTF8String];
	}

	std::string GetOSMajorVersion()
	{
		return std::to_string([[NSProcessInfo processInfo] operatingSystemVersion].majorVersion);
	}

	std::string GetOSMinorVersion()
	{
		return std::to_string([[NSProcessInfo processInfo] operatingSystemVersion].minorVersion);
	}

	std::string GetOSBuildNumber()
	{
		return std::to_string([[NSProcessInfo processInfo] operatingSystemVersion].patchVersion);
	}

	std::string GetOSKernelVersion()
	{
		return GetOSString("kern.osrelease");
	}

	std::string GetMachineName()
	{
		return [[[NSProcessInfo processInfo] hostName] UTF8String];
	}

	std::string GetUsername()
	{
		return [[[NSProcessInfo processInfo] userName] UTF8String];
	}

	unsigned GetScreenCount()
	{
		return [[NSScreen screens] count];
	}

	uint64_t GetTotalMemory()
	{
		return [[NSProcessInfo processInfo] physicalMemory];
	}

	std::string GetMachineUuid()
	{
		char buffer[128] = { 0 };
		io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
		CFStringRef uuidCf = static_cast<CFStringRef>(IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0));
		IOObjectRelease(ioRegistryRoot);
		CFStringGetCString(uuidCf, buffer, sizeof(buffer), kCFStringEncodingMacRoman);
		CFRelease(uuidCf);
		
		return buffer;
	}

	SubItem GetMacOSSubItems()
	{
		return {};
	}
}

#else

namespace PDM
{
	SubItem GetMacOSSubItems()
	{
		return {};
	}
}

#endif
