#pragma once

#include "entities.hpp"

namespace ifb {
    
    static u32
    entity_table_memory_requirement(
        const u32 capacity) {

        const u32 size_struct    = sizeof(entity_table);
        const u32 size_array_id  = (capacity * sizeof(entity_id));
        const u32 size_array_tag = (capacity * sizeof(entity_tag));
        const u32 size_total     = (size_struct + size_array_id + size_array_tag);

        return(size_total);
    }

    static entity_table*
    entity_table_memory_init(
        const u32 capacity,
        const u32 mem_size,
        void*     mem_ptr) {

        assert(
            capacity != 0 &&
            mem_size != 0 &&
            mem_ptr  != NULL
        );

        const u32 size_struct     = sizeof(entity_table);
        const u32 size_array_id   = (capacity * sizeof(entity_id));
        const u32 size_array_tag  = (capacity * sizeof(entity_tag));
        const u32 size_total      = (size_struct + size_array_id + size_array_tag);

        assert(mem_size == size_total);

        auto et = (entity_table*)mem_ptr;
        et->data.id  =  (entity_id*)((addr)et          + size_struct);
        et->data.tag = (entity_tag*)((addr)et->data.id + size_array_id);
        et->capacity = capacity;
        et->count    = 0;
        
        memset((void*)et->data.id, 0xFF, size_array_id);        
        
        return(et);
    }

    static void
    entity_table_assert_valid(
        const entity_table* et) {

        assert(
            et &&
            et->data.id  != NULL &&
            et->data.tag != NULL &&
            et->capacity != 0    &&
            et->count    <= et->capacity
        );
    }

    static u32
    entity_table_lookup_index_by_tag(
        const entity_table* et,
        const entity_tag*   tag) {

        entity_table_assert_valid(et);
        assert(tag != NULL);

        const u32 hash = entity_tag_hash(tag);

        for (
            u32 index = 0;
                index < et->capacity;
              ++index 
        ) {
            if (et->data.id[index] == hash) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    static u32
    entity_table_lookup_index_by_id(
        const entity_table* et,
        const entity_id     id) {

        entity_table_assert_valid(et);
        assert(id != ENTITY_ID_INVALID);

        for (
            u32 index = 0;
                index < et->capacity;
              ++index 
        ) {
            if (et->data.id[index] == id) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    static void
    entity_table_get(
        const entity_table* et,
        const u32           index,
        entity*             out) {

        entity_table_assert_valid(et);
        assert(
            index <  et->count &&
            out   != NULL
        );

        out->id  =  et->data.id  [index];
        out->tag = &et->data.tag [index];
    }

    static entity_id
    entity_table_insert(
        entity_table* et,
        const cchar8* tag_cstr) {

        entity_table_assert_valid(et);
        assert(tag_cstr != NULL);

        if (et->count == et->capacity) {
            entity_id id;
            id.hash = ENTITY_ID_INVALID;
            return(id);
        }

        // get the index
        const u32 index = et->count;

        // initialize the tag
        entity_tag* tag = &et->data.tag[index];
        entity_tag_init(tag, tag_cstr);

        // create the id
        entity_id id = entity_id_init(tag);
    
        // make sure this isn't a duplicate
        for (
            u32 entity = 0;
                entity < et->count;
              ++entity
        ) {
            assert(id != et->data.id[entity]);
        }

        // add the id and update the count
        et->data.id[index] = id;
        ++et->count;
        return(id);
    }

    static bool
    entity_table_remove_by_tag(
        entity_table*     et,
        const entity_tag* tag) {

        entity_table_assert_valid(et);
        assert(tag != NULL);

        if (et->count == 0) {
            return(false);
        }

        const u32 hash = entity_tag_hash(tag);
        
        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < et->count;
              ++entity
        ) {

            if (hash != et->data.id[entity].hash) {
                continue;
            }

            did_remove = true;

            // get the current entity data
            entity_id&  curr_id  = et->data.id  [entity];
            entity_tag& curr_tag = et->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == et->count - 1) {
                et->data.id[entity] = ENTITY_ID_INVALID;
                --et->count;
                break;
            }

            // get the last entity data
            const u32   last_entity = (et->count - 1);
            entity_id&  last_id     = et->data.id  [last_entity];
            entity_tag& last_tag    = et->data.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --et->count; 
            break;
        }

        return(did_remove);
    }

    static bool
    entity_table_remove_by_id(
        entity_table*     et,
        const entity_id&  id) {

        entity_table_assert_valid(et);
        assert(id != ENTITY_ID_INVALID);

        
        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < et->count;
              ++entity
        ) {

            if (id != et->data.id[entity]) {
                continue;
            }

            did_remove = true;

            // get the current entity data
            entity_id&  curr_id  = et->data.id  [entity];
            entity_tag& curr_tag = et->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == et->count - 1) {
                et->data.id[entity] = ENTITY_ID_INVALID;
                --et->count;
                break;
            }

            // get the last entity data
            const u32   last_entity = (et->count - 1);
            entity_id&  last_id     = et->data.id  [last_entity];
            entity_tag& last_tag    = et->data.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --et->count; 
            break;
        }

        return(did_remove);
    }
};