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
        const cmpnt_quad& quad) {

        cmpnt_table_quad_validate();
        assert(quad.sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.quad;

        tbl->height [quad.sparse_index] = quad.height;
        tbl->width  [quad.sparse_index] = quad.width;
    }

    IFB_INTERNAL void
    cmpnt_table_quad_update(
        const dimensions_2d& dims,
        const u32            sparse_index) {

        cmpnt_table_quad_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.quad;

        tbl->height [sparse_index] = dims.height;
        tbl->width  [sparse_index] = dims.width;
    }

    IFB_INTERNAL void
    cmpnt_table_quad_lookup(
        cmpnt_quad*   quad,
        const entity& e) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > e.index_sparse
        );

        auto tbl = _cmpnt_mngr->tables.quad;
        assert(tbl != NULL);

        quad->id            = e.id;
        quad->sparse_index = e.index_sparse;
        quad->width        = tbl->width[e.index_sparse];
        quad->height       = tbl->height[e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_table_quad_lookup(
        dimensions_2d& dims,
        const u32      sparse_index) {

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
