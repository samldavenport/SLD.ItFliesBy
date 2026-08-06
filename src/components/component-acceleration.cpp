#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_acceleration_validate(
        void) {

        assert(
            _cmpnt_mngr                  != NULL &&
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.acceleration != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_table_acceleration_update(
        const u32              sparse_index,
        const acceleration_3d& acceleration) {

        cmpnt_table_acceleration_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_acceleration* tbl = _cmpnt_mngr->tables.acceleration;

        tbl->x[sparse_index] = acceleration.x; 
        tbl->y[sparse_index] = acceleration.y; 
        tbl->z[sparse_index] = acceleration.z; 
    }

    IFB_INTERNAL void
    cmpnt_table_acceleration_lookup(
        const u32        sparse_index,
        acceleration_3d& acceleration) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity >  sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.acceleration;
        assert(tbl != NULL);
        
        acceleration.x = tbl->x[sparse_index];
        acceleration.y = tbl->y[sparse_index];
        acceleration.z = tbl->z[sparse_index];
    }
};
