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

        return(INVALID_HANDLE);
    }

    IFB_INTERNAL void
    map_destroy(
        const map_handle map_hnd) {

    }
};
