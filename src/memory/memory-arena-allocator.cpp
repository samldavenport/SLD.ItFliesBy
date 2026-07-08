#pragma once

#include "memory.hpp"

namespace ifb {

    void arena_allocator::
    init(
        memory&   mem,
        const u32 granularity) {

        assert(mem.address != 0 && mem.size != 0);

        _mem.ptr           = pfm_memory_commit(mem.ptr, 0, mem.size);
        _mem.size          = mem.size;
        _arena_size        = granularity;
        _arena_count_total = _mem.size / granularity;
        _arena_count_free  = _arena_count_total 
        _list.used         = NULL;
        _list.free         = (arena*)(_mem.ptr);
        assert(
            _mem.address       == res.address &&
            _arena_size        != 0           &&
            _arena_count_total != 0
        );

        // first arena
        arena* curr    = NULL;
        arena* prev    = _list.free;
        prev->_alctr    = this;
        prev->_next     = NULL;
        prev->_prev     = NULL;
        prev->_id       = 0;
        prev->_position = 0;
        prev->_save     = 0;

        // remaining arenas
        for (
            u32 index = 1;
                index < _arena_count_total;
              ++index) {

            // current arena
            arena* curr = (arena*)((addr)prev + _arena_size);
            curr->_alctr    = this;
            curr->_next     = NULL;
            curr->_prev     = prev;
            curr->_id       = index;
            curr->_position = 0;
            curr->_save     = 0;

            // update previous arena
            prev->_next = curr;
            prev        = curr;
        }
    }

    void arena_allocator::
    validate(
        void) {

        assert(
            _mem.address       != 0 &&
            _mem.size          != 0 &&
            _arena_count_total != 0 &&
            _arena_size        != 0 &&
            (
                _list.free != NULL ||
                _list.used != NULL
            )
        );
    }

    arena* arena_allocator::
    alloc(
        void) {

        validate();

        arena* a = _list.free;

        if (a != NULL) {

            assert(a->_prev == NULL);

            // get the next free and used
            arena* free_next = a->_next;
            arena* used_next = _list.used;

            // update the free list
            free_next->_prev = NULL;
            _list.free = free_next;

            // update the used list
            if (used_next) {
                assert(used_next->_prev == NULL);
                used_next->_prev = a;
            }
            a->_next = used_next;
            _list.used = a;
            --arena_count_free;
        }

        a->validate();
        return(a);

    }

    void arena_allocator::
    free(
        arena* a) {

        validate();
        a->validate();

        arena* used_next = a->_next;
        arena* used_prev = a->_prev;
        arena* free_next = _list.free;

        // update the used list
        if (used_next)       used_next->_prev = used_prev;
        if (used_prev)       used_prev->_next = used_next;
        if (a == _list.used) _list.used       = used_next;

        // update the free list
        if (free_next) {
            assert(free_next->_prev == NULL);
            free_next->_prev = a;
        }

        a->_prev   = NULL;
        a->_next   = free_next;
        _list.free = a;
        ++_arena_count_free;
    }

    arena* arena_allocator::
    arena_from_handle(
        const eng_arena_handle hnd) {
        
        validate();

        const u32  arena_id = hnd.val;
        const addr start  = _mem.address;
        const u32  offset = (arena_id * _arena_size);
        arena*     a      = (arena*)(start + offset);  

        a->validate();
        return(a);
    }
};