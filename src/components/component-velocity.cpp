#pragma once

#include "component.hpp"
#include "entity.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_velocity_validate(
        void) {

        assert(
            _cmpnt_mngr                  != NULL &&
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.velocity != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_table_velocity_update(
        const u32          sparse_index,
        const velocity_3d& velocity) {

        cmpnt_table_velocity_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_velocity* tbl = _cmpnt_mngr->tables.velocity;

        tbl->x[sparse_index] = velocity.x; 
        tbl->y[sparse_index] = velocity.y; 
        tbl->z[sparse_index] = velocity.z; 
    }

    IFB_INTERNAL void
    cmpnt_table_velocity_lookup(
        const u32    sparse_index,
        velocity_3d& velocity) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity >  sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.velocity;
        assert(tbl != NULL);
        
        velocity.x = tbl->x[sparse_index];
        velocity.y = tbl->y[sparse_index];
        velocity.z = tbl->z[sparse_index];
    }
};
