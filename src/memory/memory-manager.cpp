#pragma once

#include "memory.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_INTERNAL memory_mngr*
    memory_mngr_create(
        void) {

        auto mngr        = global_alloc<memory_mngr>();
        auto arena_alctr = global_alloc<arena_allocator>(); 
        assert(
            mngr        != NULL &&
            arena_alctr != NULL
        );

        _memory_mngr              = mngr;
        _memory_mngr->arena_alctr = arena_alctr; 

        return(_memory_mngr);
    }

    IFB_INTERNAL void
    memory_mngr_startup(
        memory& mem_reserved_arenas) {

        assert(
            mem_reserved_arenas.size    != 0 &&
            mem_reserved_arenas.address != 0 
        );

        const config&    cfg         = config_instance();
        arena_allocator* arena_alctr = _memory_mngr->arena_alctr;
        assert(arena_alctr);

        arena_alctr->init(
            mem_reserved_arenas,
            cfg.arena_granularity
        );
          
    }

    IFB_INTERNAL void
    memory_mngr_shutdown(
        void) {

    }

    IFB_INTERNAL arena*
    memory_mngr_arena_alloc(
        void) {

        arena_allocator* alctr = _memory_mngr->arena_alctr;
        arena*           a     = alctr->alloc(); 
        return(a);
    }

    IFB_INTERNAL void
    memory_mngr_arena_free(
        arena** a
    ) {
        assert(a && *a);

        alct
    }

    IFB_INTERNAL arena*
    memory_mngr_arena_from_handle (const eng_arena_handle hnd);

};