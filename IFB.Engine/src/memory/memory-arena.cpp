#pragma once

#include "ifb-engine.hpp"
#include "ifb.hpp"
#include "eng-internal.hpp"
#include "ifb-platform.hpp"
#include "memory-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct arena_allocator {
        memory mem;
        u32    arena_size;
        u32    arena_count_total;
        u32    arena_count_free;
        u32    handle_bit;
        bool   is_commit_on_push;
        struct {
            arena* free;
            arena* used;
        } list;
    };

    // the header is at the start of the arena, the memory pushed comes after it
    // size_committed includes the header
    struct arena {
        arena_allocator* alctr;
        arena*           next;
        arena*           prev;
        u32              id;
        u32              position;
        u32              size_committed;
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline hnd_arena arena_alloc_from         (arena_allocator* alctr);
    inline void      arena_allocator_validate (const arena_allocator* alctr);
    inline void      arena_validate           (const arena* a);
    inline arena*    arena_from_handle        (const hnd_arena hnd);
    inline u32       arena_capacity           (const arena* a);
    inline void      arena_commit             (arena* a, const u32 offset_end);
    inline void      arena_decommit           (arena* a);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL hnd_arena
    arena_alloc(
        void) {

        assert(_memory_mngr);
        return(arena_alloc_from(_memory_mngr->arena_alctr));
    }

    IFB_INTERNAL hnd_arena
    arena_large_alloc(
        void) {

        assert(_memory_mngr);
        return(arena_alloc_from(_memory_mngr->arena_alctr_large));
    }

    IFB_INTERNAL void
    arena_free(
        const hnd_arena hnd) {

        arena*           a         = arena_from_handle(hnd);
        arena_allocator* alctr     = a->alctr;
        arena*           used_next = a->next;
        arena*           used_prev = a->prev;
        arena*           free_next = alctr->list.free;

        // update the used list
        if (used_next)             used_next->prev  = used_prev;
        if (used_prev)             used_prev->next  = used_next;
        if (a == alctr->list.used) alctr->list.used = used_next;

        // update the free list
        if (free_next) {
            assert(free_next->prev == NULL);
            free_next->prev = a;
        }
        a->prev          = NULL;
        a->next          = free_next;
        a->position      = 0;
        alctr->list.free = a;
        ++alctr->arena_count_free;

        // large arenas give their memory back
        if (alctr->is_commit_on_push) {
            arena_decommit(a);
        }
    }

    IFB_INTERNAL void
    arena_reset(
        const hnd_arena hnd) {

        arena* a = arena_from_handle(hnd);

        a->position = 0;
    }

    IFB_INTERNAL u32
    arena_save(
        const hnd_arena hnd) {

        arena* a = arena_from_handle(hnd);

        return(a->position);
    }

    IFB_INTERNAL void*
    arena_push(
        const hnd_arena hnd,
        const u32       size) {

        arena* a = arena_from_handle(hnd);

        // make sure we have space
        const u32 space_remaining = (arena_capacity(a) - a->position);
        if (size > space_remaining) {
            return(NULL);
        }

        // large arenas commit as they grow
        const u32 offset     = (a->position + sizeof(arena));
        const u32 offset_end = (offset + size);
        if (offset_end > a->size_committed) {
            arena_commit(a, offset_end);
        }

        // calculate pointer
        const addr start = (addr)a;
        void*      mem   = (void*)(start + offset);

        // update the position and return
        a->position += size;
        return(mem);
    }

    IFB_INTERNAL void
    arena_revert(
        const hnd_arena hnd,
        const u32       save) {

        arena* a = arena_from_handle(hnd);

        assert(save <= a->position);

        a->position = save;
    }

    template<typename t>
    IFB_INTERNAL t*
    arena_push(
        const hnd_arena hnd,
        const u32       count) {

        assert(count != 0);

        const u32 size = count * sizeof(t);
        auto      mem  = (t*)arena_push(hnd, size);

        return(mem);
    }

    IFB_INTERNAL u32
    arena_size_free(
        const hnd_arena hnd) {

        const arena* a         = arena_from_handle(hnd);
        const u32    size_free = (arena_capacity(a) - a->position);

        return(size_free);
    }

    IFB_INTERNAL u32
    arena_size_used(
        const hnd_arena hnd) {

        arena* a = arena_from_handle(hnd);
        return(a->position);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline hnd_arena
    arena_alloc_from(
        arena_allocator* alctr) {

        arena_allocator_validate(alctr);

        // no arenas left
        arena* a = alctr->list.free;
        if (a == NULL) {
            hnd_arena hnd = {INVALID_HANDLE};
            return(hnd);
        }
        assert(a->prev == NULL);

        // update the free list
        arena* free_next = a->next;
        if (free_next) free_next->prev = NULL;
        alctr->list.free = free_next;

        // update the used list
        arena* used_next = alctr->list.used;
        if (used_next) {
            assert(used_next->prev == NULL);
            used_next->prev = a;
        }
        a->next          = used_next;
        alctr->list.used = a;
        --alctr->arena_count_free;

        a->position = 0;

        hnd_arena hnd = {a->id | alctr->handle_bit};
        return(hnd);
    }

    inline void
    arena_allocator_validate(
        const arena_allocator* alctr) {

        assert(
            alctr                    != NULL &&
            alctr->mem.address       != 0    &&
            alctr->mem.size          != 0    &&
            alctr->arena_count_total != 0    &&
            alctr->arena_size        != 0    &&
            (
                alctr->list.free != NULL ||
                alctr->list.used != NULL
            )
        );
    }

    inline void
    arena_validate(
        const arena* a) {

        assert(
            _memory_mngr != NULL &&
            a            != NULL &&
            (
                a->alctr == _memory_mngr->arena_alctr ||
                a->alctr == _memory_mngr->arena_alctr_large
            )
        );

        const arena_allocator* arena_alctr  = a->alctr;
        const u32              arena_offset = (a->id * arena_alctr->arena_size);
        const void*            arena_vptr   = (void*)(arena_alctr->mem.address + arena_offset);

        assert(
            a                 == (arena*)arena_vptr             &&
            a->id             <  arena_alctr->arena_count_total &&
            a->position       <= arena_capacity(a)              &&
            a->size_committed <= arena_alctr->arena_size
        );
    }

    inline arena*
    arena_from_handle(
        const hnd_arena hnd) {

        assert(_memory_mngr);

        const bool             is_large = (hnd.val & ARENA_HANDLE_LARGE_BIT) != 0;
        const u32              arena_id = (hnd.val & ~ARENA_HANDLE_LARGE_BIT);
        const arena_allocator* alctr    = is_large
            ? _memory_mngr->arena_alctr_large
            : _memory_mngr->arena_alctr;

        arena_allocator_validate(alctr);
        assert(arena_id < alctr->arena_count_total);

        const addr   start  = alctr->mem.address;
        const u32    offset = (arena_id * alctr->arena_size);
        arena*       a      = (arena*)(start + offset);

        arena_validate(a);
        return(a);
    }

    // the header takes up the start of the arena
    inline u32
    arena_capacity(
        const arena* a) {

        return(a->alctr->arena_size - sizeof(arena));
    }

    // commits up to offset_end, rounded up to the commit granularity
    inline void
    arena_commit(
        arena*    a,
        const u32 offset_end) {

        const arena_allocator* alctr = a->alctr;
        assert(alctr->is_commit_on_push);

        const config& cfg         = config_instance();
        const u32     granularity = cfg.arena_large_commit_granularity;
        const u32     size_target = ((offset_end + granularity - 1) / granularity) * granularity;
        const u32     size_commit = (size_target < alctr->arena_size) ? size_target : alctr->arena_size;

        void* cmt = pfm_memory_commit(a, a->size_committed, size_commit - a->size_committed);
        assert(cmt != NULL);

        a->size_committed = size_commit;
    }

    // everything but the page with the header is decommitted
    inline void
    arena_decommit(
        arena* a) {

        const u32 size_page = system_get_memory_page_size();
        if (a->size_committed <= size_page) return;

        const void* body = (void*)((addr)a + size_page);
        pfm_memory_decommit(body, a->size_committed - size_page);

        a->size_committed = size_page;
    }
};
