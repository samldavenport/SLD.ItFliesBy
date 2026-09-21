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
    // DEFINITIONS
    //--------------------------------------------------------------------
    
    struct physics_mngr {
        reservation*               res;
        physics_force_accumulator* force_accumulator;
    };

    //--------------------------------------------------------------------
    // PUBLIC METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL physics_mngr*
    physics_mngr_create(
        void) {

        _phys_mngr  = global_alloc<physics_mngr>();
        assert(_phys_mngr  != NULL);

        return(_phys_mngr);
    }

    IFB_INTERNAL void
    physics_mngr_validate(
        void) {

        assert(_phys_mngr                    != NULL);
        assert(_phys_mngr->res               != NULL);
        assert(_phys_mngr->force_accumulator != NULL);
    }

    IFB_INTERNAL void
    physics_mngr_startup(
        reservation* res) {

        assert(_phys_mngr);
        assert(res);

        _phys_mngr->res               = res;
        _phys_mngr->force_accumulator = physics_force_accumulator_create(res);
        
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

        const hnd_arena sim_arena = arena_alloc();
        if (sim_arena != INVALID_HANDLE) {
            physics_spring_calculate_forces (sim_arena); 
            physics_integrate_forces        (_phys_mngr->force_accumulator, dt, sim_arena);
            physics_force_accumulator_reset (_phys_mngr->force_accumulator);
            arena_free(sim_arena);
        }
    }

    //--------------------------------------------------------------------
    // PRIVATE METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void*
    physics_mngr_res_alloc(
        const u32 size_min) {

        assert(size_min   != 0);
        assert(_phys_mngr != NULL);

        void* mem = reservation_push_bytes(_phys_mngr->res, size_min);

        return(mem);
    }
    
    IFB_INTERNAL physics_force_accumulator*
    physics_mngr_get_force_accumulator(
        void) {

        assert(_phys_mngr);
        return(_phys_mngr->force_accumulator);
    }
};
