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

        // get config value(s)
        const ifb_config& cfg = config_instance();
        _entity_mngr->capacity.dense  = cfg.entity_capacity;
        _entity_mngr->capacity.sparse = cfg.entity_capacity / cfg.sparse_set_max_load_p100;
        assert(_entity_mngr->capacity.dense  != 0);
        assert(_entity_mngr->capacity.sparse != 0);
        
        // cast pointers
        const u32  size_entity_ids            = (sizeof(entity_id)        * _entity_mngr->capacity.dense);
        const u32  size_entity_tags           = (sizeof(entity_tag)       * _entity_mngr->capacity.dense);
        const u32  size_entity_arch           = (sizeof(entity_archetype) * _entity_mngr->capacity.dense); 
        const u32  size_entity_sparse_index   = (sizeof(u32)              * _entity_mngr->capacity.dense); 
        const u32  size_entity_dense_index    = (sizeof(u32)              * _entity_mngr->capacity.sparse);    
        const addr addr_ids                   = _entity_mngr->mem.address;
        const addr addr_tags                  = (addr_ids    + size_entity_ids);
        const addr addr_arch                  = (addr_tags   + size_entity_tags);
        const addr addr_sparse                = (addr_arch   + size_entity_arch);
        const addr addr_dense                 = (addr_sparse + size_entity_sparse_index); 

        _entity_mngr->data.dense.id           =        (entity_id*)addr_ids;
        _entity_mngr->data.dense.tag          =       (entity_tag*)addr_tags;
        _entity_mngr->data.dense.archetype    = (entity_archetype*)addr_arch;
        _entity_mngr->data.dense.sparse_index =              (u32*)addr_sparse;
        _entity_mngr->data.sparse.dense_index =              (u32*)addr_dense; 

        // initialize values
        memset((void*)addr_ids,    0xFF, size_entity_ids);
        memset((void*)addr_sparse, 0xFF, size_entity_sparse_index);
        memset((void*)addr_arch,   0x00, size_entity_arch);

        // validate
        entity_mngr_validate();
    }

    static void
    entity_mngr_shutdown(
        void) {

        entity_mngr_validate();

        pfm_memory_decommit(_entity_mngr->mem.ptr, _entity_mngr->mem.size);

        _entity_mngr->capacity.dense          = 0;
        _entity_mngr->count                   = 0;
        _entity_mngr->data.dense.id           = NULL;
        _entity_mngr->data.dense.tag          = NULL;
        _entity_mngr->data.dense.archetype    = NULL;
        _entity_mngr->data.dense.sparse_index = NULL;
        _entity_mngr->data.sparse.dense_index = NULL;
        _entity_mngr->mem.address             = 0;
        _entity_mngr->mem.size                = 0;
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
        _entity_mngr->data.dense.id           = NULL;
        _entity_mngr->data.dense.tag          = NULL;
        _entity_mngr->data.dense.archetype    = NULL;
        _entity_mngr->data.dense.sparse_index = NULL;
        _entity_mngr->data.sparse.dense_index = NULL;
        _entity_mngr->capacity.dense          = 0;
        _entity_mngr->capacity.sparse         = 0;
        _entity_mngr->count                   = 0;
        _entity_mngr->mem.address             = 0;
        _entity_mngr->mem.size                = 0;
        
        return(_entity_mngr);
    }

    static void
    entity_mngr_validate(
        void) {

        assert(
            _entity_mngr &&
            _entity_mngr->data.dense.id           != NULL &&
            _entity_mngr->data.dense.tag          != NULL &&
            _entity_mngr->data.dense.archetype    != NULL &&
            _entity_mngr->data.dense.sparse_index != NULL &&
            _entity_mngr->data.sparse.dense_index != NULL &&
            _entity_mngr->capacity.dense          != 0    &&
            _entity_mngr->capacity.sparse         != 0    &&
            _entity_mngr->count                   <= _entity_mngr->capacity.dense
        );
    }
};