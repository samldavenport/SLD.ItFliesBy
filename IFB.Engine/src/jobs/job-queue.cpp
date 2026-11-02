#pragma once

#include "eng-events.hpp"

namespace ifb {

    IFB_INTERNAL job_queue*
    job_queue_create(
        void) {

        
    }

    IFB_INTERNAL bool
    job_queue_has_work(
        const job_queue* queue) {

    }

    IFB_INTERNAL bool
    job_queue_enqueue(
        job_queue* queue,
        const job& j) {

    }
    
    IFB_INTERNAL bool
    job_queue_dequeue(
        job_queue* queue,
        job&       j) {
        
    }

};
