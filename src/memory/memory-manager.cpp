#pragma once

#include "memory.hpp"
#include "eng-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void init_arena_allocator (arena_allocator* alctr, memory& res, const u32 granularity);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

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

        const config& cfg = config_instance();

        init_arena_allocator(
            _memory_mngr->arena_alctr,
            mem_reserved_arenas,
            cfg.arena_granularity
        );
          
    }

    IFB_INTERNAL void
    memory_mngr_shutdown(
        void) {

    }

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    init_arena_allocator(
        arena_allocator* alctr,
        memory&          res,
        const u32 granularity) {

        assert(
            alctr       != NULL &&
            res.address != 0    &&
            res.size    != 0
        );

        // initial properties
        alctr->mem.ptr           = pfm_memory_commit(res.ptr, 0, res.size);
        alctr->mem.size          = res.size;  
        alctr->arena_size        = granularity;
        alctr->arena_count_total = res.size / granularity;
        alctr->arena_count_free  = alctr->arena_count_total; 
        alctr->list.used         = NULL;
        alctr->list.free         = (arena*)(alctr->mem.ptr); 
        assert(
            alctr->mem.address       == res.address &&
            alctr->arena_size        != 0           &&
            alctr->arena_count_total != 0
        );

        // first arena
        arena* curr = NULL;
        arena* prev = alctr->list.free;
        prev->alctr    = alctr;
        prev->next     = NULL;
        prev->prev     = NULL;
        prev->id       = 0;
        prev->position = 0;
        prev->save     = 0;

        // remaining arenas
        for (
            u32 index = 1;
                index < alctr->arena_count_total;
              ++index) {

            // current arena
            arena* curr = (arena*)((addr)prev + alctr->arena_size);
            curr->alctr    = alctr;
            curr->next     = NULL;
            curr->prev     = prev;
            curr->id       = index;
            curr->position = 0;
            curr->save     = 0;

            // update previous arena
            prev->next = curr;
            prev       = curr;
        }
    }
};