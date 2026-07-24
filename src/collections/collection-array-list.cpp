#pragma once

#include "ifb-collections.hpp"

#define ARRAY_LIST_FUNC template<typename t> auto array_list<t>::

namespace ifb {
    
    ARRAY_LIST_FUNC
    init(
        t*        elmnts,
        const u32 capacity) -> void {

        assert(elmnts != NULL && capacity != 0);

        _elmnt    = elmnts;
        _capacity = capacity;
        _count    = 0;

        reset();
    }

    ARRAY_LIST_FUNC
    validate (void) const -> void {

        assert(
            _elmnt    != NULL &&
            _capacity != 0    &&
            _count    <= _capacity
        );
    }

    ARRAY_LIST_FUNC
    get(
        const u32 index) -> t& {

        validate();

        assert(index < _count);

        return(_elmnt[index]);
    }

    ARRAY_LIST_FUNC 
    capacity(
        void) const -> u32 {

        validate();
        return(_capacity);
    }

    ARRAY_LIST_FUNC 
    count(
        void) const -> u32 {
            
        validate();
        return(_count);
    }

    ARRAY_LIST_FUNC 
    is_full(
        void) const -> bool {

        validate();
        return(_count == _capacity);
    }

    ARRAY_LIST_FUNC
    index_of(
        const t& elmnt,
        u32&     index) const -> bool  {

        validate();
        index = INVALID_INDEX


        // if there are no elements, nothing to do
        if (_count == 0) {
            return(false);
        }

        const u32 size_elmnt   = sizeof(t);
        const addr addr_elmnt = (addr)&elmnt;
        const addr addr_first = (addr)_elmnt[0];
        const addr addr_last  = (addr)_elmnt[_count - 1];

        // if the element is not in the address space or
        // not a multiple of the element size,
        // nothing to do
        if (
            addr_elmnt < addr_first ||
            addr_elmnt > addr_last  ||
            addr_elmnt % size_elmnt != 0
        ) {
            return(false);
        } 

        // sanity check
        // the first and last offsets should
        // be a multiple of the element size
        const u32 offset_first = 0;
        const u32 offset_last  = (addr_last - addr_first); 
        assert(
            offset_first % size_elmnt == 0 &&
            offset_last  % size_elmnt == 0
        );

        // calculate the index and return
        const u32 offset_elmnt = (addr_elmnt - addr_first);
        const u32 index_elmnt  = (offset_elmnt / size_elmnt);  
        return(index_elmnt);
    }

    ARRAY_LIST_FUNC
    contains(
        const t& elmnt)const -> bool {

        validate();

        bool does_contain = false;
        for (
            u32 i = 0;
            i < _count;
            ++i) {

            if (elmnt == _elmnt[i]) {
                does_contain = true;
                break;
            }
        }

        return(does_contain);
    }


    ARRAY_LIST_FUNC
    reset(
        void) -> void {

        validate();
        _count = 0;
    }


    ARRAY_LIST_FUNC
    add(
        const t&  elmnt) -> bool {

        validate();

        if (_count == _capacity) {
            return(false);
        }

        _elmnt[_count++] = elmnt;
        return(true);
    }

    ARRAY_LIST_FUNC
    remove(
        const t&  elmnt) -> void {

        validate();

        u32 index = 0;
        assert(index_of(elmnt, index));

        remove_at(index);
    }

    ARRAY_LIST_FUNC
    remove_at(
        const u32 index) -> void {

        validate();

        assert(index < _count);

        const u32 index_last = (_count - 1); 

        if (index != index_last) {
            _elmnt[index] = _elmnt[index_last];
        }

        --_count;
    }
};
