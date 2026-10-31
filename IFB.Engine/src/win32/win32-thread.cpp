#pragma once

#include "ifb-platform.hpp"
#include "ifb-win32.hpp"

namespace ifb {

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
    
    IFB_PLATFORM_API void
    pfm_thread_set_ideal_core(
        const pfm_hnd_thread thread,
        const u32            core_index) {

        SetThreadIdealProcessor(thread, core_index);        
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
};

