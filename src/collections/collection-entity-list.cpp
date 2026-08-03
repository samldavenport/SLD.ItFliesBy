#pragma once

#include "collections-internal.hpp"
#include "entity.hpp"
#include "ifb-types.hpp"

namespace ifb {

    bool
    entity_list::arena_init(
        arena* a) {
    
        assert(a);

        const u32 save = arena_save(a);

        const auto& cfg = config_instance();

        _dense_index = arena_push<u32>(a, cfg.entity_capacity);
        _capacity    = cfg.entity_capacity;
        _count       = 0;

        const bool did_init = (_dense_index != NULL);
        if (did_init) arena_commit(a, save);
        else          arena_revert(a, save);
    
        return(did_init);
    }

    bool
    entity_list::stack_init(
        stack& s) {

        const auto& cfg = config_instance();
        
        _dense_index = s.push_struct<u32>(cfg.entity_capacity);
        _capacity    = cfg.entity_capacity;
        _count       = 0;
   
        return(_dense_index != NULL);
    }

    bool
    entity_list::add(
        const u32 dense_index) {

        validate();
        assert(dense_index != INVALID_INDEX);

        if (_count == _capacity) {
            return(false);
        }

        for (
            u32 i = 0;
            i < _count;
            ++i
        ) {
            if (dense_index == _dense_index[i]) {
                return(false);
            }
        }

        _dense_index[_count++] = dense_index;
        return(true);
    }

    bool
    entity_list::remove(
        const u32 dense_index) {

        validate();
        assert(dense_index != INVALID_INDEX);

        // check if the last element is the id
        if (dense_index == _dense_index[_count - 1]) {
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
            if (dense_index == _dense_index[i]) {
                _dense_index[i] = _dense_index[_count - 1];
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

        assert(_dense_index != NULL);
        assert(_capacity    != 0);
        assert(_count       <= _capacity);
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
        const u32 dense_index) const {

        validate();
        assert(dense_index != INVALID_INDEX);

        bool does_contain = false;
        for (
            u32 i = 0;
            i < _count;
            ++i
        ) {
            if (dense_index == _dense_index[i]) {
                does_contain = true;
                break;
            }
        }

        return(does_contain);
    }

    entity_id
    entity_list::get_entity_id(
        const u32 index) const {

        validate();
        assert(index != INVALID_INDEX);
       
        const u32 dense_index = _dense_index[index];
        assert(dense_index < _entity_mngr->count);    

        const entity_id id = _entity_mngr->data.dense.id[dense_index];
        return(id);
    }

    entity_archetype
    entity_list::get_entity_archetype(
        const u32 index) const {

        validate();
        assert(index != INVALID_INDEX);
       
        const u32 dense_index = _dense_index[index];
        assert(dense_index < _entity_mngr->count);    

        const entity_archetype atype = _entity_mngr->data.dense.archetype[dense_index];
        return(atype);
    }

    u32
    entity_list::get_dense_index(
        const u32 index) const {

        validate();
        assert(index != INVALID_INDEX);
       
        const u32 dense_index = _dense_index[index];
        return(dense_index);
    }

    u32
    entity_list::get_sparse_index(
        const u32 index) const {
        
        validate();
        assert(index != INVALID_INDEX);
       
        const u32 dense_index = _dense_index[index];
        assert(dense_index < _entity_mngr->count);    

        const u32 sparse_index = _entity_mngr->data.dense.sparse_index[dense_index];
        return(sparse_index);
    }

    const cchar*
    entity_list::get_tag(
        const u32 index) const {
        
        validate();
        assert(index != INVALID_INDEX);
       
        const u32 dense_index = _dense_index[index];
        assert(dense_index < _entity_mngr->count);    

        const cchar* tag_cstr = _entity_mngr->data.dense.tag[dense_index].cstr;
        return(tag_cstr);
    }
};
