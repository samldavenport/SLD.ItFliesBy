#pragma once

#include "memory.hpp"
#include "eng-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void init_arena_allocator (arena_allocator* alctr, reservation* res, const u32 granularity);

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
        reservation* res) {

        assert(res);

        const config& cfg = config_instance();

        init_arena_allocator(
            _memory_mngr->arena_alctr,
            res,
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
        reservation*     res,
        const u32        granularity) {

        assert(
            alctr       != NULL &&
            res         != NULL &
            granularity != 0
        );

        // initial properties
        alctr->mem.ptr           = reservation_push_all     (res); 
        alctr->mem.size          = reservation_get_capacity (res);  
        alctr->arena_size        = granularity;
        alctr->arena_count_total = alctr->mem.size / granularity;
        alctr->arena_count_free  = alctr->arena_count_total; 
        alctr->list.used         = NULL;
        alctr->list.free         = (arena*)(alctr->mem.ptr); 
        assert(
            alctr->mem.ptr           != NULL &&
            alctr->arena_size        != 0    &&
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
