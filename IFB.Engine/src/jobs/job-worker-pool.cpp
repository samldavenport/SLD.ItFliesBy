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
       
        // get a count of CPU cores
        // this is also the amount of workers we'll have
        // ideally one per core
        const u32 core_count   = pfm_system_get_core_count();
        const u32 worker_count = (core_count > 1) ? (core_count - 1) : 0;

        worker_pool->count = (worker_count < config::thread_capacity)
            ? worker_count
            : config::thread_capacity;

        for (
            u32 i_worker = 0;
                i_worker < core_count;
              ++i_worker) {

            auto& worker = worker_pool->workers[i_worker];

            worker.h_arena      = arena_alloc();
            worker.h_pfm_thread = pfm_thread_create(job_dspchr_main, &worker);

            assert(worker.h_arena != INVALID_HANDLE);
            assert(worker.h_pfm_thread);
        }
    }
};
