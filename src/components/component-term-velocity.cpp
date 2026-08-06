#pragma once

#include "component.hpp"
#include "ifb-types.hpp"

namespace ifb {

    IFB_INTERNAL void 
    cmpnt_table_term_velocity_validate(
        void) {

        assert(_cmpnt_mngr                       != NULL);
        assert(_cmpnt_mngr->capacity             != 0);
        assert(_cmpnt_mngr->tables.term_velocity != NULL);
       
        auto tbl = _cmpnt_mngr->tables.term_velocity;

        assert(tbl->x != NULL);
        assert(tbl->y != NULL);
        assert(tbl->z != NULL);
    }

    IFB_INTERNAL void
    cmpnt_table_term_velocity_update(
        const u32               sparse_index,
        const term_velocity_3d& tv) {

        cmpnt_table_term_velocity_validate(); 
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.term_velocity;

        tbl->x[sparse_index] = tv.x; 
        tbl->y[sparse_index] = tv.y; 
        tbl->z[sparse_index] = tv.z; 
    }

    IFB_INTERNAL void
    cmpnt_table_term_velocity_lookup(
        const u32         sparse_index,
        term_velocity_3d& tv) {

        cmpnt_table_term_velocity_validate();        
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.term_velocity;
    
        tv.x = tbl->x[sparse_index];
        tv.y = tbl->y[sparse_index];
        tv.z = tbl->z[sparse_index];
    }
};
