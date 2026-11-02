#pragma once

#include "ifb-engine.hpp"
#include "eng-jobs.hpp"

namespace ifb {

    IFB_ENGINE_API bool
    eng_job_add(
        const job& j) {

        return(job_dspchr_add_job(j));
    }

    IFB_ENGINE_API void
    eng_job_wait(
        void) {

        job_dspchr_wait();
    }
};
