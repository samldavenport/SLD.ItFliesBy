#pragma once

#include "ifb-collections.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STATIC METHODS
    //--------------------------------------------------------------------

    void
    stack::init(
        memory& mem) {

        assert(
            mem.size    != 0 &&
            mem.address != 0
        );

        zero_memory(mem);
        _mem.size    = mem.size;        
        _mem.address = mem.address;
    }

    //--------------------------------------------------------------------
    // PUBLIC METHODS
    //--------------------------------------------------------------------

    void stack::
    validate(
        void) const {

        assert(
            _mem.address != 0         &&
            _mem.size    != 0         &&
            _pos         <= _mem.size &&
            _save        <= _pos
        );
    }

    u32 stack::
    size_total(void) const {

        validate();
        return(_mem.size);
    }

    u32 stack::
    size_used(void) const {

        validate();
        return(_pos);
    }

    u32 stack::
    size_free(void) const {

        validate();
        return(_mem.size - _pos);
    }

    void* stack::
    head(void) const {

        validate();
        return((void*)(_mem.address + _pos));
    }

    void* stack::
    tail(void) const {

        validate();
        return(_mem.ptr);
    }

    u32 stack::
    save(void) {

        validate();
        assert(_save == 0);

        _save = _pos;
        return(_save);
    }

    void stack::
    reset(void) {

        validate();
        _pos  = 0;
        _save = 0;
    }

    void* stack::
    push(
        const u32 size) {

        validate();

        void* push_mem = NULL;

        const u32 new_position = (_pos + size);
        if (new_position <= _mem.size) {

            push_mem = (void*)(_mem.address + _pos);
            _pos = new_position;
        }

        return(push_mem);
    }

    void stack::
    pull(
        const u32 size) {

        validate();

        if (size <= _pos) {
            _pos -= size;
        }
    }

    void stack::
    revert(
        const u32 save) {

        validate();

        assert(save == _save);

        _pos  = _save;
        _save = 0;

    }

    template<typename t>
    t* stack:: 
    push_struct(
        const u32 count) {

        const u32 size = (count * sizeof(t));
        auto      mem  = (t*)push(size);
        assert(mem);

        return(mem);
    }
};