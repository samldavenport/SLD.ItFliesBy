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
        const cmpnt_velocity& velocity) {

        cmpnt_table_velocity_validate();
        assert(velocity.sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_velocity* tbl = _cmpnt_mngr->tables.velocity;

        tbl->x[velocity.sparse_index] = velocity.x; 
        tbl->y[velocity.sparse_index] = velocity.y; 
        tbl->z[velocity.sparse_index] = velocity.z; 
    }

    IFB_INTERNAL void
    cmpnt_table_velocity_update(
        const velocity_3d& velocity,
        const u32          sparse_index) {

        cmpnt_table_velocity_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_velocity* tbl = _cmpnt_mngr->tables.velocity;

        tbl->x[sparse_index] = velocity.x; 
        tbl->y[sparse_index] = velocity.y; 
        tbl->z[sparse_index] = velocity.z; 
    }

    IFB_INTERNAL void
    cmpnt_table_velocity_lookup(
        cmpnt_velocity& velocity,
        const entity&   e) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > e.index_sparse
        );

        auto tbl = _cmpnt_mngr->tables.velocity;
        assert(tbl != NULL);

        velocity.id           = e.id;
        velocity.sparse_index = e.index_sparse;
        velocity.x            = tbl->x[e.index_sparse];
        velocity.y            = tbl->y[e.index_sparse];
        velocity.z            = tbl->z[e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_table_velocity_lookup(
        velocity_3d& velocity,
        const u32    sparse_index) {

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
