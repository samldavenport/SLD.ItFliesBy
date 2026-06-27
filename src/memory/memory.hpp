#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct memory_manager;
    struct arena_allocator;
    struct stack_allocator;
    struct block_allocator;
    struct arena;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL u32    memory_manager_memory_requirement (const u32 allocator_count);
    IFB_INTERNAL void   memory_manager_memory_init        (const u32 allocator_count, memory& mem_stack);
    IFB_INTERNAL void   memory_manager_startup            (memory& mem_reserved);
    IFB_INTERNAL void   memory_manager_shutdown           (void);

    IFB_INTERNAL arena* arena_alloc       (void);
    IFB_INTERNAL arena* arena_from_handle (const eng_arena_handle hnd);
    IFB_INTERNAL void   arena_free        (arena* a);
    IFB_INTERNAL void   arena_reset       (arena* a);
    IFB_INTERNAL u32    arena_save        (arena* a);
    IFB_INTERNAL void*  arena_push        (arena* a, const u32 size);
    IFB_INTERNAL void   arena_revert      (arena* a, const u32 save);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct arena_allocator {
        memory mem;
        u32    arena_size;
        u32    arena_count;
        struct {
            arena* free;
            arena* used;
        } list;
    };

    struct arena {
        arena_allocator* alctr;
        arena*           next;
        arena*           prev;
        u32              id;
        u32              position;
        u32              save;
    };

    struct memory_manager {
        arena_allocator* arena_alctr;
    } static * _memory_manager;
};

#endif //MEMORY_HPP