#pragma once

#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "memory-reservation.cpp"
#include "physics.hpp"
#include "eng-internal.hpp"
#include <cassert>
#include <cstddef>
#include "physics-internal.hpp"

namespace ifb {
    
    //--------------------------------------------------------------------
    // PUBLIC METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL physics_mngr*
    physics_mngr_create(
        void) {

        auto mngr  = global_alloc<physics_mngr>();
        auto mem   = global_alloc<physics_memory>();

        assert(mngr  != NULL);
        assert(mem   != NULL);
    
        mngr->memory = mem;

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
        reservation* res) {

        auto phys_mem = _phys_mngr->memory; 

        assert(res);
        assert(phys_mem);


        phys_mem->simulation_arena = arena_alloc();
        phys_mem->res              = res;
        assert(phys_mem->simulation_arena != INVALID_HANDLE);
        
        _phys_mngr->force_accumulator = physics_accumulator_create(res);
        
        physics_mngr_validate();
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

    //--------------------------------------------------------------------
    // PRIVATE METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void*
    physics_mngr_res_alloc(
        const u32 size_min) {

        assert(size_min != 0);
        assert(_phys_mngr != NULL);

        physics_memory* phys_mem = _phys_mngr->memory;
        assert(phys_mem != NULL);

        void* mem = reservation_push_bytes(phys_mem->res, size_min);

        return(mem);
    }
};
