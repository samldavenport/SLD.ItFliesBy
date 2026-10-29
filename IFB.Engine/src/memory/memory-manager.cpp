#pragma once

#include "ifb.hpp"
#include "eng-context.hpp"
#include "ifb-platform.hpp"
#include "memory-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void init_arena_allocator       (arena_allocator* alctr, reservation* res, const u32 granularity);
    IFB_INLINE void init_arena_allocator_large (arena_allocator* alctr, reservation* res, const u32 granularity);
    IFB_INLINE void init_arena_list            (arena_allocator* alctr, const u32 size_committed);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL memory_mngr*
    memory_mngr_create(
        void) {

        auto mngr              = global_alloc<memory_mngr>();
        auto arena_alctr       = global_alloc<arena_allocator>();
        auto arena_alctr_large = global_alloc<arena_allocator>();
        assert(
            mngr              != NULL &&
            arena_alctr       != NULL &&
            arena_alctr_large != NULL
        );

        _memory_mngr                    = mngr;
        _memory_mngr->arena_alctr       = arena_alctr;
        _memory_mngr->arena_alctr_large = arena_alctr_large;

        return(_memory_mngr);
    }

    IFB_INTERNAL void
    memory_mngr_startup(
        reservation* res,
        reservation* res_large) {

        assert(res);
        assert(res_large);

        const config& cfg = config_instance();

        init_arena_allocator(
            _memory_mngr->arena_alctr,
            res,
            cfg.arena_granularity
        );

        init_arena_allocator_large(
            _memory_mngr->arena_alctr_large,
            res_large,
            cfg.arena_large_granularity
        );
    }

    IFB_INTERNAL void
    memory_mngr_shutdown(
        void) {

    }

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    // every arena is committed up front
    IFB_INLINE void
    init_arena_allocator(
        arena_allocator* alctr,
        reservation*     res,
        const u32        granularity) {

        assert(
            alctr       != NULL &&
            res         != NULL &&
            granularity != 0
        );

        alctr->mem.ptr           = reservation_push_all     (res);
        alctr->mem.size          = reservation_get_capacity (res);
        alctr->arena_size        = granularity;
        alctr->handle_bit        = 0;
        alctr->is_commit_on_push = false;
        assert(alctr->mem.ptr != NULL);

        init_arena_list(alctr, granularity);
    }

    // the arenas are only reserved, the page with each header is committed
    // the rest is committed as the arena grows
    IFB_INLINE void
    init_arena_allocator_large(
        arena_allocator* alctr,
        reservation*     res,
        const u32        granularity) {

        assert(
            alctr       != NULL &&
            res         != NULL &&
            granularity != 0
        );

        const u32 size_page = system_get_memory_page_size();
        assert(granularity % size_page == 0);

        alctr->mem.ptr           = reservation_claim_all    (res);
        alctr->mem.size          = reservation_get_capacity (res);
        alctr->arena_size        = granularity;
        alctr->handle_bit        = ARENA_HANDLE_LARGE_BIT;
        alctr->is_commit_on_push = true;
        assert(alctr->mem.ptr != NULL);

        // commit the headers
        const u32 arena_count = alctr->mem.size / granularity;
        for (
            u32 index = 0;
                index < arena_count;
              ++index) {

            void* cmt = pfm_memory_commit(alctr->mem.ptr, index * granularity, size_page);
            assert(cmt != NULL);
        }

        init_arena_list(alctr, size_page);
    }

    // all the arenas start on the free list
    IFB_INLINE void
    init_arena_list(
        arena_allocator* alctr,
        const u32        size_committed) {

        alctr->arena_count_total = alctr->mem.size / alctr->arena_size;
        alctr->arena_count_free  = alctr->arena_count_total;
        alctr->list.used         = NULL;
        alctr->list.free         = (arena*)(alctr->mem.ptr);
        assert(
            alctr->arena_count_total != 0 &&
            alctr->arena_count_total <  ARENA_HANDLE_LARGE_BIT
        );

        arena* prev = NULL;
        for (
            u32 index = 0;
                index < alctr->arena_count_total;
              ++index) {

            arena* curr = (arena*)(alctr->mem.address + (index * alctr->arena_size));
            curr->alctr          = alctr;
            curr->next           = NULL;
            curr->prev           = prev;
            curr->id             = index;
            curr->position       = 0;
            curr->size_committed = size_committed;

            if (prev) prev->next = curr;
            prev = curr;
        }
    }
};
