#pragma once

#include "ifb.hpp"
#include "memory-arena.cpp"
#include "memory-reservation.cpp"
#include "eng-context.hpp"
#include <cassert>
#include <cstddef>
#include "eng-physics.hpp"
#include "eng-memory.hpp"
#include "ifb-engine.hpp"
#include "ifb-platform.hpp"

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
        phys_cllsn_rslvr* collision_resolver;
        phys_cntct_list*  contact_list_curr; // the contacts from this step
        phys_cntct_list*  contact_list_prev; // the contacts from the last step, for the phases
        eng_physics_stats stats;             // the last step's
    };

    // milliseconds between two timestamps
    inline f32
    phys_mngr_elapsed_ms(
        const u64 start,
        const u64 end) {

        const u64 freq = pfm_system_frequency();
        if (freq == 0) return(0.0f);
        return((f32)((f64)(end - start) * 1000.0 / (f64)freq));
    }

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
        assert(_phys_mngr->collision_resolver != NULL);
        assert(_phys_mngr->contact_list_curr  != NULL);
        assert(_phys_mngr->contact_list_prev  != NULL);
        assert(_phys_mngr->contact_list_curr  != _phys_mngr->contact_list_prev);
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
        _phys_mngr->collision_resolver = phys_cllsn_rslvr_create ();
        _phys_mngr->contact_list_curr  = phys_cntct_list_create  ();
        _phys_mngr->contact_list_prev  = phys_cntct_list_create  ();

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

        // this step's contacts become the last step's
        // the resolver resets the other list and fills it with this step's
        phys_cntct_list* cntct_list_tmp = _phys_mngr->contact_list_prev;
        _phys_mngr->contact_list_prev   = _phys_mngr->contact_list_curr;
        _phys_mngr->contact_list_curr   = cntct_list_tmp;

        // collisions are resolved after integration, once the components hold the new positions
        // the resolver fills the contact list with the pairs that actually overlap
        const u64 t_start = pfm_system_timestamp();
        phys_cllsn_dtctr_run   (_phys_mngr->collision_detector, dt);
        const u64 t_detect = pfm_system_timestamp();
        phys_frc_sprcalc_run   (_phys_mngr->spring_calculator);
        const u64 t_springs = pfm_system_timestamp();
        phys_frc_intgrtr_run   (_phys_mngr->force_integrator, _phys_mngr->force_accumulator, dt);
        const u64 t_integrate = pfm_system_timestamp();
        phys_cllsn_rslvr_run   (_phys_mngr->collision_resolver, cllsn_list, _phys_mngr->contact_list_curr, dt);
        phys_frc_accmltr_reset (_phys_mngr->force_accumulator);
        const u64 t_resolve = pfm_system_timestamp();

        // the step is done, so the procs can't change anything out from under it
        // forces they add land in the next step
        _phys_cntct_events_dropped = 0;
        (void)phys_cntct_list_walk_phases(
            _phys_mngr->contact_list_curr,
            _phys_mngr->contact_list_prev,
            phys_cntct_dispatch
        );
        const u64 t_dispatch = pfm_system_timestamp();

        eng_physics_stats& stats  = _phys_mngr->stats;
        stats.count_pairs         = phys_cllsn_list_get_count(cllsn_list);
        stats.count_pairs_dropped = phys_cllsn_list_get_count_dropped(cllsn_list);
        stats.capacity_pairs      = config::entity_capacity;
        stats.count_contacts      = phys_cntct_list_get_count(_phys_mngr->contact_list_curr);
        stats.count_resolved      = phys_cllsn_rslvr_get_count_resolved(_phys_mngr->collision_resolver);
        stats.count_events_dropped = _phys_cntct_events_dropped;
        stats.ms_detect           = phys_mngr_elapsed_ms(t_start,     t_detect);
        stats.ms_springs          = phys_mngr_elapsed_ms(t_detect,    t_springs);
        stats.ms_integrate        = phys_mngr_elapsed_ms(t_springs,   t_integrate);
        stats.ms_resolve          = phys_mngr_elapsed_ms(t_integrate, t_resolve);
        stats.ms_dispatch         = phys_mngr_elapsed_ms(t_resolve,   t_dispatch);
        stats.ms_total            = phys_mngr_elapsed_ms(t_start,     t_dispatch);
    }

    IFB_INTERNAL const eng_physics_stats&
    phys_mngr_get_stats(
        void) {

        assert(_phys_mngr);
        return(_phys_mngr->stats);
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
