#pragma once

#include "eng-jobs.hpp"
#include "eng-context.hpp"

namespace ifb {

    struct job_queue {
        job* job_array;
        u32  head;
        u32  capacity;
        u32  count;
    };
    
    IFB_INTERNAL job_queue*
    job_queue_create(
        void) {

        auto queue = (job_queue*)global_alloc(sizeof(job_queue));
        assert(queue);

        return(queue);
    }

    IFB_INTERNAL void
    job_queue_init(
        job_queue* queue,
        memory     queue_mem) {

        assert(queue);
        assert(queue_mem.size >= sizeof(job));
        assert(queue_mem.ptr  != NULL);
        assert((queue_mem.address % alignof(job)) == 0);

        queue->job_array = (job*)queue_mem.ptr;
        queue->capacity  = (u32)(queue_mem.size / sizeof(job));
        queue->head      = 0;
        queue->count     = 0;
    }

    // the queue doesn't lock, callers must hold the dispatcher lock

    IFB_INTERNAL bool
    job_queue_has_work(
        const job_queue* queue) {

        assert(queue);

        return(queue->count > 0);
    }

    IFB_INTERNAL bool
    job_queue_enqueue(
        job_queue* queue,
        const job& j) {

        assert(queue);
        assert(j.proc);
        assert(queue->count <= queue->capacity);

        if (queue->count == queue->capacity) {
            return(false);
        }

        const u32 tail = (queue->head + queue->count) % queue->capacity;

        queue->job_array[tail] = j;
        ++queue->count;

        return(true);
    }

    IFB_INTERNAL bool
    job_queue_dequeue(
        job_queue* queue,
        job&       j) {

        assert(queue);
        assert(queue->count <= queue->capacity);

        if (queue->count == 0) {
            return(false);
        }

        j = queue->job_array[queue->head];

        queue->head = (queue->head + 1) % queue->capacity;
        --queue->count;

        return(true);
    }

};
