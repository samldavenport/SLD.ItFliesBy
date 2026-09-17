#pragma once

#include "component.hpp"
#include "eng-internal.hpp"
#include "memory-reservation.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr*
    cmpnt_mngr_create(
        void) {

        _cmpnt_mngr = global_alloc<cmpnt_mngr>();
        assert(_cmpnt_mngr);

        return(_cmpnt_mngr);
    }

    IFB_INTERNAL void
    cmpnt_mngr_startup(
        reservation* res) {

        assert(res);

        const auto& cfg = config_instance();

        // calculate capacity
        _cmpnt_mngr->capacity = (cfg.entity_capacity / cfg.sparse_set_max_load_p100);
        assert(_cmpnt_mngr->capacity != 0);

        // commit memory to a stack
        _cmpnt_mngr->mem = reservation_push_stack_all(res);
        assert(_cmpnt_mngr->mem);

        // create tables
        _cmpnt_mngr->tbl.position.stack_init      (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.color.stack_init         (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.quad.stack_init          (_cmpnt_mngr->mem); 
        _cmpnt_mngr->tbl.rigid_body.stack_init    (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.velocity.stack_init      (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.acceleration.stack_init  (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.inv_mass.stack_init      (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.drag.stack_init          (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.term_velocity.stack_init (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.spring.stack_init        (_cmpnt_mngr->mem);
        _cmpnt_mngr->tbl.map_coords.stack_init    (_cmpnt_mngr->mem);
    }
};
