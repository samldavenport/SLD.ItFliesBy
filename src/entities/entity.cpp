#pragma once

#include "entities.hpp"

namespace ifb {
   
    IFB_INTERNAL u32
    entity_lookup_index_by_tag(
        const entity_tag* tag) {

        entity_mngr_validate();
        assert(tag != NULL);

        const entity_id id = tag->hash();

        for (
            u32 index = 0;
                index < _entity_mngr->capacity;
              ++index 
        ) {
            if (id == _entity_mngr->data.id[index]) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    IFB_INTERNAL u32
    entity_lookup_index_by_id(
        const entity_id id) {

        entity_mngr_validate();
        assert(id != ENTITY_ID_INVALID);

        for (
            u32 index = 0;
                index < _entity_mngr->capacity;
              ++index 
        ) {
            if (_entity_mngr->data.id[index] == id) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    IFB_INTERNAL bool
    entity_lookup_by_archetype(
        const entity_archetype atype,
        entity_id_list*        id_list) {

        entity_mngr_validate();
        entity_id_list_validate(id_list);

        u32& id_index = id_list->count;
        id_index = 0;

        for (
            u32 entity_index = 0;
                entity_index < _entity_mngr->count;
              ++entity_index) {

            const entity_archetype curr_atype = _entity_mngr->data.archetype [entity_index];
            const entity_id        curr_id    = _entity_mngr->data.id        [entity_index];

            if (atype == curr_atype) {
                id_list->array[id_index++] = curr_id;
            }
        }

        const bool did_find = (id_list->count > 0); 
        return(did_find);        
    }


    IFB_INTERNAL void
    entity_get(
        const u32 index,
        entity*   out) {

        entity_mngr_validate();
        assert(
            index <  _entity_mngr->count &&
            out   != NULL
        );

        *out = entity(
            _entity_mngr->data.id        [index],
            _entity_mngr->data.tag       [index],
            _entity_mngr->data.archetype [index]
        );
    }

    IFB_INTERNAL entity_id
    entity_create(
        const cchar* tag_cstr) {

        entity_mngr_validate();
        assert(tag_cstr != NULL);

        // return invalid if we are at capactiy
        if (_entity_mngr->count == _entity_mngr->capacity) {
            return(ENTITY_ID_INVALID);
        }

        // get the index
        const u32 index = _entity_mngr->count;

        // create the id and tag
        const entity_tag tag = entity_tag(tag_cstr);
        const entity_id  id  = tag.hash(); 
    
        // make sure this isn't a duplicate
        for (
            u32 entity = 0;
                entity < _entity_mngr->count;
              ++entity
        ) {
            assert(id != _entity_mngr->data.id[entity]);
        }

        // add the id and tag 
        _entity_mngr->data.id  [index] = id;
        _entity_mngr->data.tag [index] = tag;

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

            if (id != _entity_mngr->data.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = _entity_mngr->data.id  [entity];
            entity_tag& curr_tag = _entity_mngr->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == _entity_mngr->count - 1) {
                _entity_mngr->data.id[entity] = ENTITY_ID_INVALID;
                --_entity_mngr->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (_entity_mngr->count - 1);
            entity_id&  last_id     = _entity_mngr->data.id  [last_entity];
            entity_tag& last_tag    = _entity_mngr->data.tag [last_entity];

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

            if (id != _entity_mngr->data.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = _entity_mngr->data.id  [entity];
            entity_tag& curr_tag = _entity_mngr->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == _entity_mngr->count - 1) {
                _entity_mngr->data.id[entity] = ENTITY_ID_INVALID;
                --_entity_mngr->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (_entity_mngr->count - 1);
            entity_id&  last_id     = _entity_mngr->data.id  [last_entity];
            entity_tag& last_tag    = _entity_mngr->data.tag [last_entity];

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

    IFB_INTERNAL entity_id*
    entity_query_by_archetype(
        arena*                 a,
        const entity_archetype archetype,
        const u32              count) {

        assert(a != NULL && count != 0);

        const u32 size     = count * sizeof(entity_id);
        auto      id_array = (entity_id*)arena_push(a, size);

        if (id_array) {

            memset((void*)id_array, 0xFF, size);

            for (
                u32 index = 0;
                    index < count;
                  ++index
            ) {
                
            }
        }

        return(id_array);
    }

    IFB_INTERNAL void
    entity_id_list_arena_init(
        entity_id_list* list,
        arena*          a) {

        entity_mngr_validate();

        assert(list != NULL && a != NULL);

        const u32 size = _entity_mngr->capacity * sizeof(entity_id);

        list->array    = (entity_id*)arena_push(a, size);
        list->capacity = _entity_mngr->capacity;
        list->count    = 0;

        assert(list->array != NULL);
    }

    IFB_INTERNAL void
    entity_id_list_validate(
        const entity_id_list* list) {

        entity_mngr_validate();

        assert(
            list           != NULL                   &&
            list->array    != NULL                   &&
            list->capacity == _entity_mngr->capacity &&
            list->count    <= list->capacity
        );
    }

};