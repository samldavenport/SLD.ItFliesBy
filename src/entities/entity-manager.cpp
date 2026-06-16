#pragma once

#include "entities.hpp"

namespace ifb {
    
    static void
    entity_manager_startup(
        entity_manager* em,
        const memory&   mem_res) {

        assert(
            em           != NULL &&
            mem_res.size != 0    &&
            mem_res.ptr  != NULL
        );

        // commit memory
        em->mem.ptr  = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        em->mem.size = mem_res.size;
        assert(em->mem.ptr);
        
        // calculate capacity
        const u32 entity_size = sizeof(entity_id) + sizeof(entity_tag);
        em->capacity = (mem_res.size / entity_size);
        assert(em->capacity != 0);

        // cast pointers
        const u32  size_entity_ids = (sizeof(entity_id) * em->capacity);
        const addr addr_ids        = em->mem.address;
        const addr addr_tags       = addr_ids + size_entity_ids;  
        em->data.id  =  (entity_id*)addr_ids;
        em->data.tag = (entity_tag*)addr_tags;

        memset((void*)addr_ids, 0xFF, size_entity_ids);

        entity_manager_assert_valid(em);
    }

    static void
    entity_manager_shutdown(
        entity_manager* em) {

        entity_manager_assert_valid(em);

        pfm_memory_decommit(em->mem.ptr, em->mem.size);

        em->capacity    = 0;
        em->count       = 0;
        em->data.id     = NULL;
        em->data.tag    = NULL;
        em->mem.address = 0;
        em->mem.size    = 0;
    }


    static u32
    entity_manager_memory_requirement(
        void) {

        const u32 size_struct = sizeof(entity_manager);
        return(size_struct);
    }

    static entity_manager*
    entity_manager_memory_init(
        const memory& mem) {

        assert(
            mem.size != 0 &&
            mem.ptr  != NULL
        );

        const u32 size_struct = sizeof(entity_manager);
        assert(mem.size == size_struct);

        auto em = (entity_manager*)mem.ptr;
        em->data.id     = NULL;
        em->data.tag    = NULL;
        em->capacity    = 0;
        em->count       = 0;
        em->mem.address = 0;
        em->mem.size    = 0;
        
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

};