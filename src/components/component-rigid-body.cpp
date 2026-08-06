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
        const cmpnt_rigid_body& rb) {

        cmpnt_table_rigid_body_validate();
        assert(rb.sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;

        tbl->origin_x [rb.sparse_index] = rb.origin_x;
        tbl->origin_y [rb.sparse_index] = rb.origin_y;
        tbl->origin_z [rb.sparse_index] = rb.origin_z;
        tbl->width    [rb.sparse_index] = rb.width;
        tbl->height   [rb.sparse_index] = rb.height;
    }

    IFB_INTERNAL void
    cmpnt_table_rigid_body_update(
        const rigid_body& rb,
        const u32   sparse_index) {

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
        cmpnt_rigid_body& rb,
        const entity&     e) {

        cmpnt_table_rigid_body_validate();
        assert(e.index_sparse < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;
        assert(tbl != NULL);

        rb.id           = e.id;
        rb.sparse_index = e.index_sparse;
        rb.origin_x     = tbl->origin_x [e.index_sparse];
        rb.origin_y     = tbl->origin_y [e.index_sparse];
        rb.origin_z     = tbl->origin_z [e.index_sparse];
        rb.width        = tbl->width    [e.index_sparse];
        rb.height       = tbl->height   [e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_table_rigid_body_lookup(
        rigid_body& rb,
        const u32   sparse_index) {
            
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
