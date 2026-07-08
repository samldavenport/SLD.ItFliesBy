#pragma once

#include "memory.hpp"

namespace ifb {

    void arena::
    validate(
        void) {
        
        assert(
            _alctr == _memory_mngr->arena_alctr
        );

        const arena_allocator* arena_alctr  = _alctr;
        const u32              arena_offset = (_id * arena_alctr->_arena_size); 
        const void*            arena_vptr   = (void*)(arena_alctr->_mem.address + arena_offset); 

        assert(
            a->_id       <  arena_alctr->_arena_count_total &&
            a->_save     <= a->_position                    &&
            a->_position <  arena_alctr->_arena_size
        );
    }

    void arena::
    reset(
        void) {

        validate();
        _alctr->validate();

        _position = 0;
        _save     = 0;
    }

    u32 arena::
    save(
        void) {

        validate();
        _alctr->validate();

        assert(_save = 0);
        _save = _position;
        return(_save);
    }

    void* arena::
    push(
        const u32 size) {

        validate();
        _alctr->validate();
  
        const arena_allocator* alctr           = _alctr;
        const u32              space_remaining = (_alctr->arena_size - _position); 

        // make sure we have space        
        if (size > space_remaining) {
            return(NULL);
        }

        // calculate pointer
        const u32  offset = (_position + sizeof(arena));
        const addr start  = (addr)a;
        void*      mem    = (void*)(start + offset);

        // update the position and return
        _position += size;
        return(mem);
    }

    void arena::
    revert(
        const u32 save) {

        validate();
        _alctr->validate();
  
        assert(_save == save);

        _position = _save;
        _save     = 0;
    }

    void arena::
    commit(
        const u32 save) {

        validate();
        _alctr->validate();
  
        assert(save == _save);
        _save = 0;
    }

    template<typename t>
    t* arena::
    push(
        const u32 count) {

        assert(count != 0);
        
        const u32 size = count * sizeof(t);
        auto      mem  = (t*)push(size);

        return(mem);
    }
};