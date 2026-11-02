#pragma once

#include "ifb-platform.hpp"
#include "ifb-win32.hpp"

namespace ifb {

    static_assert(sizeof(pfm_thread_mutex) == sizeof(SRWLOCK),            "pfm_thread_mutex must match SRWLOCK");
    static_assert(sizeof(pfm_thread_cond)  == sizeof(CONDITION_VARIABLE), "pfm_thread_cond must match CONDITION_VARIABLE");

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------
    
    IFB_WIN32_INTERNAL DWORD WINAPI
    win32_thread_main(
        LPVOID param) {

        const auto thread = (win32_thread*)param;
        thread->proc(thread->ctx);
        return(0);
    }

    // gets the first logical processor of a physical core
    // returns false if the core doesn't exist or the query fails
    IFB_WIN32_INTERNAL bool
    win32_thread_get_core_processor(
        const u32         core_index,
        PROCESSOR_NUMBER& processor) {

        alignas(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) BYTE buffer[16384];
        DWORD size = sizeof(buffer);

        const BOOL result = GetLogicalProcessorInformationEx(
            RelationProcessorCore,
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer,
            &size
        );
        if (!result) return(false);

        u32   i_core = 0;
        DWORD offset = 0;
        while(offset < size) {

            const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info =
                (const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer + offset);

            if (i_core == core_index) {
                const GROUP_AFFINITY& group = info->Processor.GroupMask[0];

                BYTE bit = 0;
                while(((group.Mask >> bit) & 1) == 0) ++bit;

                processor.Group    = group.Group;
                processor.Number   = bit;
                processor.Reserved = 0;
                return(true);
            }

            ++i_core;
            offset += info->Size;
        }

        return(false);
    }

    //--------------------------------------------------------------------
    // API METHODS
    //--------------------------------------------------------------------
    
    IFB_PLATFORM_API pfm_hnd_thread
    pfm_thread_create(
        pfm_fptr_thread_proc proc,
        void*                ctx) {

        assert(proc);
        assert(ctx);

        // find a free slot
        win32_thread* thread = NULL;
        for (
            u32 i_thread = 0;
                i_thread < config::thread_capacity;
              ++i_thread) {

            if (_pfm_ctx.threads[i_thread].hnd == NULL) {
                thread = &_pfm_ctx.threads[i_thread];
                break; 
            }
        }
        if (thread == NULL) return(NULL);

        thread->proc = proc;
        thread->ctx  = ctx;
        thread->hnd  = CreateThread(
            NULL,
            0,
            win32_thread_main,
            thread,
            CREATE_SUSPENDED,
            NULL
        );
        if (thread->hnd == NULL) return(NULL);

        SetThreadPriority(thread->hnd, THREAD_PRIORITY_NORMAL);

        ResumeThread(thread->hnd);

        return(thread->hnd);
    }

    IFB_PLATFORM_API pfm_hnd_thread
    pfm_thread_get_current(
        void) {

        auto hnd = (pfm_hnd_thread)GetCurrentThread();
        assert(hnd);

        return(hnd);
    }
    
    IFB_PLATFORM_API bool
    pfm_thread_set_ideal_core(
        const pfm_hnd_thread thread,
        const u32            core_index) {

        PROCESSOR_NUMBER processor;
        if (!win32_thread_get_core_processor(core_index, processor)) {
            return(false);
        }

        const BOOL result = SetThreadIdealProcessorEx(thread, &processor, NULL);
        return(result != FALSE);
    }
    
    IFB_PLATFORM_API void
    pfm_thread_mutex_lock(
        pfm_thread_mutex& mutex) {


        AcquireSRWLockExclusive((PSRWLOCK)&mutex);
    } 

    IFB_PLATFORM_API void
    pfm_thread_mutex_unlock(
        pfm_thread_mutex& mutex) {

        ReleaseSRWLockExclusive((PSRWLOCK)&mutex);
    } 

    IFB_PLATFORM_API void
    pfm_thread_cond_wait(
        pfm_thread_cond&  cond,
        pfm_thread_mutex& mutex) {

        const BOOL result = SleepConditionVariableSRW(
            (PCONDITION_VARIABLE)&cond,
            (PSRWLOCK)&mutex,
            INFINITE,
            0
        );
        assert(result);
    } 
    
    IFB_PLATFORM_API void
    pfm_thread_cond_wake_one(
        pfm_thread_cond&  cond) {

        WakeConditionVariable((PCONDITION_VARIABLE)&cond);
    }

    IFB_PLATFORM_API void
    pfm_thread_cond_wake_all(
        pfm_thread_cond&  cond) {

        WakeAllConditionVariable((PCONDITION_VARIABLE)&cond);
    }
};

