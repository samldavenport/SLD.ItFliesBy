#pragma once

#include "eng-internal.hpp"
#include "ifb-platform.hpp"

namespace ifb {

    IFB_ENG_INTERNAL void
    eng_system_update_time(
        void) {

        auto sys = _eng_context->system;

        static const u64 freq = pfm_system_frequency();

        sys->time.timestamp_prev = sys->time.timestamp_curr;
        sys->time.timestamp_curr = pfm_system_timestamp();
        sys->time.frequency      = freq;  
    }
    
    IFB_ENG_INTERNAL f32 
    eng_system_get_delta_time_s(
        void) {

        auto      sys     = _eng_context->system;
        const f64 elapsed = (f64)sys->time.timestamp_curr - (f64)sys->time.timestamp_prev;
        const f64 dt_s    = elapsed / (f64)sys->time.frequency;
   
        //TODO(SAM): debugging to ensure constant dt
        return((f32)dt_s);
        //return(0.016667f);
    }
};
