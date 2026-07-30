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
    
    IFB_ENG_INTERNAL f64 
    eng_system_get_delta_time_ms(
        void) {

        auto      sys     = _eng_context->system;
        const f64 elapsed = (f64)sys->time.timestamp_curr - (f64)sys->time.timestamp_prev;
        const f64 dt_s    = elapsed / (f64)sys->time.frequency;
        const f64 dt_ms   = dt_s * 1000.0; 
    
        return(dt_ms);
    }
};
