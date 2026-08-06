#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_rigid_body_validate(
        void) {

        assert(_cmpnt_mngr                    != NULL);
        assert(_cmpnt_mngr->capacity          != 0);
        assert(_cmpnt_mngr->tables.rigid_body != NULL);

        auto tbl = _cmpnt_mngr->tables.rigid_body;

        assert(tbl->origin_x != NULL);
        assert(tbl->origin_y != NULL);
        assert(tbl->origin_z != NULL);
        assert(tbl->width    != NULL);
        assert(tbl->height   != NULL);
    }

    IFB_INTERNAL void
    cmpnt_table_rigid_body_update(
        const u32         sparse_index,
        const rigid_body& rb) {

        cmpnt_table_rigid_body_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;

        tbl->origin_x [sparse_index] = rb.origin.x;
        tbl->origin_y [sparse_index] = rb.origin.y;
        tbl->origin_z [sparse_index] = rb.origin.z;
        tbl->width    [sparse_index] = rb.dimensions.width;
        tbl->height   [sparse_index] = rb.dimensions.height;
    }

    IFB_INTERNAL void
    cmpnt_table_rigid_body_lookup(
        const u32   sparse_index,
        rigid_body& rb) {
            
        cmpnt_table_rigid_body_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;
        assert(tbl != NULL);

        rb.origin.x          = tbl->origin_x [sparse_index];
        rb.origin.y          = tbl->origin_y [sparse_index];
        rb.origin.z          = tbl->origin_z [sparse_index];
        rb.dimensions.width  = tbl->width    [sparse_index];
        rb.dimensions.height = tbl->height   [sparse_index];
    }
};
