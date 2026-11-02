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

    using job_proc = void (*) (const hnd_arena h_arena, void* ctx);

    IFB_INTERNAL job_dspchr*  job_dspchr_create   (void);
    IFB_INTERNAL void         job_dspchr_startup  (void);
    IFB_INTERNAL void         job_dspchr_shutdown (void);
    IFB_INTERNAL void         job_dspchr_alloc    (void);
    IFB_INTERNAL void         job_dspchr_main     (void* ctx);

    IFB_INTERNAL job_queue*   job_queue_create    (void);
    IFB_INTERNAL void         job_queue_init      (job_queue* queue);
    IFB_INTERNAL bool         job_queue_has_work  (const job_queue* queue);
    IFB_INTERNAL bool         job_queue_enqueue   (job_queue* queue, const job& j);
    IFB_INTERNAL bool         job_queue_dequeue   (job_queue* queue, job& j);


    // aligned so a job can cast the data to its own context struct
    struct alignas(16) job_ctx {
        byte data[64];
    };

    struct job {
        job_proc proc;  
        job_ctx  ctx;
    };

    struct job_worker {
        pfm_hnd_thread h_pfm_thread;
        hnd_arena      h_arena;
    };

    struct job_queue {
        job* job_array;
        u32  head;
        u32  capacity;
        u32  count;
    };

    struct job_worker_array {
       job_worker  workers [config::thread_capacity];
       u32         count;
    };

    struct job_dspchr {
        reservation*      res;
        pfm_thread_mutex  lock;
        pfm_thread_cond   work_available;
        pfm_thread_cond   all_done;
        job_queue*        queue;
        job_worker_array* workers;
        u32               unfinished; // jobs queued or running
        bool              shutdown;
    } static * _job_dspchr;

};

#endif //JOBS_HPP
