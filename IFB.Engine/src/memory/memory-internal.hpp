#ifndef MEMORY_INTERNAL_HPP
#define MEMORY_INTERNAL_HPP

#include "ifb-memory.hpp"

#define ifb_arena_push_struct(h_arena, type)        (type*)arena_push(h_arena, sizeof(type))
#define ifb_arena_push_array(h_arena, count, type)  (type*)arena_push(h_arena, count * sizeof(type))

// large arena handles have the high bit set, so every arena method works with either kind
#define ARENA_HANDLE_LARGE_BIT 0x80000000

namespace ifb {

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    struct memory_mngr {
        arena_allocator* arena_alctr;
        arena_allocator* arena_alctr_large;
    } static * _memory_mngr;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL memory_mngr* memory_mngr_create   (void);
    IFB_INTERNAL void         memory_mngr_startup  (reservation* res, reservation* res_large);
    IFB_INTERNAL void         memory_mngr_shutdown (void);

    IFB_INTERNAL reservation* reservation_create              (const memory& mem);
    IFB_INTERNAL void*        reservation_push_bytes          (reservation* res, const u32 size_min); 
    IFB_INTERNAL void*        reservation_push_pages          (reservation* res, const u32 page_count); 
    IFB_INTERNAL void*        reservation_push_all            (reservation* res);
    IFB_INTERNAL void*        reservation_claim_all           (reservation* res);
    IFB_INTERNAL stack*       reservation_push_stack_bytes    (reservation* res, const u32 size_min);
    IFB_INTERNAL stack*       reservation_push_stack_pages    (reservation* res, const u32 page_count);
    IFB_INTERNAL stack*       reservation_push_stack_all      (reservation* res);
    IFB_INTERNAL void         reservation_decommit            (reservation* res);
    IFB_INTERNAL u32          reservation_get_size_used       (const reservation* res);
    IFB_INTERNAL u32          reservation_get_page_count_used (const reservation* res);
    IFB_INTERNAL u32          reservation_get_capacity        (const reservation* res);
    IFB_INTERNAL u32          reservation_get_page_capacity   (const reservation* res);

    IFB_INTERNAL hnd_arena    arena_alloc      (void);
    IFB_INTERNAL hnd_arena    arena_large_alloc(void);
    IFB_INTERNAL void         arena_free       (const hnd_arena arena);
    IFB_INTERNAL void         arena_reset      (const hnd_arena arena);
    IFB_INTERNAL u32          arena_save       (const hnd_arena arena);
    IFB_INTERNAL void*        arena_push       (const hnd_arena arena, const u32 size);
    IFB_INTERNAL void         arena_revert     (const hnd_arena arena, const u32 save);
    IFB_INTERNAL u32          arena_size_free  (const hnd_arena arena);
    IFB_INTERNAL u32          arena_size_used  (const hnd_arena arena);
    template<typename t>
    IFB_INTERNAL t*           arena_push       (const hnd_arena arena, const u32 count = 1);

    IFB_INTERNAL u32          block_alctr_mem_requriement (const u32 granularity, const u32 block_count);
    IFB_INTERNAL void         block_alctr_init            (block_allocator* alctr, memory mem, const u32 granularity);
    IFB_INTERNAL void*        block_alloc                 (block_allocator* alctr);
    IFB_INTERNAL void         block_free                  (void* mem);
};

#endif //MEMORY_INTERNAL_HPP
