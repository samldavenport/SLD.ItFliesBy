#pragma once

#include "map-internal.hpp"
#include "ifb-types.hpp"
#include "sld.hpp"
#include "map.hpp"
#include "map-memory.cpp"
#include <cassert>
#include <sld-strings.hpp>

namespace ifb {

    IFB_INTERNAL map_handle
    map_create(
        const cchar* map_name) {

        assert(map_name);

        map_memory_stack* stack = map_memory_stack_alloc(_map_mngr->memory);
        if (!stack) return(INVALID_HANDLE);

        map* m = map_memory_stack_push_map(_map_mngr->memory, stack);
        if (!m) return(INVALID_HANDLE);

        const u32 name_length = cstr_c16_init(&m->name, map_name);
        assert(name_length != 0);
    
        const handle h = map_memory_handle_from_ptr(_map_mngr->memory, (void*)m);
        m->hnd.val = h.val;

        return(m->hnd);
    }

    IFB_INTERNAL void
    map_destroy(
        const map_handle map_hnd) {

    }
};
