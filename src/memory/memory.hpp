#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "ifb-engine.hpp"
#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct memory_mngr;
    struct arena_allocator;
    struct stack_allocator;
    struct block_allocator;
    struct global_allocator;
    struct arena;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL memory_mngr* memory_mngr_create   (void);
    IFB_INTERNAL void         memory_mngr_startup  (memory& mem_reserved_arenas);
    IFB_INTERNAL void         memory_mngr_shutdown (void);

    IFB_INTERNAL arena_handle arena_alloc      (void);
    IFB_INTERNAL void             arena_free       (const arena_handle arena);
    IFB_INTERNAL void             arena_reset      (const arena_handle arena);
    IFB_INTERNAL u32              arena_save       (const arena_handle arena);
    IFB_INTERNAL void*            arena_push       (const arena_handle arena, const u32 size);
    IFB_INTERNAL void             arena_revert     (const arena_handle arena, const u32 save);
    IFB_INTERNAL void             arena_commit     (const arena_handle arena, const u32 save);        
    IFB_INTERNAL u32              arena_size_free  (const arena_handle arena);
    IFB_INTERNAL u32              arena_size_used  (const arena_handle arena);

    template<typename t>
    IFB_INTERNAL t*           arena_push           (const arena_handle arena, const u32 count = 1);

    IFB_INTERNAL u32          block_alctr_mem_requriement (const u32 granularity, const u32 block_count);
    IFB_INTERNAL void         block_alctr_init            (block_allocator* alctr, memory mem, const u32 granularity);
    IFB_INTERNAL void*        block_alloc                 (block_allocator* alctr);
    IFB_INTERNAL void         block_free                  (void* mem);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct arena_allocator {
        memory mem;
        u32    arena_size;
        u32    arena_count_total;
        u32    arena_count_free;
        struct {
            arena* free;
            arena* used;
        } list;
    };

    struct block_allocator {
        addr start;
        u32  block_size;
        u32  block_count_total;
        u32  block_mask_hi;
        u32  block_mask_lo;
    };

    struct block_memory {
        block_allocator* alctr;
        u32              id;
    };

    struct arena {
        arena_allocator* alctr;
        arena*           next;
        arena*           prev;
        u32              id;
        u32              position;
        u32              save;
    };

    struct memory_mngr {
        arena_allocator* arena_alctr;
    } static * _memory_mngr;
};

#endif //MEMORY_HPP
