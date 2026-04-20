// Copyright © 2026 CCP ehf.

#pragma once

#include <bitset>
#include <array>
#include <string>
#include <vector>
#include <thread>

#if _WIN32
#include <intrin.h>
#endif

#include "utilities.h"

namespace PDM
{
    constexpr CPUArchitecture GetCPUArchitecture()
    {
#if _M_IX86 || __i386
		return CPUArchitecture::X86;
#elif _M_AMD64 || __amd64
		return CPUArchitecture::X86_64;
#elif _M_ARM || __arm__
		return CPUArchitecture::ARM;
#elif __aarch64__
		return CPUArchitecture::ARM64;
#else
		return CPUArchitecture::UNKNOWN;
#error Unknown CPU architecture
#endif
    }

// Assume x86
#if !__aarch64__
    constexpr auto HYPER_V_NAME = "Microsoft Hv";

    class CPUID
    {
        uint32_t regs[4];

    public:
        explicit CPUID(unsigned funcId)
        {
        #if _WIN32
            __cpuidex(reinterpret_cast<int*>(regs), static_cast<int>(funcId), 0);
        #else
            asm volatile
            (
                "cpuid" :
                "=a" (regs[0]),
                "=b" (regs[1]),
                "=c" (regs[2]),
                "=d" (regs[3]) :
                "a" (funcId),
                "c" (0)
            );
        #endif
        }

        const uint32_t& EAX() const { return regs[0]; }
        const uint32_t& EBX() const { return regs[1]; }
        const uint32_t& ECX() const { return regs[2]; }
        const uint32_t& EDX() const { return regs[3]; }

        bool has_data() const
        {
            return EAX() || EBX() || ECX() || EDX();
        }

        std::string register_to_string(uint32_t reg) const
        {
            return std::string(reinterpret_cast<const char*>(&reg), 4);
        }

        static std::string GetVendor()
        {
            CPUID id0(0);
            std::string vendor = id0.get_ebx_string() + id0.get_edx_string() + id0.get_ecx_string();
            trim(vendor);
            return vendor;
        }

        static std::string GetBrand()
        {
            std::string brand;

            if (CPUID(CPUID_FLAG).EAX() >= CPUID_MODEL_NAME_FLAG)
            {
                for (unsigned i = 0; i < 3; i++)
                {
                    CPUID id(CPUID_MODEL_NAME_OFFSET_FLAG + i);
                    brand += id.get_eax_string() + id.get_ebx_string() + id.get_ecx_string() + id.get_edx_string();
                }

                trim(brand);
            }

            return brand;
        }

        static Bitness GetBitness()
        {
            Bitness bitness;

            CPUID id8(CPUID::CPUID_FLAG);
            if (id8.EAX() >= CPUID::CPUID_EXTENDED_FLAG)
            {
                CPUID id81(CPUID::CPUID_EXTENDED_FLAG);
                bitness = id81.EDX() & CPUID::CPUID_X64_FLAG ? Bitness::BITNESS_64 : Bitness::BITNESS_32;
            }
            else
            {
                bitness = Bitness::BITNESS_32;
            }

            return bitness;
        }

        static std::tuple<int32_t, int32_t> GetCPUModelStepping()
        {
            int32_t model = 0;
            int32_t stepping = 0;

            if (CPUID(0).EAX() > 0)
            {
                CPUID id1(1);
                model = (id1.EAX() >> 4) & 0xf;
                stepping = id1.EAX() & 0xf;
            }

            return {model, stepping};
        }

        static int32_t GetModel()
        {
            auto [model, stepping] = GetCPUModelStepping();
            return model;
        }

        static int32_t GetStepping()
        {
            auto [model, stepping] = GetCPUModelStepping();
            return stepping;
        }

        std::string get_eax_string() const { return register_to_string(EAX()); }
        std::string get_ebx_string() const { return register_to_string(EBX()); }
        std::string get_ecx_string() const { return register_to_string(ECX()); }
        std::string get_edx_string() const { return register_to_string(EDX()); }

        static constexpr uint32_t CPUID_FLAG                   = 0x80000000;
        static constexpr uint32_t CPUID_EXTENDED_FLAG          = 0x80000001;
        static constexpr uint32_t CPUID_X64_FLAG               = 0x20000000;
        static constexpr uint32_t CPUID_MODEL_NAME_FLAG        = 0x80000004;
        static constexpr uint32_t CPUID_MODEL_NAME_OFFSET_FLAG = 0x80000002;
        static constexpr uint32_t HYPERVISOR_PRESENT_FLAG      = 0x80000000;
        static constexpr uint32_t HYPERVISOR_INFO_FLAG         = 0x40000000;
    };

	bool HasHypervisorBit()
	{
		return CPUID(1).ECX() & CPUID::HYPERVISOR_PRESENT_FLAG;
	}

	std::string GetHypervisorName()
	{
		if (!HasHypervisorBit()) return "";

		CPUID id(CPUID::HYPERVISOR_INFO_FLAG);
		auto str = id.get_ebx_string() + id.get_ecx_string() + id.get_edx_string();
		trim(str);
		return str;
	}

	bool IsHyperVGuestOS()
	{
		if (GetHypervisorName() != HYPER_V_NAME) return false;

		// TODO: More checks?
		if (CPUID(CPUID::HYPERVISOR_INFO_FLAG + 128).has_data() ||
			CPUID(CPUID::HYPERVISOR_INFO_FLAG + 129).has_data() ||
			CPUID(CPUID::HYPERVISOR_INFO_FLAG + 130).has_data())
			return true;

		return false;
	}

	bool IsSuspectedVM()
	{
		if (HasVMExecutionTiming() || IsHyperVGuestOS()) return true;
		if (!HasHypervisorBit()) return false;
		if (GetHypervisorName() != HYPER_V_NAME) return true;

		return false;
	}

    // Extension checking taken from https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
    class InstructionSet
    {
    public:
        InstructionSet() :
            isIntel_{ false },
            isAMD_{ false },
            f_1_ECX_{ 0 },
            f_1_EDX_{ 0 },
            f_7_EBX_{ 0 },
            f_7_ECX_{ 0 },
            f_81_ECX_{ 0 },
            f_81_EDX_{ 0 }
        {
            std::string vendor = CPUID::GetVendor();
            if (vendor == "GenuineIntel")
                isIntel_ = true;
            else if (vendor == "AuthenticAMD")
                isAMD_ = true;
            
            uint32_t ids = CPUID(0).EAX();
            if (ids >= 1)
            {
                CPUID id1(1);
                f_1_ECX_ = id1.ECX();
                f_1_EDX_ = id1.EDX();
            }
            if (ids >= 7)
            {
                CPUID id7(7);
                f_7_EBX_ = id7.EBX();
                f_7_ECX_ = id7.ECX();
            }
            if (CPUID(CPUID::CPUID_FLAG).EAX() >= CPUID::CPUID_EXTENDED_FLAG)
            {
                CPUID ext(CPUID::CPUID_EXTENDED_FLAG);
                f_81_ECX_ = ext.ECX();
                f_81_EDX_ = ext.EDX();
            }
        };

        bool SSE3() { return f_1_ECX_[0]; }
        bool PCLMULQDQ() { return f_1_ECX_[1]; }
        bool MONITOR() { return f_1_ECX_[3]; }
        bool SSSE3() { return f_1_ECX_[9]; }
        bool FMA() { return f_1_ECX_[12]; }
        bool CMPXCHG16B() { return f_1_ECX_[13]; }
        bool SSE41() { return f_1_ECX_[19]; }
        bool SSE42() { return f_1_ECX_[20]; }
        bool MOVBE() { return f_1_ECX_[22]; }
        bool POPCNT() { return f_1_ECX_[23]; }
        bool AES() { return f_1_ECX_[25]; }
        bool XSAVE() { return f_1_ECX_[26]; }
        bool OSXSAVE() { return f_1_ECX_[27]; }
        bool AVX() { return f_1_ECX_[28]; }
        bool F16C() { return f_1_ECX_[29]; }
        bool RDRAND() { return f_1_ECX_[30]; }

        bool MSR() { return f_1_EDX_[5]; }
        bool CX8() { return f_1_EDX_[8]; }
        bool SEP() { return f_1_EDX_[11]; }
        bool CMOV() { return f_1_EDX_[15]; }
        bool CLFSH() { return f_1_EDX_[19]; }
        bool MMX() { return f_1_EDX_[23]; }
        bool FXSR() { return f_1_EDX_[24]; }
        bool SSE() { return f_1_EDX_[25]; }
        bool SSE2() { return f_1_EDX_[26]; }

        bool FSGSBASE() { return f_7_EBX_[0]; }
        bool BMI1() { return f_7_EBX_[3]; }
        bool HLE() { return isIntel_ && f_7_EBX_[4]; }
        bool AVX2() { return f_7_EBX_[5]; }
        bool BMI2() { return f_7_EBX_[8]; }
        bool ERMS() { return f_7_EBX_[9]; }
        bool INVPCID() { return f_7_EBX_[10]; }
        bool RTM() { return isIntel_ && f_7_EBX_[11]; }
        bool AVX512F() { return f_7_EBX_[16]; }
        bool RDSEED() { return f_7_EBX_[18]; }
        bool ADX() { return f_7_EBX_[19]; }
        bool AVX512PF() { return f_7_EBX_[26]; }
        bool AVX512ER() { return f_7_EBX_[27]; }
        bool AVX512CD() { return f_7_EBX_[28]; }
        bool SHA() { return f_7_EBX_[29]; }

        bool PREFETCHWT1() { return f_7_ECX_[0]; }

        bool LAHF() { return f_81_ECX_[0]; }
        bool LZCNT() { return isIntel_ && f_81_ECX_[5]; }
        bool ABM() { return isAMD_ && f_81_ECX_[5]; }
        bool SSE4a() { return isAMD_ && f_81_ECX_[6]; }
        bool XOP() { return isAMD_ && f_81_ECX_[11]; }
        bool TBM() { return isAMD_ && f_81_ECX_[21]; }

        bool SYSCALL() { return isIntel_ && f_81_EDX_[11]; }
        bool MMXEXT() { return isAMD_ && f_81_EDX_[22]; }
        bool RDTSCP() { return isIntel_ && f_81_EDX_[27]; }
        bool _3DNOWEXT() { return isAMD_ && f_81_EDX_[30]; }
        bool _3DNOW() { return isAMD_ && f_81_EDX_[31]; }

    private:
        bool isIntel_;
        bool isAMD_;
        std::bitset<32> f_1_ECX_;   
        std::bitset<32> f_1_EDX_;
        std::bitset<32> f_7_EBX_;
        std::bitset<32> f_7_ECX_;
        std::bitset<32> f_81_ECX_;
        std::bitset<32> f_81_EDX_;
    };

    std::vector<std::string> GetCPUExtensions()
    {
        std::vector<std::string> extensions;

        auto add_extension = [&extensions](std::string isa_feature, bool is_supported)
        {
            if (is_supported) extensions.push_back(isa_feature);
        };

        InstructionSet instructionSet;

        add_extension("3DNOW",       instructionSet._3DNOW());
        add_extension("3DNOWEXT",    instructionSet._3DNOWEXT());
        add_extension("ABM",         instructionSet.ABM());
        add_extension("ADX",         instructionSet.ADX());
        add_extension("AES",         instructionSet.AES());
        add_extension("AVX",         instructionSet.AVX());
        add_extension("AVX2",        instructionSet.AVX2());
        add_extension("AVX512CD",    instructionSet.AVX512CD());
        add_extension("AVX512ER",    instructionSet.AVX512ER());
        add_extension("AVX512F",     instructionSet.AVX512F());
        add_extension("AVX512PF",    instructionSet.AVX512PF());
        add_extension("BMI1",        instructionSet.BMI1());
        add_extension("BMI2",        instructionSet.BMI2());
        add_extension("CLFSH",       instructionSet.CLFSH());
        add_extension("CMPXCHG16B",  instructionSet.CMPXCHG16B());
        add_extension("CX8",         instructionSet.CX8());
        add_extension("ERMS",        instructionSet.ERMS());
        add_extension("F16C",        instructionSet.F16C());
        add_extension("FMA",         instructionSet.FMA());
        add_extension("FSGSBASE",    instructionSet.FSGSBASE());
        add_extension("FXSR",        instructionSet.FXSR());
        add_extension("HLE",         instructionSet.HLE());
        add_extension("INVPCID",     instructionSet.INVPCID());
        add_extension("LAHF",        instructionSet.LAHF());
        add_extension("LZCNT",       instructionSet.LZCNT());
        add_extension("MMX",         instructionSet.MMX());
        add_extension("MMXEXT",      instructionSet.MMXEXT());
        add_extension("MONITOR",     instructionSet.MONITOR());
        add_extension("MOVBE",       instructionSet.MOVBE());
        add_extension("MSR",         instructionSet.MSR());
        add_extension("OSXSAVE",     instructionSet.OSXSAVE());
        add_extension("PCLMULQDQ",   instructionSet.PCLMULQDQ());
        add_extension("POPCNT",      instructionSet.POPCNT());
        add_extension("PREFETCHWT1", instructionSet.PREFETCHWT1());
        add_extension("RDRAND",      instructionSet.RDRAND());
        add_extension("RDSEED",      instructionSet.RDSEED());
        add_extension("RDTSCP",      instructionSet.RDTSCP());
        add_extension("RTM",         instructionSet.RTM());
        add_extension("SEP",         instructionSet.SEP());
        add_extension("SHA",         instructionSet.SHA());
        add_extension("SSE",         instructionSet.SSE());
        add_extension("SSE2",        instructionSet.SSE2());
        add_extension("SSE3",        instructionSet.SSE3());
        add_extension("SSE4.1",      instructionSet.SSE41());
        add_extension("SSE4.2",      instructionSet.SSE42());
        add_extension("SSE4a",       instructionSet.SSE4a());
        add_extension("SSSE3",       instructionSet.SSSE3());
        add_extension("SYSCALL",     instructionSet.SYSCALL());
        add_extension("TBM",         instructionSet.TBM());
        add_extension("XOP",         instructionSet.XOP());
        add_extension("XSAVE",       instructionSet.XSAVE());

        return extensions;
    }
// ARM64, currently only supported on macOS
#else
    class CPUID
    {
    public:
		static Bitness GetBitness()
        {
            return Bitness::BITNESS_64;
        }

		static std::string GetBrand()
        {
            return GetOSString("machdep.cpu.brand_string");
        }

		static std::string GetVendor()
        {
            return "Apple";
        }

		static int32_t GetModel()
        {
            return GetOSInteger("hw.cpufamily");
        }

		static int32_t GetStepping()
        {
            return GetOSInteger("hw.cpusubfamily");
        }
    };

    bool HasHypervisorBit()
	{
        return GetOSInteger("kern.hv_vmm_present");
	}

	std::string GetHypervisorName()
	{
        // Model name will always contain 'mac' on original hardware
		std::string hw = GetOSString("hw.model");
        return tolower(hw).find("mac") == std::string::npos ? hw : "";
	}

	bool IsHyperVGuestOS()
    {
        return false;
    }

    std::vector<std::string> GetCPUExtensions()
    {
        // TODO: Query extensions
        return {"AES", "CRC32", "PMULL", "SHA1", "SHA2"};
    }

	bool IsSuspectedVM()
	{
		return
            HasVMExecutionTiming() ||
            HasHypervisorBit() ||
            GetHypervisorName() != "";
	}
#endif

    size_t GetTimingCycles()
	{
		volatile size_t time1 = 0;
		volatile size_t time2 = 0;

#if _WIN64
		time1 = __rdtsc();
		time2 = __rdtsc();
#elif _WIN32
		__asm
		{
			RDTSC
			MOV time1, EAX
			RDTSC
			MOV time2, EAX
		}
#elif __aarch64__
		asm volatile("mrs %0, cntvct_el0" : "=r" (time1));
		asm volatile("mrs %0, cntvct_el0" : "=r" (time2));
#else
		asm volatile("RDTSC" : "=a" (time1));
		asm volatile("RDTSC" : "=a" (time2));
#endif
		return time2 - time1;
	}
}