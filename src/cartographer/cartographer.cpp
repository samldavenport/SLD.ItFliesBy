#pragma once

#include "cartographer.hpp"
#include "eng-internal.hpp"
#include "cartographer-internal.cpp"
#include "cartographer-memory.cpp"

namespace ifb {

    IFB_INTERNAL cartographer*
    cartographer_create(
        void) {

        auto c = global_alloc<cartographer>();
        assert(c != NULL);

        return(c);

    }

    IFB_INTERNAL void
    cartographer_startup(
        memory& mem) {
        
    }

    IFB_INTERNAL void
    cartographer_shutdown(
        void) {

    }
};
