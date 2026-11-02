#pragma once

#include "eng-jobs.hpp"
#include "ifb-platform.hpp"
#include "eng-memory.hpp"
#include "eng-context.hpp"

namespace ifb {

    IFB_INTERNAL job_dspchr*
    job_dspchr_create(
        void) {

        auto dspchr = (job_dspchr*)global_alloc(sizeof(job_dspchr));
        assert(dspchr);

        dspchr->queue = job_queue_create();
        assert(dspchr);

        return(dspchr);
    }

    IFB_INTERNAL void
    job_dspchr_startup(
        void) {

        assert(_job_dspchr);   
        assert(_job_dspchr->queue);

        _job_dspchr->shutdown = false;

        job_queue_init(_job_dspchr->queue);
    
        // get a count of CPU cores
        const u32 core_count = pfm_system_get_core_count();
    }

    IFB_INTERNAL void
    job_dspchr_shutdown(
        void) {

        _job_dspchr->shutdown = true;
    }

    IFB_INTERNAL void*
    job_dspchr_alloc(
        const u32 size) {

        auto mem = reservation_push_bytes(_job_dspchr->res, size);
        return(mem);
    }
    
    IFB_INTERNAL void
    job_dspchr_main(
        void* ctx) {

        auto worker = (job_worker*)ctx;
    
        for (;;) {

            // lock the dispatcher
            pfm_thread_mutex_lock(_job_dspchr->lock);
       
            // wait for work while we are still running
            while(!job_queue_has_work(_job_dspchr->queue) && !_job_dspchr->shutdown) {
                pfm_thread_cond_wait(_job_dspchr->work_available, _job_dspchr->lock);
            }

            // received shutdown
            if (!job_queue_has_work(_job_dspchr->queue)) {
                pfm_thread_mutex_unlock(_job_dspchr->lock);
                return;
            }

            // get the next job
            job next_job;
            const bool did_get = job_queue_dequeue(_job_dspchr->queue,next_job);
            assert(did_get);

            // unlock the dispatcher
            pfm_thread_mutex_unlock(_job_dspchr->lock);
       
            // reset the arena and do the work
            arena_reset(worker->h_arena);
            next_job.proc(worker->h_arena, &next_job.ctx);

            // wake anyone waiting if this was the last job
            pfm_thread_mutex_lock(_job_dspchr->lock);
            const bool is_idle = (--_job_dspchr->unfinished == 0);
            pfm_thread_mutex_unlock(_job_dspchr->lock);

            if (is_idle) {
                pfm_thread_cond_wake_all(_job_dspchr->all_done);
            }
        }
    }

};
