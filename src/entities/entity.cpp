#pragma once

#include "entities.hpp"

namespace ifb {
   
    IFB_INTERNAL bool
    entity_lookup_by_archetype(
        entity_list*        id_list,
        const entity_archetype atype
    ) {

        entity_mngr_validate();
        entity_list_validate(id_list);

        u32& id_index = id_list->count;
        id_index = 0;

        for (
            u32 entity_index = 0;
                entity_index < _entity_mngr->count;
              ++entity_index) {

            const entity_archetype curr_atype = _entity_mngr->data.dense.archetype [entity_index];
            const entity_id        curr_id    = _entity_mngr->data.dense.id        [entity_index];

            if (atype == curr_atype) {
                id_list->data.id[id_index++] = curr_id;
            }
        }

        const bool did_find = (id_list->count > 0); 
        return(did_find);        
    }

    IFB_INTERNAL entity_id
    entity_create(
        const cchar*           tag_cstr,
        const entity_archetype atype) {

        entity_mngr_validate();
        assert(tag_cstr != NULL);

        // return invalid if we are at capactiy
        if (_entity_mngr->count == _entity_mngr->capacity.dense) {
            return(ENTITY_ID_INVALID);
        }

        // get the index

        // create the id and tag
        const entity_tag tag = entity_tag(tag_cstr);
        const entity_id  id  = tag.hash(); 
    
        // make sure this isn't a duplicate
        for (
            u32 entity = 0;
                entity < _entity_mngr->count;
              ++entity
        ) {
            assert(id != _entity_mngr->data.dense.id[entity]);
        }

        // add the id and tag 
        const u32 index = _entity_mngr->count;
        _entity_mngr->data.dense.id        [index] = id;
        _entity_mngr->data.dense.tag       [index] = tag;
        _entity_mngr->data.dense.archetype [index] = atype;

        // update the count and return the id
        ++_entity_mngr->count;
        return(id);
    }

    IFB_INTERNAL bool
    entity_destroy_by_tag(
        const cchar* tag_cstr) {

        entity_mngr_validate();
        assert(tag_cstr != NULL);

        if (_entity_mngr->count == 0) {
            return(false);
        }

        entity_tag tag(tag_cstr);
        entity_id  id = tag.hash();

        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < _entity_mngr->count;
              ++entity
        ) {

            if (id != _entity_mngr->data.dense.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = _entity_mngr->data.dense.id  [entity];
            entity_tag& curr_tag = _entity_mngr->data.dense.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == _entity_mngr->count - 1) {
                _entity_mngr->data.dense.id[entity] = ENTITY_ID_INVALID;
                --_entity_mngr->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (_entity_mngr->count - 1);
            entity_id&  last_id     = _entity_mngr->data.dense.id  [last_entity];
            entity_tag& last_tag    = _entity_mngr->data.dense.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --_entity_mngr->count; 
            break;
        }

        return(did_remove);
    }

    IFB_INTERNAL bool
    entity_destroy_by_id(
        const entity_id  id) {

        entity_mngr_validate();
        assert(id != ENTITY_ID_INVALID);

        
        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < _entity_mngr->count;
              ++entity
        ) {

            if (id != _entity_mngr->data.dense.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = _entity_mngr->data.dense.id  [entity];
            entity_tag& curr_tag = _entity_mngr->data.dense.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == _entity_mngr->count - 1) {
                _entity_mngr->data.dense.id[entity] = ENTITY_ID_INVALID;
                --_entity_mngr->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (_entity_mngr->count - 1);
            entity_id&  last_id     = _entity_mngr->data.dense.id  [last_entity];
            entity_tag& last_tag    = _entity_mngr->data.dense.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --_entity_mngr->count; 
            break;
        }

        return(did_remove);
    }

    IFB_INTERNAL entity_list*
    entity_list_arena_create(
        arena* a) {

        entity_mngr_validate();
        assert(a != NULL);

        
        auto* list = (entity_list*)arena_push(a, sizeof(entity_list));
        assert(list);

        const u32 size_array = _entity_mngr->capacity.dense * sizeof(entity_id);
        list->data.id           = (entity_id*)arena_push(a, size_array);
        list->data.sparse_index =       (u32*)arena_push(a, size_array);
        list->data.dense_index  =       (u32*)arena_push(a, size_array);
        list->capacity          = _entity_mngr->capacity.dense;
        list->count             = 0;
        assert(list->data.id           != NULL);
        assert(list->data.sparse_index != NULL);
        assert(list->data.dense_index  != NULL);

        return(list);
    }

    IFB_INTERNAL void
    entity_list_validate(
        const entity_list* list) {

        entity_mngr_validate();

        assert(
            list           != NULL                         &&
            list->data.id    != NULL                         &&
            list->capacity == _entity_mngr->capacity.dense &&
            list->count    <= list->capacity
        );
    }

};