#ifndef JOBS_HPP
#define JOBS_HPP

#include "ifb.hpp"
#include "ifb-platform.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------

    struct job_dspchr;
    struct job_worker;
    struct job_queue;
    struct job_worker_pool;
    struct job_counter;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL job_dspchr*      job_dspchr_create      (void);
    IFB_INTERNAL void             job_dspchr_startup     (reservation* res);
    IFB_INTERNAL void             job_dspchr_shutdown    (void);
    IFB_INTERNAL void             job_dspchr_main        (void* ctx);
    IFB_INTERNAL bool             job_dspchr_add_job     (const job& j);
    IFB_INTERNAL void             job_dspchr_wait        (void);
    IFB_INTERNAL u32              job_dspchr_get_worker_count (void);

    // waiting on a batch of jobs, without waiting on every job in the queue like job_dspchr_wait
    // add to the counter before adding the jobs, each job marks itself done as the last thing it does
    IFB_INTERNAL void             job_counter_add        (job_counter& counter, const u32 count);
    IFB_INTERNAL void             job_counter_done       (job_counter& counter);
    IFB_INTERNAL void             job_counter_wait       (job_counter& counter);

    IFB_INTERNAL job_queue*       job_queue_create       (void);
    IFB_INTERNAL void             job_queue_init         (job_queue* queue, memory queue_mem);
    IFB_INTERNAL bool             job_queue_has_work     (const job_queue* queue);
    IFB_INTERNAL bool             job_queue_enqueue      (job_queue* queue, const job& j);
    IFB_INTERNAL bool             job_queue_dequeue      (job_queue* queue, job& j);
    
    IFB_INTERNAL job_worker_pool* job_worker_pool_create (void);
    IFB_INTERNAL void             job_worker_pool_init   (job_worker_pool* worker_pool);
    IFB_INTERNAL void             job_worker_pool_join   (job_worker_pool* worker_pool);
    IFB_INTERNAL u32              job_worker_pool_get_count (const job_worker_pool* worker_pool);

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct job_worker {
        pfm_hnd_thread h_pfm_thread;
        hnd_arena      h_arena;
    };

    // the jobs in a batch that haven't finished, under the dispatcher's lock
    // it can live on the waiting thread's stack, a job doesn't touch it after it marks itself done
    struct job_counter {
        u32 unfinished;
    };

};

#endif //JOBS_HPP
