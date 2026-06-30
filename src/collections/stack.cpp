#pragma once

#include "ifb-collections.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STATIC METHODS
    //--------------------------------------------------------------------

    static u32 stack::memory_requirement (const u32 size) { }
    static u32 stack::memory_init        (const u32 size, memory& mem) { }

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

    void* stack::
    save(void) {

        validate();
        assert(_save == 0);

        _save = _pos;
        return((void*)(_mem.address + _save));
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

        const u32 size_remaninig = (_mem.size - _pos);
        if (size_remaninig >= size) {

            push_mem = (void*)(_mem.address + _pos);
            _mem.pos += size;
        }
        return(NULL);
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
        const void* save) {

        validate();

        assert(
            save != NULL && 
            save == _save
        );

        const addr addr_save = (addr)save;
        
        _pos  = (u32)(addr_save - _mem.address);
        _save = 0;
    }
};