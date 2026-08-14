#pragma once

#include "cartographer.hpp"
#include "eng-internal.hpp"
#include "cartographer-internal.cpp"
#include "cartographer-memory.cpp"
#include "cartographer-tile.cpp"


namespace ifb {

    IFB_INTERNAL cartographer*
    cartographer_create(
        void) {

        auto c     = global_alloc<cartographer>();
        auto alctr = global_alloc<cartographer_block_allocator>();
        assert(c     != NULL);
        assert(alctr != NULL);

        c->block_alctr = alctr;
        c->first_map   = NULL;

        _cartographer = c;
        return(c);

    }

    IFB_INTERNAL void
    cartographer_startup(
        memory& mem) {
       
        cartographer_memory_init(mem);
    }

    IFB_INTERNAL void
    cartographer_shutdown(
        void) {

    }
};
