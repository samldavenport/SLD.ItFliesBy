#pragma once

#include "cartographer.hpp"
#include "cartographer-internal.cpp"

namespace ifb {

    IFB_INTERNAL map*
    cartographer_map_create(
        void) {

        map* m = cartographer_memory_alloc_map(); 
        return(m);
    }    

    IFB_INTERNAL void
    cartographer_map_destroy(
        map* m) {

    }

    IFB_INTERNAL void
    cartographer_map_add_tile(
        tile* t) {

    }

    IFB_INTERNAL void
    cartographer_map_remove_tile(
        tile* t) {

    }
};
