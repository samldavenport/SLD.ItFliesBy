#pragma once

#include "entities.hpp"

namespace ifb {
    
    static void
    entity_mngr_startup(
        const memory& mem_res) {

        assert(
            mem_res.size != 0    &&
            mem_res.ptr  != NULL
        );

        // commit memory
        _entity_mngr->mem.ptr  = pfm_memory_commit(mem_res.ptr, 0, mem_res.size);
        _entity_mngr->mem.size = mem_res.size;
        assert(_entity_mngr->mem.ptr);
        
        // calculate capacity
        const u32 entity_size = sizeof(entity_id) + sizeof(entity_tag) + sizeof(entity_archetype);
        _entity_mngr->capacity = (mem_res.size / entity_size);
        assert(_entity_mngr->capacity != 0);

        // cast pointers
        const u32  size_entity_ids      = (sizeof(entity_id)        * _entity_mngr->capacity);
        const u32  size_entity_tags     = (sizeof(entity_tag)       * _entity_mngr->capacity);
        const u32  size_entity_arch     = (sizeof(entity_archetype) * _entity_mngr->capacity); 
        const addr addr_ids             = _entity_mngr->mem.address;
        const addr addr_tags            = addr_ids  + size_entity_ids;
        const addr addr_arch            = addr_tags + size_entity_tags; 
        _entity_mngr->data.id        =        (entity_id*)addr_ids;
        _entity_mngr->data.tag       =       (entity_tag*)addr_tags;
        _entity_mngr->data.archetype = (entity_archetype*)addr_arch;

        memset((void*)addr_ids,  0xFF, size_entity_ids);
        memset((void*)addr_arch, 0x00, size_entity_arch);

        entity_mngr_validate();
    }

    static void
    entity_mngr_shutdown(
        void) {

        entity_mngr_validate();

        pfm_memory_decommit(_entity_mngr->mem.ptr, _entity_mngr->mem.size);

        _entity_mngr->capacity    = 0;
        _entity_mngr->count       = 0;
        _entity_mngr->data.id     = NULL;
        _entity_mngr->data.tag    = NULL;
        _entity_mngr->mem.address = 0;
        _entity_mngr->mem.size    = 0;
    }


    static u32
    entity_mngr_memory_requirement(
        void) {

        const u32 size_struct = sizeof(entity_manager);
        return(size_struct);
    }

    static entity_manager*
    entity_mngr_memory_init(
        const memory& mem) {

        assert(
            mem.size != 0 &&
            mem.ptr  != NULL
        );

        const u32 size_struct = sizeof(entity_manager);
        assert(mem.size == size_struct);

        _entity_mngr = (entity_manager*)mem.ptr;
        _entity_mngr->data.id     = NULL;
        _entity_mngr->data.tag    = NULL;
        _entity_mngr->capacity    = 0;
        _entity_mngr->count       = 0;
        _entity_mngr->mem.address = 0;
        _entity_mngr->mem.size    = 0;
        
        return(_entity_mngr);
    }

    static void
    entity_mngr_validate(
        void) {

        assert(
            _entity_mngr &&
            _entity_mngr->data.id        != NULL &&
            _entity_mngr->data.tag       != NULL &&
            _entity_mngr->data.archetype != NULL &&
            _entity_mngr->capacity       != 0    &&
            _entity_mngr->count          <= _entity_mngr->capacity
        );
    }
};