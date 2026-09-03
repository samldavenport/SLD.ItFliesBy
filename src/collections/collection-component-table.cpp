#pragma once

#include "ifb-collections.hpp"
#include "ifb-config.hpp"

#define CMPNT_TBL_FUNC template<typename t> auto component_table<t>::

namespace ifb {

    CMPNT_TBL_FUNC 
    stack_init(stack& s) -> void {

        static const u32 capacity = config_instance().entity_capacity;
        _cmpnt = s.push_struct<t>(capacity);
        assert(_cmpnt != NULL);
    }

    CMPNT_TBL_FUNC
    lookup(
        const u32 sparse_index,
        t& cmpnt) -> void {

        static const u32 capacity = config_instance().entity_capacity;
        assert(_cmpnt);
        assert(sparse_index < capacity);

        cmpnt = _cmpnt[sparse_index];
    }

    CMPNT_TBL_FUNC
    update(
        const u32 sparse_index,
        const t&  cmpnt) -> void {

        static const u32 capacity = config_instance().entity_capacity;
        assert(_cmpnt);
        assert(sparse_index < capacity);

        _cmpnt[sparse_index] = cmpnt;
    }
};
