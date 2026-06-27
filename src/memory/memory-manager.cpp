#pragma once

#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void init_arena_allocator (arena_allocator* alctr, memory& res, const u32 granularity);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------


    IFB_INTERNAL u32
    memory_manager_memory_requirement(
        void) {

        const u32 size_mngr        = sizeof(memory_manager);
        const u32 size_arena_alctr = sizeof(arena_allocator);
        const u32 size_mem_req = (
            size_mngr + 
            size_arena_alctr            
        );

        return(size_mem_req);
    }

    IFB_INTERNAL memory_manager*
    memory_manager_memory_init(
        memory& mem_stack) {

        const u32 mem_req = memory_manager_memory_requirement();
        assert(
            mem_stack.size    == mem_req &&
            mem_stack.address != 0
        );

        zero_memory(mem_stack.ptr, mem_stack.size);
        _memory_manager              = (memory_manager*)mem_stack.ptr;
        _memory_manager->arena_alctr = (arena_allocator*)((addr)_memory_manager + sizeof(memory_manager));

        return(_memory_manager);
    }

    IFB_INTERNAL void
    memory_manager_startup(
        memory& mem_reserved_arenas) {

        assert(
            mem_reserved_arenas.size    != 0 &&
            mem_reserved_arenas.address != 0 
        );

        const ifb_config& cfg = config_instance();

        init_arena_allocator(
            _memory_manager->arena_alctr,
            mem_reserved_arenas,
            cfg.arena_granularity
        );
          
    }

    IFB_INTERNAL void
    memory_manager_shutdown(
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
        alctr->mem.ptr     = pfm_memory_commit(res.ptr, 0, res.size);
        alctr->mem.size    = res.size;  
        alctr->arena_size  = granularity;
        alctr->arena_count = res.size / granularity;
        alctr->list.used   = NULL;
        alctr->list.free   = (arena*)(alctr->mem.ptr); 
        assert(
            alctr->mem.address == res.address &&
            alctr->arena_size  != 0 &&
            alctr->arena_count != 0
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
                index < alctr->arena_count;
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