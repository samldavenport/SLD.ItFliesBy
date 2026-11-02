#ifndef JOBS_HPP
#define JOBS_HPP

#include "ifb.hpp"
#include "ifb-platform.hpp"
#include "eng-memory.hpp"

namespace ifb {

    struct job_dspchr;
    struct job;
    struct job_worker;
    struct job_queue;
    struct job_worker_pool;


    IFB_INTERNAL job_dspchr*  job_dspchr_create   (void);
    IFB_INTERNAL void         job_dspchr_startup  (reservation* res);
    IFB_INTERNAL void         job_dspchr_shutdown (void);
    IFB_INTERNAL void         job_dspchr_main     (void* ctx);

    IFB_INTERNAL job_queue*   job_queue_create    (void);
    IFB_INTERNAL void         job_queue_init      (job_queue* queue, memory queue_mem);
    IFB_INTERNAL bool         job_queue_has_work  (const job_queue* queue);
    IFB_INTERNAL bool         job_queue_enqueue   (job_queue* queue, const job& j);
    IFB_INTERNAL bool         job_queue_dequeue   (job_queue* queue, job& j);
    
    IFB_INTERNAL job_worker_pool* job_worker_pool_create (void);
    IFB_INTERNAL void             job_worker_pool_init   (job_worker_pool* worker_pool);

    struct job_worker {
        pfm_hnd_thread h_pfm_thread;
        hnd_arena      h_arena;
    };


};

#endif //JOBS_HPP
