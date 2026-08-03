#pragma once

#include "component.hpp"
#include "entity.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_acceleration_table_validate(
        void) {

        assert(
            _cmpnt_mngr                  != NULL &&
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.acceleration != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_update(
        const cmpnt_acceleration& acceleration) {

        cmpnt_acceleration_table_validate();
        assert(acceleration.sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_acceleration* tbl = _cmpnt_mngr->tables.acceleration;

        tbl->x[acceleration.sparse_index] = acceleration.x; 
        tbl->y[acceleration.sparse_index] = acceleration.y; 
        tbl->z[acceleration.sparse_index] = acceleration.z; 
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_update(
        const acceleration_3d& acceleration,
        const u32          sparse_index) {

        cmpnt_acceleration_table_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_acceleration* tbl = _cmpnt_mngr->tables.acceleration;

        tbl->x[sparse_index] = acceleration.x; 
        tbl->y[sparse_index] = acceleration.y; 
        tbl->z[sparse_index] = acceleration.z; 
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_lookup(
        cmpnt_acceleration& acceleration,
        const entity&   e) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > e.index_sparse
        );

        auto tbl = _cmpnt_mngr->tables.acceleration;
        assert(tbl != NULL);

        acceleration.id           = e.id;
        acceleration.sparse_index = e.index_sparse;
        acceleration.x            = tbl->x[e.index_sparse];
        acceleration.y            = tbl->y[e.index_sparse];
        acceleration.z            = tbl->z[e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_lookup(
        acceleration_3d& acceleration,
        const u32    sparse_index) {

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
