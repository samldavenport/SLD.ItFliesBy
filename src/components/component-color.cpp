#pragma once

#include "component.hpp"
#include "entity.cpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_color_validate(
        void) {

        assert(
            _cmpnt_mngr               != NULL &&
            _cmpnt_mngr->capacity     != 0    &&
            _cmpnt_mngr->tables.color != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_table_color_update(
        const u32             sparse_index,
        const color_rgba_u32& color) {
        
        cmpnt_table_color_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.color;

        tbl->rgba_hex[sparse_index] = color.hex;
    }

    IFB_INTERNAL void
    cmpnt_table_color_lookup(
        const u32       sparse_index,
        color_rgba_u32& color) {

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
