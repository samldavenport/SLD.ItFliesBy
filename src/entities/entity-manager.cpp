#pragma once

#include "entities.hpp"

namespace ifb {
    
    static u32
    entity_manager_memory_requirement(
        const u32 capacity) {

        const u32 size_struct    = sizeof(entity_manager);
        const u32 size_array_id  = (capacity * sizeof(entity_id));
        const u32 size_array_tag = (capacity * sizeof(entity_tag));
        const u32 size_total     = (size_struct + size_array_id + size_array_tag);

        return(size_total);
    }

    static entity_manager*
    entity_manager_memory_init(
        const u32 capacity,
        const u32 mem_size,
        void*     mem_ptr) {

        assert(
            capacity != 0 &&
            mem_size != 0 &&
            mem_ptr  != NULL
        );

        const u32 size_struct     = sizeof(entity_manager);
        const u32 size_array_id   = (capacity * sizeof(entity_id));
        const u32 size_array_tag  = (capacity * sizeof(entity_tag));
        const u32 size_total      = (size_struct + size_array_id + size_array_tag);

        assert(mem_size == size_total);

        auto em = (entity_manager*)mem_ptr;
        em->data.id  =  (entity_id*)((addr)em          + size_struct);
        em->data.tag = (entity_tag*)((addr)em->data.id + size_array_id);
        em->capacity = capacity;
        em->count    = 0;
        
        memset((void*)em->data.id, 0xFF, size_array_id);        
        
        return(em);
    }

    static void
    entity_manager_assert_valid(
        const entity_manager* em) {

        assert(
            em &&
            em->data.id  != NULL &&
            em->data.tag != NULL &&
            em->capacity != 0    &&
            em->count    <= em->capacity
        );
    }

    static u32
    entity_manager_lookup_index_by_tag(
        const entity_manager* em,
        const entity_tag*   tag) {

        entity_manager_assert_valid(em);
        assert(tag != NULL);

        const u32 hash = entity_tag_hash(tag);

        for (
            u32 index = 0;
                index < em->capacity;
              ++index 
        ) {
            if (em->data.id[index] == hash) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    static u32
    entity_manager_lookup_index_by_id(
        const entity_manager* em,
        const entity_id     id) {

        entity_manager_assert_valid(em);
        assert(id != ENTITY_ID_INVALID);

        for (
            u32 index = 0;
                index < em->capacity;
              ++index 
        ) {
            if (em->data.id[index] == id) {
                return(index);
            }
        }

        return(ENTITY_ID_INVALID);
    }

    static void
    entity_manager_gem(
        const entity_manager* em,
        const u32           index,
        entity*             out) {

        entity_manager_assert_valid(em);
        assert(
            index <  em->count &&
            out   != NULL
        );

        out->id  =  em->data.id  [index];
        out->tag = &em->data.tag [index];
    }

    static entity_id
    entity_manager_insert(
        entity_manager* em,
        const cchar8* tag_cstr) {

        entity_manager_assert_valid(em);
        assert(tag_cstr != NULL);

        if (em->count == em->capacity) {
            entity_id id;
            id.hash = ENTITY_ID_INVALID;
            return(id);
        }

        // gem the index
        const u32 index = em->count;

        // initialize the tag
        entity_tag* tag = &em->data.tag[index];
        entity_tag_init(tag, tag_cstr);

        // create the id
        entity_id id = entity_id_init(tag);
    
        // make sure this isn't a duplicate
        for (
            u32 entity = 0;
                entity < em->count;
              ++entity
        ) {
            assert(id != em->data.id[entity]);
        }

        // add the id and update the count
        em->data.id[index] = id;
        ++em->count;
        return(id);
    }

    static bool
    entity_manager_remove_by_tag(
        entity_manager*     em,
        const entity_tag* tag) {

        entity_manager_assert_valid(em);
        assert(tag != NULL);

        if (em->count == 0) {
            return(false);
        }

        const u32 hash = entity_tag_hash(tag);
        
        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < em->count;
              ++entity
        ) {

            if (hash != em->data.id[entity].hash) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = em->data.id  [entity];
            entity_tag& curr_tag = em->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == em->count - 1) {
                em->data.id[entity] = ENTITY_ID_INVALID;
                --em->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (em->count - 1);
            entity_id&  last_id     = em->data.id  [last_entity];
            entity_tag& last_tag    = em->data.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --em->count; 
            break;
        }

        return(did_remove);
    }

    static bool
    entity_manager_remove_by_id(
        entity_manager*     em,
        const entity_id&  id) {

        entity_manager_assert_valid(em);
        assert(id != ENTITY_ID_INVALID);

        
        bool did_remove = false;

        for (
            u32 entity = 0;
                entity < em->count;
              ++entity
        ) {

            if (id != em->data.id[entity]) {
                continue;
            }

            did_remove = true;

            // gem the current entity data
            entity_id&  curr_id  = em->data.id  [entity];
            entity_tag& curr_tag = em->data.tag [entity];

            // if this is the last entity,
            // clear the id and update the count
            if (entity == em->count - 1) {
                em->data.id[entity] = ENTITY_ID_INVALID;
                --em->count;
                break;
            }

            // gem the last entity data
            const u32   last_entity = (em->count - 1);
            entity_id&  last_id     = em->data.id  [last_entity];
            entity_tag& last_tag    = em->data.tag [last_entity];

            // swap the current and last entity data
            // and update the count
            curr_id  = last_id;
            curr_tag = last_tag;
            last_id  = ENTITY_ID_INVALID;
            --em->count; 
            break;
        }

        return(did_remove);
    }
};