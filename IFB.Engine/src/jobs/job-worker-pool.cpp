#pragma once

#include "eng-jobs.hpp"
#include "eng-context.hpp"
#include "ifb-platform.hpp"
#include "ifb.hpp"

namespace ifb {

    struct job_worker_pool {
       job_worker  workers [config::thread_capacity];
       u32         count;
    };
    
    IFB_INTERNAL job_worker_pool*
    job_worker_pool_create(
        void) {

        auto pool = (job_worker_pool*)global_alloc(sizeof(job_worker_pool));
        assert(pool);

        return(pool);
    }
    
    IFB_INTERNAL void
    job_worker_pool_init(
        job_worker_pool* worker_pool) {

        assert(worker_pool);
       
        // one worker per CPU core, leaving a core for the main thread
        const u32 core_count   = pfm_system_get_core_count();
        const u32 worker_count = (core_count > 1) ? (core_count - 1) : 0;

        worker_pool->count = (worker_count < config::thread_capacity)
            ? worker_count
            : config::thread_capacity;

        for (
            u32 i_worker = 0;
                i_worker < worker_pool->count;
              ++i_worker) {

            auto& worker = worker_pool->workers[i_worker];

            worker.h_arena      = arena_alloc();
            worker.h_pfm_thread = pfm_thread_create(job_dspchr_main, &worker);

            assert(worker.h_arena != INVALID_HANDLE);
            assert(worker.h_pfm_thread);
        }
    }

    IFB_INTERNAL void
    job_worker_pool_join(
        job_worker_pool* worker_pool) {

        assert(worker_pool);

        for (
            u32 i_worker = 0;
                i_worker < worker_pool->count;
              ++i_worker) {

            auto& worker = worker_pool->workers[i_worker];

            // already joined by an earlier shutdown
            if (worker.h_pfm_thread == NULL) continue;

            pfm_thread_join(worker.h_pfm_thread);
            worker.h_pfm_thread = NULL;
        }
    }

    IFB_INTERNAL u32
    job_worker_pool_get_count(
        const job_worker_pool* worker_pool) {

        assert(worker_pool);

        return(worker_pool->count);
    }
};
