#pragma once

#include "eng-internal.hpp"
#include "ifb-engine.hpp"
#include "ifb-platform.hpp"
#include "sld.hpp"

namespace ifb {

    inline eng_system_info*
    system_info_get_and_validate(
        void) {

        assert(_eng_context);
        
        auto sys = _eng_context->system;
        assert(sys);

        return(sys);
    }

    IFB_ENG_INTERNAL void
    system_refresh_info(
        void) {
        
        auto sys = system_info_get_and_validate();
        
        pfm_memory_get_info(sys->mem_info);
    }
    
    IFB_ENG_INTERNAL void
    system_update_time(
        void) {

        auto sys = system_info_get_and_validate();

        static const u64 freq = pfm_system_frequency();

        sys->time.timestamp_prev = sys->time.timestamp_curr;
        sys->time.timestamp_curr = pfm_system_timestamp();
        sys->time.frequency      = freq;  
    }
    
    IFB_ENG_INTERNAL f32 
    system_get_delta_time_s(
        void) {

        auto sys = system_info_get_and_validate();
        
        const f64 elapsed = (f64)sys->time.timestamp_curr - (f64)sys->time.timestamp_prev;
        const f64 dt_s    = elapsed / (f64)sys->time.frequency;
   
        return((f32)dt_s);
    }
    
    IFB_ENG_INTERNAL u32
    system_get_memory_page_size(
        void) {

        auto sys = system_info_get_and_validate();

        return(sys->mem_info.size_page);
    }

    IFB_ENG_INTERNAL u32
    system_get_memory_granularity(
        void) {

        auto sys = system_info_get_and_validate();

        return(sys->mem_info.size_granularity);
    }

    IFB_ENG_INTERNAL u32
    system_align_to_memory_page_size(
        const u32 size) {

        if (size == 0) return(0);
       
        const u32 size_page = system_get_memory_page_size();
        assert(size_page > 0);
        assert(size_is_pow_2(size_page));
    
        const u32 size_aligned = size_align_pow_2(size, size_page);
        return(size_aligned);
    }

    IFB_ENG_INTERNAL u32
    system_align_to_memory_granularity(
        const u32 size) {

        if (size == 0) return(0);
       
        const u32 size_granularity = system_get_memory_granularity();
        assert(size_granularity > 0);
        assert(size_is_pow_2(size_granularity));
    
        const u32 size_aligned = size_align_pow_2(size, size_granularity);
        return(size_aligned);
    }
};
