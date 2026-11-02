#include "ifb-platform.hpp"

namespace ifb {
   
    IFB_PLATFORM_API u64
    pfm_system_timestamp(
        void) {

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);

        const u64 timestamp = (u64)counter.QuadPart;
        return(timestamp);
    }

    IFB_PLATFORM_API u64
    pfm_system_frequency(
        void) {

        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);

        const u64 freq_u64 = (u64)freq.QuadPart;
        return(freq_u64);
    }
    
    IFB_PLATFORM_API u32
    pfm_system_get_core_count(
        void) {

        alignas(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) BYTE buffer[16384];
        DWORD size = sizeof(buffer);

        const BOOL result = GetLogicalProcessorInformationEx(
            RelationProcessorCore,
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer,
            &size
        );
        if (!result) {
            const DWORD logical_count = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
            return((logical_count > 0) ? logical_count : 1);
        }

        u32   core_count = 0;
        DWORD offset     = 0;
        while(offset < size) {

            const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info =
                (const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer + offset);
            ++core_count;
            offset += info->Size;
        }

        return(core_count);
    }
}
