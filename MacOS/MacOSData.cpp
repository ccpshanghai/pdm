#include "MacOSData.h"

#ifdef __APPLE__

namespace PDM
{
	Bitness GetOSBitnessInternal()
	{
		return Bitness::BITNESS_64;
	}

	OS GetOSType()
	{
		return OS::MACOS;
	}

	std::string GetOSName()
	{
		return "";
	}

	std::string GetOSMajorVersion()
	{
		return "";
	}

	std::string GetOSMinorVersion()
	{
		return "";
	}

	std::string GetOSBuildNumber()
	{
		return "";
	}

	std::string GetOSKernelVersion()
	{
		return "";
	}

	std::string GetMachineName()
	{
		return "";
	}

	std::string GetUsername()
	{
		return "";
	}

	unsigned GetScreenCount()
	{
		return 0;
	}

	uint64_t GetTotalMemory()
	{
		return 0;
	}

	std::string GetMachineUuid()
	{
		return "";
	}

	bool IsRunningVM()
	{
		return false;
	}

	SubItem GetMacOSSubItems()
	{
		return {};
	}
}

#else

PDM::SubItem GetMacOSSubItems()
{
	return {};
}

#endif
