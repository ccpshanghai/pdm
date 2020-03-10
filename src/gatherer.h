#pragma once

#include "../include/pdm_data.h"

#ifdef _WIN32
#include <intrin.h>
#endif

namespace PDM
{
	class CPUID
	{
		uint32_t regs[4];

	public:
		explicit CPUID(unsigned funcId, unsigned subFuncId = 0)
		{
		#ifdef _WIN32
			__cpuidex(reinterpret_cast<int*>(regs), static_cast<int>(funcId), static_cast<int>(subFuncId));
		#else
			asm volatile
			(
				"cpuid" :
				"=a" (regs[0]),
				"=b" (regs[1]),
				"=c" (regs[2]),
				"=d" (regs[3]) :
				"a" (funcId),
				"c" (subFuncId)
			);
			// ECX is set to zero for CPUID function 4
		#endif
		}

		const uint32_t &EAX() const { return regs[0]; }
		const uint32_t &EBX() const { return regs[1]; }
		const uint32_t &ECX() const { return regs[2]; }
		const uint32_t &EDX() const { return regs[3]; }
	};

	PDMData GatherData();
}
