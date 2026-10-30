#pragma once

#include <cassert>

#include "eng-events.hpp"
#include "eng-memory.hpp"

namespace ifb {

    IFB_INTERNAL void*
    event_mngr_alloc(
        const u32 size) {

        assert(size        != 0);
        assert(_event_mngr != NULL);

        void* mem = reservation_push_bytes(_event_mngr->res, size);

        return(mem);
    }
};
