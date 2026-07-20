#pragma once

#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline void arena_allocator_validate (void);
    inline void arena_validate           (const arena* a);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL arena*
    arena_alloc(
        void) {

        arena_allocator_validate();

        arena_allocator* alctr = _memory_mngr->arena_alctr;
        arena*           a     = alctr->list.free;

        if (a != NULL) {

            assert(a->prev == NULL);

            // get the next free and used
            arena* free_next = a->next;
            arena* used_next = alctr->list.used;

            // update the free list
            free_next->prev = NULL;
            alctr->list.free = free_next;

            // update the used list
            if (used_next) {
                assert(used_next->prev == NULL);
                used_next->prev = a;
            }
            a->next = used_next;
            alctr->list.used = a;
            --alctr->arena_count_free;
        }

        a->save     = 0;
        a->position = 0;
        return(a);
    }


    IFB_INTERNAL arena*
    arena_from_handle(
        const eng_arena_handle hnd) {

        arena_allocator_validate();

        const u32              arena_id = hnd.val;
        const arena_allocator* alctr    = _memory_mngr->arena_alctr;

        const addr   start  = alctr->mem.address;
        const u32    offset = (arena_id * alctr->arena_size);
        arena*       a      = (arena*)(start + offset);  

        arena_validate(a);
        return(a);
    }


    IFB_INTERNAL void
    arena_free(
        arena* a) {

        arena_allocator_validate();
        arena_validate(a);

        arena_allocator* alctr     = _memory_mngr->arena_alctr;
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
        alctr->list.free = a;
        ++alctr->arena_count_free;
    }

    IFB_INTERNAL void
    arena_reset(
        arena* a) {

        arena_allocator_validate();
        arena_validate(a);

        a->position = 0;
        a->save     = 0;
    }

    IFB_INTERNAL u32
    arena_save(
        arena* a) {

        arena_allocator_validate();
        arena_validate(a);

        assert(a->save == 0);
        a->save = a->position;
        return(a->save);
    }

    IFB_INTERNAL void*
    arena_push(
        arena*    a,
        const u32 size) {

        arena_allocator_validate();
        arena_validate(a);
        
        const arena_allocator* alctr           = _memory_mngr->arena_alctr;
        const u32              space_remaining = (alctr->arena_size - a->position); 

        // make sure we have space        
        if (size > space_remaining) {
            return(NULL);
        }

        // calculate pointer
        const u32  offset = (a->position + sizeof(arena));
        const addr start  = (addr)a;
        void*      mem    = (void*)(start + offset);

        // update the position and return
        a->position += size;
        return(mem);
    }

    IFB_INTERNAL void
    arena_revert(
        arena*    a,
        const u32 save) {

        arena_allocator_validate();
        arena_validate(a);

        assert(a->save == save);

        a->position = a->save;
        a->save     = 0;
    }

    IFB_INTERNAL void
    arena_commit(
        arena*    a,
        const u32 save) {

        arena_allocator_validate();
        arena_validate(a);

        assert(save == a->save);
        a->save = 0;
    }

    template<typename t>
    IFB_INTERNAL t*
    arena_push(
        arena*    a,
        const u32 count) {

        assert(count != 0);
        
        const u32 size = count * sizeof(t);
        auto      mem  = (t*)arena_push(a, size);

        return(mem);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline void
    arena_allocator_validate(
        void) {

        assert(_memory_mngr);

        arena_allocator* alctr = _memory_mngr->arena_alctr;
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
            a               != NULL &&
            a->alctr        == _memory_mngr->arena_alctr
        );

        const arena_allocator* arena_alctr  = a->alctr;
        const u32              arena_offset = (a->id * arena_alctr->arena_size); 
        const void*            arena_vptr   = (void*)(arena_alctr->mem.address + arena_offset); 

        assert(
            a           == (arena*)arena_vptr             &&
            a->id       <  arena_alctr->arena_count_total &&
            a->save     <= a->position                    &&
            a->position <  arena_alctr->arena_size
        );
    }


};