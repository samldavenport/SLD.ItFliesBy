#pragma once

#include "entities.hpp"

namespace ifb {
    
    static void
    entity_manager_startup(
        const memory& mem_res) {

        assert(
            mem_res.size != 0    &&
            mem_res.ptr  != NULL
        );

        // commit memory
        _entity_manager->mem.ptr  = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        _entity_manager->mem.size = mem_res.size;
        assert(_entity_manager->mem.ptr);
        
        // calculate capacity
        const u32 entity_size = sizeof(entity_id) + sizeof(entity_tag);
        _entity_manager->capacity = (mem_res.size / entity_size);
        assert(_entity_manager->capacity != 0);

        // cast pointers
        const u32  size_entity_ids = (sizeof(entity_id) * _entity_manager->capacity);
        const addr addr_ids        = _entity_manager->mem.address;
        const addr addr_tags       = addr_ids + size_entity_ids;  
        _entity_manager->data.id  =  (entity_id*)addr_ids;
        _entity_manager->data.tag = (entity_tag*)addr_tags;

        memset((void*)addr_ids, 0xFF, size_entity_ids);

        entity_manager_assert_valid();
    }

    static void
    entity_manager_shutdown(
        void) {

        entity_manager_assert_valid();

        pfm_memory_decommit(_entity_manager->mem.ptr, _entity_manager->mem.size);

        _entity_manager->capacity    = 0;
        _entity_manager->count       = 0;
        _entity_manager->data.id     = NULL;
        _entity_manager->data.tag    = NULL;
        _entity_manager->mem.address = 0;
        _entity_manager->mem.size    = 0;
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

        _entity_manager = (entity_manager*)mem.ptr;
        _entity_manager->data.id     = NULL;
        _entity_manager->data.tag    = NULL;
        _entity_manager->capacity    = 0;
        _entity_manager->count       = 0;
        _entity_manager->mem.address = 0;
        _entity_manager->mem.size    = 0;
        
        return(_entity_manager);
    }

    static void
    entity_manager_assert_valid(
        void) {

        assert(
            _entity_manager &&
            _entity_manager->data.id  != NULL &&
            _entity_manager->data.tag != NULL &&
            _entity_manager->capacity != 0    &&
            _entity_manager->count    <= _entity_manager->capacity
        );
    }

};