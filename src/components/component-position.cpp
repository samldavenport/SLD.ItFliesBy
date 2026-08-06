#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_position_validate(
        void) {

        assert(
            _cmpnt_mngr                  != NULL &&
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.position != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_table_position_update(
        const u32          sparse_index,
        const position_3d& position) {

        cmpnt_table_position_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_position* tbl = _cmpnt_mngr->tables.position;

        tbl->x[sparse_index] = position.x; 
        tbl->y[sparse_index] = position.y; 
        tbl->z[sparse_index] = position.z; 
    }

    IFB_INTERNAL void
    cmpnt_table_position_lookup(
        const u32    sparse_index,
        position_3d& position) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity >  sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.position;
        assert(tbl != NULL);
        
        position.x = tbl->x[sparse_index];
        position.y = tbl->y[sparse_index];
        position.z = tbl->z[sparse_index];
    }
};
