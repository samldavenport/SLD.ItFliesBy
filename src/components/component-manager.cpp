#pragma once

#include <cassert>

#include "component.hpp"
#include "eng-internal.hpp"
#include "memory-reservation.cpp"
#include "component-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr*
    cmpnt_mngr_create(
        void) {

        _cmpnt_mngr = global_alloc<cmpnt_mngr>();
        assert(_cmpnt_mngr);

        _cmpnt_mngr->cmpnt_tbl = global_alloc<cmpnt_table>();
        assert(_cmpnt_mngr->cmpnt_tbl); 

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

        auto* tbl = _cmpnt_mngr->cmpnt_tbl;
        assert(tbl);

        tbl->array_position      =      (cmpnt_position*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_position));      
        tbl->array_color         =         (cmpnt_color*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_color));         
        tbl->array_quad          =          (cmpnt_quad*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_quad));          
        tbl->array_rigid_body    =    (cmpnt_rigid_body*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_rigid_body));    
        tbl->array_velocity      =      (cmpnt_velocity*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_velocity));      
        tbl->array_acceleration  =  (cmpnt_acceleration*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_acceleration));  
        tbl->array_inv_mass      =      (cmpnt_inv_mass*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_inv_mass));      
        tbl->array_drag          =          (cmpnt_drag*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_drag));          
        tbl->array_term_velocity = (cmpnt_term_velocity*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_term_velocity)); 
        tbl->array_spring        =        (cmpnt_spring*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_spring));        
        tbl->array_map_coords    =    (cmpnt_map_coords*)reservation_push_bytes(res, _cmpnt_mngr->capacity * sizeof(cmpnt_map_coords));    
    
        assert(tbl->array_position);
        assert(tbl->array_color);
        assert(tbl->array_quad);
        assert(tbl->array_rigid_body);
        assert(tbl->array_velocity);
        assert(tbl->array_acceleration);
        assert(tbl->array_inv_mass);
        assert(tbl->array_drag);
        assert(tbl->array_term_velocity);
        assert(tbl->array_spring);
        assert(tbl->array_map_coords);

    }
};
