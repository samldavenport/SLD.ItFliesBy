#pragma once

#include "eng-jobs.hpp"
#include "ifb-platform.hpp"
#include "eng-memory.hpp"
#include "eng-context.hpp"

namespace ifb {

    struct job_dspchr {
        reservation*     res;
        job_queue*       queue;
        job_worker_pool* worker_pool;
        hnd_arena        h_arena_inline; // only allocated if there are no workers
        pfm_thread_mutex lock;
        pfm_thread_cond  work_available;
        pfm_thread_cond  all_done;
        u32              unfinished; // jobs queued or running
        bool             shutdown;
    } static * _job_dspchr;

    IFB_INTERNAL job_dspchr*
    job_dspchr_create(
        void) {

        _job_dspchr = (job_dspchr*)global_alloc(sizeof(job_dspchr));
        assert(_job_dspchr);

        _job_dspchr->queue = job_queue_create();
        _job_dspchr->worker_pool = job_worker_pool_create();
        assert(_job_dspchr->queue);
        assert(_job_dspchr->worker_pool);

        return(_job_dspchr);
    }

    IFB_INTERNAL void
    job_dspchr_startup(
        reservation* res) {

        assert(res);
        assert(_job_dspchr);   
        assert(_job_dspchr->queue);

        _job_dspchr->res      = res;
        _job_dspchr->shutdown = false;

        memory queue_mem;
        queue_mem.size = reservation_get_capacity (_job_dspchr->res);
        queue_mem.ptr  = reservation_push_all     (_job_dspchr->res);
        assert(queue_mem.size != 0);
        assert(queue_mem.ptr  != NULL);

        job_queue_init       (_job_dspchr->queue, queue_mem);
        job_worker_pool_init (_job_dspchr->worker_pool);

        if (job_worker_pool_get_count(_job_dspchr->worker_pool) == 0) {
            _job_dspchr->h_arena_inline = arena_alloc();
            assert(_job_dspchr->h_arena_inline != INVALID_HANDLE);
        }
    }

    // workers finish everything in the queue before they exit
    IFB_INTERNAL void
    job_dspchr_shutdown(
        void) {

        assert(_job_dspchr);

        pfm_thread_mutex_lock(_job_dspchr->lock);
        _job_dspchr->shutdown = true;
        pfm_thread_mutex_unlock(_job_dspchr->lock);

        pfm_thread_cond_wake_all(_job_dspchr->work_available);

        job_worker_pool_join(_job_dspchr->worker_pool);
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

    IFB_INTERNAL bool
    job_dspchr_add_job(
        const job& j) {

        assert(_job_dspchr);
        assert(_job_dspchr->queue);

        assert(j.proc);

        // no workers to run it, so do the work now
        // saving and reverting lets a job add another job
        if (job_worker_pool_get_count(_job_dspchr->worker_pool) == 0) {

            if (_job_dspchr->shutdown) return(false);

            job        inline_job = j;
            const u32  save       = arena_save(_job_dspchr->h_arena_inline);

            inline_job.proc(_job_dspchr->h_arena_inline, &inline_job.ctx);
            arena_revert(_job_dspchr->h_arena_inline, save);

            return(true);
        }

        // add the job while the dispatcher is locked
        // nothing new is accepted once shutdown starts
        pfm_thread_mutex_lock(_job_dspchr->lock);
        const bool did_add = !_job_dspchr->shutdown && job_queue_enqueue(_job_dspchr->queue, j);
        if (did_add) {
            ++_job_dspchr->unfinished;
        }
        pfm_thread_mutex_unlock(_job_dspchr->lock);

        // wake a worker to run it
        if (did_add) {
            pfm_thread_cond_wake_one(_job_dspchr->work_available);
        }

        return(did_add);
    }

    // don't call this from a job, that job counts as unfinished and would wait forever
    IFB_INTERNAL void
    job_dspchr_wait(
        void) {

        assert(_job_dspchr);

        pfm_thread_mutex_lock(_job_dspchr->lock);
        while(_job_dspchr->unfinished > 0) {
            pfm_thread_cond_wait(_job_dspchr->all_done, _job_dspchr->lock);
        }
        pfm_thread_mutex_unlock(_job_dspchr->lock);
    }
};
