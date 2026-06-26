#ifndef IFB_MEMORY_HPP
#define IFB_MEMORY_HPP

#include "ifb.hpp"

namespace ifb {

    struct memory_manager;
    struct arena_allocator;
    struct arena;

    IFB_INTERNAL u32             memory_manager_memory_requirement (void);
    IFB_INTERNAL memory_manager* memory_manager_memory_init        (memory& stack_mem);
    IFB_INTERNAL void            memory_manager_startup            (memory& reserved_mem);
    
    IFB_INTERNAL arena* arena_alloc  (void);
    IFB_INTERNAL void   arena_free   (arena* a);
    IFB_INTERNAL void*  arena_push   (arena* a, u32 size, u32 alignment = 0);
    IFB_INTERNAL u32    arena_save   (arena* a);
    IFB_INTERNAL void   arena_revert (arena* a);
    IFB_INTERNAL void   arena_reset  (arena* a);


    struct memory_manager {
        arena_allocator arena_alctr;
    } static * _memory_manager;

    struct arena_allocator {
        memory mem;
        u32    granularity;
        struct {
            arena* free;
            arena* used;
        } arena_list;
    };

    struct arena {
        arena_allocator* alctr;
        arena*           next;
        arena*           prev;
        u32              id;
        u32              pos;
        u32              save;
    };
};

#endif //IFB_MEMORY_HPP
