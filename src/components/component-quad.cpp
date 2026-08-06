#pragma once

#include "component.hpp"

namespace ifb {


    IFB_INTERNAL void
    cmpnt_table_quad_validate(
        void) {

        assert(
            _cmpnt_mngr              != NULL &&
            _cmpnt_mngr->capacity    != 0    &&
            _cmpnt_mngr->tables.quad != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_table_quad_update(
        const u32            sparse_index,
        const dimensions_2d& dims) {

        cmpnt_table_quad_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.quad;

        tbl->height [sparse_index] = dims.height;
        tbl->width  [sparse_index] = dims.width;
    }

    IFB_INTERNAL void
    cmpnt_table_quad_lookup(
        const u32      sparse_index,
        dimensions_2d& dims) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity >  sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.quad;
        assert(tbl != NULL);
 

        dims.width  = tbl->width  [sparse_index];
        dims.height = tbl->height [sparse_index];
    }
};
