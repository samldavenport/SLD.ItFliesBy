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
};
