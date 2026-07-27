#pragma once

#include "collections-internal.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"

namespace ifb {
    
    bool
    entity_id_list::arena_init(
        arena* a) {

        assert(a);
        
        const auto& config = config_instance();

        _array    = arena_push<entity_id>(a, config.entity_capacity);
        _capacity = config.entity_capacity;
        _count    = 0;

        return(_array != NULL);
    }

    bool
    entity_id_list::stack_init(
        stack& s) {

        const auto& config = config_instance();
    
        _array    = s.push_struct<entity_id>(config.entity_capacity);
        _capacity = config.entity_capacity;
        _count    = 0;
   
        return(_array != NULL);
    }

    bool
    entity_id_list::add(
        const entity_id id) {

        validate();
        assert(id != ENTITY_ID_INVALID);

        if (_count == _capacity) {
            return(false);
        }

        for (
            u32 i = 0;
            i < _count;
            ++i
        ) {
            if (id == _array[i]) {
                return(false);
            }
        }

       _array[_count++] = id;
   
        return(true);
    }
    
    bool
    entity_id_list::remove(
        const entity_id id) {

        validate();
        assert(id != ENTITY_ID_INVALID);

        // check if the last element is the id
        if (id == _array[_count - 1]) {
            --_count;
            return(true);
        }

        // search the list for the id
        for (
            u32 i = 0;
            i < _count - 1;
            ++i
        ) {
            // if we find it,
            // swap it with the last element,
            // reduce the count and return
            if (id == _array[i]) {
                _array[i] = _array[_count - 1];
                return(true);
            }
        }

        // we didn't find it
        return(false);
    }
    
    void
    entity_id_list::reset(
        void) {

        validate();
        _count = 0;
    }
    
    u32
    entity_id_list::capacity(
        void) const {

        return(_capacity); 
    }
    
    u32
    entity_id_list::count(
        void) const {

        return(_count);
    }
    
    bool
    entity_id_list::contains(
        const entity_id id) const {
    
        assert(id != ENTITY_ID_INVALID);
        validate();

        for (
            u32 i = 0;
            i < _count;
            ++i
        ) {
            if (id == _array[i]) {
                return(true);
            }
        }

        return(false);
    }
    
    void
    entity_id_list::validate(
        void) const {
    
        assert(_array    != NULL);
        assert(_capacity != 0);
        assert(_count    <= _capacity);
    }

};
