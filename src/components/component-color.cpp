#pragma once

#include "component.hpp"
#include "entity.cpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_color_table_validate(
        void) {

        assert(
            _cmpnt_mngr               != NULL &&
            _cmpnt_mngr->capacity     != 0    &&
            _cmpnt_mngr->tables.color != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_color_table_update(
        const cmpnt_color& color) {

        cmpnt_color_table_validate();
        assert(color.sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.color;

        tbl->rgba_hex[color.sparse_index] = color.hex_rgba;
    }

    IFB_INTERNAL void
    cmpnt_color_table_update(
        const color_rgba_u32& color,
        const u32             sparse_index) {
        
        cmpnt_color_table_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.color;

        tbl->rgba_hex[sparse_index] = color.hex;
    }

    IFB_INTERNAL void
    cmpnt_color_table_lookup(
        cmpnt_color&  color,
        const entity& e) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > e.index_sparse
        );

        auto tbl = _cmpnt_mngr->tables.color;
        assert(tbl != NULL);

        color.id           = e.id;
        color.sparse_index = e.index_sparse;
        color.hex_rgba     = tbl->rgba_hex[e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_color_table_lookup(
        color_rgba_u32& color,
        const u32       sparse_index) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.color;
        assert(tbl);

        color.hex = tbl->rgba_hex[sparse_index];

    }
};
