#pragma once

#include "memory-arena.cpp"
#include "physics.hpp"
#include "eng-internal.hpp"
#include <cassert>
#include <cstddef>

namespace ifb {
    
    IFB_INTERNAL physics_mngr*
    physics_mngr_create(
        void) {

        auto mngr  = global_alloc<physics_mngr>();
        auto mem   = global_alloc<physics_memory>();
        auto accum = global_alloc<physics_accumulator>();         

        assert(mngr  != NULL);
        assert(mem   != NULL);
        assert(accum != NULL);
    
        mngr->memory            = mem;
        mngr->force_accumulator = accum;

        _phys_mngr = mngr;
        return(mngr);
    }

    IFB_INTERNAL void
    physics_mngr_validate(
        void) {

        assert(_phys_mngr                    != NULL);
        assert(_phys_mngr->memory            != NULL);
        assert(_phys_mngr->force_accumulator != NULL);
    }

    IFB_INTERNAL void
    physics_mngr_startup(
        memory& mem_res) {

        physics_mngr_validate();
        auto phys_mem = _phys_mngr->memory; 

        assert(mem_res.address != 0);
        assert(mem_res.size    != 0);

        // commit stack memory
        memory mem_stack;
        mem_stack.ptr  = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        mem_stack.size = mem_res.size;
        assert(mem_stack.ptr != NULL);

        // initialize stack and remaining structures
        phys_mem->stack.init(mem_stack);
        _phys_mngr->force_accumulator = physics_accumulator_init(phys_mem->stack);

        phys_mem->simulation_arena = arena_alloc();
        assert(phys_mem->simulation_arena != NULL);
    }

    IFB_INTERNAL void
    physics_mngr_shutdown(
        void) {

        //TODO(SAM)
        
    }

    IFB_INTERNAL void
    physics_mngr_simulate(
        const f32 dt) {

        auto phys_mem = _phys_mngr->memory;

        arena_reset                     (phys_mem->simulation_arena);
        physics_spring_calculate_forces (phys_mem->simulation_arena); 
        physics_integrate_forces        (dt, phys_mem->simulation_arena);
        physics_accumulator_reset       (_phys_mngr->force_accumulator);
    }
};
