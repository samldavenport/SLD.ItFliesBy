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

    // older sdks don't define it
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#   define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

    // sleeps on a high resolution timer until about a millisecond before, then spins the rest,
    // since the timer can wake up a little late but spinning the whole wait would keep a core busy
    IFB_PLATFORM_API void
    pfm_system_wait_until(
        const u64 timestamp) {

        static const u64 freq      = pfm_system_frequency();
        static const u64 spin_time = freq / 1000;

        // made once and kept, it's only used from the main thread
        // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION needs windows 10 1803, older versions fall back to a normal timer
        static HANDLE timer = NULL;
        if (timer == NULL) {
            timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
            if (timer == NULL) {
                timer = CreateWaitableTimerExW(NULL, NULL, 0, TIMER_ALL_ACCESS);
            }
        }

        u64 now = pfm_system_timestamp();
        if (now >= timestamp) return;

        const u64 remaining = timestamp - now;
        if (timer != NULL && remaining > spin_time) {

            // in 100 ns units, negative means relative to now
            const u64     sleep_ticks = remaining - spin_time;
            LARGE_INTEGER due;
            due.QuadPart = -(LONGLONG)((sleep_ticks * 10000000ull) / freq);

            if (SetWaitableTimerEx(timer, &due, 0, NULL, NULL, NULL, 0)) {
                WaitForSingleObject(timer, INFINITE);
            }
        }

        while (pfm_system_timestamp() < timestamp) {
            YieldProcessor();
        }
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
