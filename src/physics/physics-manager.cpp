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
    
    struct phys_mngr {
        reservation*      res;
        phys_frc_accmltr* force_accumulator;
        phys_frc_intgrtr* force_integrator;
        phys_frc_sprcalc* spring_calculator;
        phys_cllsn_dtctr* collision_detector;
    };

    //--------------------------------------------------------------------
    // PUBLIC METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL phys_mngr*
    phys_mngr_create(
        void) {

        _phys_mngr  = global_alloc<phys_mngr>();
        assert(_phys_mngr  != NULL);

        return(_phys_mngr);
    }

    IFB_INTERNAL void
    phys_mngr_validate(
        void) {

        assert(_phys_mngr                     != NULL);
        assert(_phys_mngr->res                != NULL);
        assert(_phys_mngr->force_accumulator  != NULL);
        assert(_phys_mngr->force_integrator   != NULL);
        assert(_phys_mngr->spring_calculator  != NULL);
        assert(_phys_mngr->collision_detector != NULL);
    }

    IFB_INTERNAL void
    phys_mngr_startup(
        reservation* res) {

        assert(_phys_mngr);
        assert(res);

        _phys_mngr->res                = res;
        _phys_mngr->spring_calculator  = phys_frc_sprcalc_create ();
        _phys_mngr->force_accumulator  = phys_frc_accmltr_create ();
        _phys_mngr->force_integrator   = phys_frc_intgrtr_create (); 
        _phys_mngr->collision_detector = phys_cllsn_dtctr_create ();

        phys_mngr_validate();
    }

    IFB_INTERNAL void
    phys_mngr_shutdown(
        void) {

        //TODO(SAM)
    }

    IFB_INTERNAL void
    phys_mngr_simulate(
        const f32 dt) {

        const phys_cllsn_list* cllsn_list = phys_cllsn_dtctr_get_list(_phys_mngr->collision_detector);

        phys_cllsn_dtctr_run   (_phys_mngr->collision_detector, dt);
        phys_frc_sprcalc_run   (_phys_mngr->spring_calculator);
        phys_frc_intgrtr_run   (_phys_mngr->force_integrator, _phys_mngr->force_accumulator, cllsn_list, dt);
        phys_frc_accmltr_reset (_phys_mngr->force_accumulator);
    }

    //--------------------------------------------------------------------
    // PRIVATE METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL void*
    phys_mngr_res_alloc(
        const u32 size_min) {

        assert(size_min   != 0);
        assert(_phys_mngr != NULL);

        void* mem = reservation_push_bytes(_phys_mngr->res, size_min);

        return(mem);
    }
    
    IFB_INTERNAL phys_frc_accmltr*
    phys_mngr_get_frc_accmltr(
        void) {

        assert(_phys_mngr);
        return(_phys_mngr->force_accumulator);
    }
};
