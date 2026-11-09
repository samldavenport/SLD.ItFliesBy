#pragma once

#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "ifb-platform.hpp"
#include "eng-context.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct memory_stats_entry {
        const char*        name;
        const reservation* res;
        bool               is_arena_pool;
    };

    // filled at startup, read only after that
    struct memory_stats_registry {
        u32                count;
        memory_stats_entry entries[ENG_MEM_STATS_RESERVATION_MAX];
    } static _memory_stats;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void memory_stats_arena_pool (arena_allocator* alctr, eng_mem_stats_arena_pool& pool);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    memory_stats_register_reservation(
        const char*        name,
        const reservation* res,
        const bool         is_arena_pool) {

        assert(name);
        assert(res);
        assert(_memory_stats.count < ENG_MEM_STATS_RESERVATION_MAX);

        memory_stats_entry& entry = _memory_stats.entries[_memory_stats.count];
        entry.name          = name;
        entry.res           = res;
        entry.is_arena_pool = is_arena_pool;
        ++_memory_stats.count;
    }

    //--------------------------------------------------------------------
    // API METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_ENGINE_API void
    eng_memory_get_stats(
        eng_mem_stats& stats) {

        // reservations
        stats.reservation_count = _memory_stats.count;
        for (
            u32 index = 0;
                index < _memory_stats.count;
              ++index) {

            const memory_stats_entry&  entry = _memory_stats.entries[index];
            eng_mem_stats_reservation& out   = stats.reservations[index];

            out.name          = entry.name;
            out.size_capacity = reservation_get_capacity  (entry.res);
            out.size_used     = reservation_get_size_used (entry.res);
            out.is_arena_pool = entry.is_arena_pool;
        }

        // arena pools
        assert(_memory_mngr);
        memory_stats_arena_pool(_memory_mngr->arena_alctr,       stats.arenas);
        memory_stats_arena_pool(_memory_mngr->arena_alctr_large, stats.arenas_large);

        // global stack
        global_stack_validate();
        stats.global_stack.size_capacity = _global_stack->size;
        stats.global_stack.size_used     = _global_stack->position;
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the lock keeps the used list stable while we walk it
    // positions and commits can change on the thread that owns the arena, so they're a close read
    IFB_INLINE void
    memory_stats_arena_pool(
        arena_allocator*          alctr,
        eng_mem_stats_arena_pool& pool) {

        arena_allocator_validate(alctr);

        pfm_thread_mutex_lock(alctr->lock);

        u64 size_used           = 0;
        u64 size_committed_used = 0;
        for (
            const arena* a = alctr->list.used;
                         a != NULL;
                         a = a->next) {

            size_used           += a->position;
            size_committed_used += a->size_committed;
        }

        const u32 count_free = alctr->arena_count_free;

        pfm_thread_mutex_unlock(alctr->lock);

        // free small arenas stay fully committed, free large arenas only keep the header page
        const u32 size_committed_free = alctr->is_commit_on_push ? alctr->size_page : alctr->arena_size;

        pool.arena_size        = alctr->arena_size;
        pool.arena_count_total = alctr->arena_count_total;
        pool.arena_count_used  = alctr->arena_count_total - count_free;
        pool.size_capacity     = alctr->mem.size;
        pool.size_used         = size_used;
        pool.size_committed    = size_committed_used + ((u64)count_free * size_committed_free);
    }
};
