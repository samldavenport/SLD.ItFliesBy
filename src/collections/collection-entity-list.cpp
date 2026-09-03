#pragma once

#include "collections-internal.hpp"
#include "entity.hpp"
#include "ifb-types.hpp"

namespace ifb {

    bool
    entity_list::arena_init(
        const eng_arena_handle a) {

        const u32 save = arena_save(a);

        const auto& cfg = config_instance();

        _ids      = arena_push<entity_id>(a, cfg.entity_capacity);
        _capacity = cfg.entity_capacity;
        _count    = 0;

        const bool did_init = (_ids != NULL);
        if (did_init) arena_commit(a, save);
        else          arena_revert(a, save);
    
        return(did_init);
    }

    bool
    entity_list::stack_init(
        stack& s) {

        const auto& cfg = config_instance();
        
        _ids      = s.push_struct<entity_id>(cfg.entity_capacity);
        _capacity = cfg.entity_capacity;
        _count    = 0;
   
        return(_ids != NULL);
    }

    bool
    entity_list::add(
        const entity_id id) {

        validate();
        assert(id != ENTITY_ID_INVALID);

        if (_count == _capacity) {
            return(false);
        }

        const bool can_add = !contains(id);
        if (can_add) {
            _ids[_count++] = id;
        }
        return(can_add);
    }

    bool
    entity_list::remove(
        const entity_id id) {

        validate();
        assert(id != ENTITY_ID_INVALID);

        // check if the last element is the id
        if (id == _ids[_count - 1]) {
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
            if (id == _ids[i]) {
                _ids[i] = _ids[_count - 1];
                return(true);
            }
        }

        // we didn't find it
        return(false);
    }

    void
    entity_list::reset(
        void) {

        validate();
        _count = 0;
    }

    void
    entity_list::validate(
        void) const {

        assert(_ids      != NULL);
        assert(_capacity != 0);
        assert(_count    <= _capacity);
    }

    u32
    entity_list::capacity (
        void) const {

        validate();
        return(_capacity);
    }

    u32
    entity_list::count(
        void) const {

        validate();
        return(_count);
    }

    bool
    entity_list::contains(
        const entity_id id) const {

        validate();
        assert(id != ENTITY_ID_INVALID);

        bool does_contain = false;
        for (
            u32 i = 0;
            i < _count;
            ++i
        ) {
            if (id == _ids[i]) {
                does_contain = true;
                break;
            }
        }

        return(does_contain);
    }
};
