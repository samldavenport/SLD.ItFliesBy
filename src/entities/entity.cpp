#pragma once

#include "entities.hpp"

namespace ifb {
   
    IFB_INTERNAL u32
    entity_lookup_index_by_tag(
        const entity_tag* tag) {

        entity_manager_assert_valid();
        assert(tag != NULL);

        const entity_id id = tag->hash();

        for (
            u32 index = 0;
                index < _entity_manager->capacity;
              ++index 
        ) {
            if (id == _entity_manager->data.id[index]) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    IFB_INTERNAL u32
    entity_lookup_index_by_id(
        const entity_id id) {

        entity_manager_assert_valid();
        assert(id != ENTITY_ID_INVALID);

        for (
            u32 index = 0;
                index < _entity_manager->capacity;
              ++index 
        ) {
            if (_entity_manager->data.id[index] == id) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    IFB_INTERNAL void
    entity_get(
        const u32 index,
        entity*   out) {

        entity_manager_assert_valid();
        assert(
            index <  _entity_manager->count &&
            out   != NULL
        );

        *out = entity(
            _entity_manager->data.id        [index],
            _entity_manager->data.tag       [index],
            _entity_manager->data.archetype [index]
        );
    }

    IFB_INTERNAL entity_id
    entity_create(
        const cchar* tag_cstr) {

        entity_manager_assert_valid();
        assert(tag_cstr != NULL);

        if (_entity_manager->count == _entity_manager->capacity) {
            entity_id id;
            id = ENTITY_ID_INVALID;
            return(id);
        }

        // gem the index
        const u32 index = _entity_manager->count;

        // initialize the tag
        entity_tag& tag = _entity_manager->data.tag[index];

        // create the id
        entity_id id = tag.hash(); 
    
        // make sure this isn't a duplicate
        for (
            u32 entity = 0;
                entity < _entity_manager->count;
              ++entity
        ) {
            assert(id != _entity_manager->data.id[entity]);
        }

        // add the id and update the count
        _entity_manager->data.id[index] = id;
        ++_entity_manager->count;
        return(id);
    }

    IFB_INTERNAL bool
    entity_destroy_by_tag(
        const cchar* tag_cstr) {

        entity_manager_assert_valid();
        assert(tag_cstr != NULL);

        if (_entity_manager->count == 0) {
            return(false);
        }

        entity_tag tag(tag_cstr);
        entity_id  id = tag.hash();

        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < _entity_manager->count;
              ++entity
        ) {

            if (id != _entity_manager->data.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = _entity_manager->data.id  [entity];
            entity_tag& curr_tag = _entity_manager->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == _entity_manager->count - 1) {
                _entity_manager->data.id[entity] = ENTITY_ID_INVALID;
                --_entity_manager->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (_entity_manager->count - 1);
            entity_id&  last_id     = _entity_manager->data.id  [last_entity];
            entity_tag& last_tag    = _entity_manager->data.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --_entity_manager->count; 
            break;
        }

        return(did_remove);
    }

    IFB_INTERNAL bool
    entity_destroy_by_id(
        const entity_id  id) {

        entity_manager_assert_valid();
        assert(id != ENTITY_ID_INVALID);

        
        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < _entity_manager->count;
              ++entity
        ) {

            if (id != _entity_manager->data.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = _entity_manager->data.id  [entity];
            entity_tag& curr_tag = _entity_manager->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == _entity_manager->count - 1) {
                _entity_manager->data.id[entity] = ENTITY_ID_INVALID;
                --_entity_manager->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (_entity_manager->count - 1);
            entity_id&  last_id     = _entity_manager->data.id  [last_entity];
            entity_tag& last_tag    = _entity_manager->data.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --_entity_manager->count; 
            break;
        }

        return(did_remove);
    } 
};