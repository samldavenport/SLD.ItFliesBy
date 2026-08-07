#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_table_spring_validate(
        void) {

        assert(_cmpnt_mngr);
        assert(_cmpnt_mngr->tables.spring);

        auto tbl = _cmpnt_mngr->tables.spring;

        assert(tbl->anchor);
        assert(tbl->damping);
        assert(tbl->rest_length);
        assert(tbl->stiffness);
    }   

    IFB_INTERNAL void
    cmpnt_table_spring_update(
        const u32     sparse_index,
        const spring& s) {

        cmpnt_table_spring_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.spring;
        tbl->anchor      [sparse_index] = s.anchor;
        tbl->damping     [sparse_index] = s.damping;
        tbl->rest_length [sparse_index] = s.rest_length;
        tbl->stiffness   [sparse_index] = s.stiffness;

    }

    IFB_INTERNAL void
    cmpnt_table_spring_lookup(
        const u32 sparse_index,
        spring&   s) {

        cmpnt_table_spring_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);
        
        auto tbl = _cmpnt_mngr->tables.spring;
        s.anchor      = tbl->anchor      [sparse_index];
        s.damping     = tbl->damping     [sparse_index];
        s.rest_length = tbl->rest_length [sparse_index];
        s.stiffness   = tbl->stiffness   [sparse_index];
    }
};
