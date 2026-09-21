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
        physics_force_integrator*  force_integrator;
        physics_spring_calculator* spring_calculator;
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
        _phys_mngr->spring_calculator = physics_spring_calculator_create ();
        _phys_mngr->force_accumulator = physics_force_accumulator_create ();
        _phys_mngr->force_integrator  = physics_force_integrator_create  (); 

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

        physics_spring_calculator_run   (_phys_mngr->spring_calculator); 
        physics_force_integrator_run    (_phys_mngr->force_integrator, _phys_mngr->force_accumulator, dt);
        physics_force_accumulator_reset (_phys_mngr->force_accumulator);
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
