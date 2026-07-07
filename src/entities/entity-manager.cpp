#pragma once

#include "entity.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_INTERNAL entity_mngr*
    entity_mngr_create(
        void) {

        _entity_mngr = global_alloc<entity_mngr>();
        assert(_entity_mngr);

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

    IFB_INTERNAL void
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

    IFB_INTERNAL void
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
        const config& cfg = config_instance();
        _entity_mngr->capacity.dense  = cfg.entity_capacity;
        _entity_mngr->capacity.sparse = cfg.entity_capacity / cfg.sparse_set_max_load_p100;
        assert(_entity_mngr->capacity.dense  != 0);
        assert(_entity_mngr->capacity.sparse != 0);
        
        // cast pointers
        const u32  size_entity_ids  = (sizeof(entity_id)        * _entity_mngr->capacity.dense);
        const u32  size_entity_tags = (sizeof(entity_tag)       * _entity_mngr->capacity.dense);
        const u32  size_entity_arch = (sizeof(entity_archetype) * _entity_mngr->capacity.dense); 
        const u32  size_index       = (sizeof(u32)              * _entity_mngr->capacity.dense); 
        const addr addr_ids         = _entity_mngr->mem.address;
        const addr addr_tags        = (addr_ids    + size_entity_ids);
        const addr addr_arch        = (addr_tags   + size_entity_tags);
        const addr addr_sparse      = (addr_arch   + size_entity_arch);
        const addr addr_dense       = (addr_sparse + size_index); 

        // set the properties
        _entity_mngr->data.dense.id           =        (entity_id*)addr_ids;
        _entity_mngr->data.dense.tag          =       (entity_tag*)addr_tags;
        _entity_mngr->data.dense.archetype    = (entity_archetype*)addr_arch;
        _entity_mngr->data.dense.sparse_index =              (u32*)addr_sparse;
        _entity_mngr->data.sparse.dense_index =              (u32*)addr_dense;


        // initialize values
        for (
            u32 dense_index = 0;
                dense_index < _entity_mngr->capacity.dense;
              ++dense_index) {

            _entity_mngr->data.dense.id           [dense_index] = ENTITY_ID_INVALID;
            _entity_mngr->data.dense.sparse_index [dense_index] = INVALID_INDEX;
        }

        for (
            u32 sparse_index = 0;
                sparse_index < _entity_mngr->capacity.sparse;
              ++sparse_index) {

            _entity_mngr->data.sparse.dense_index [sparse_index] = INVALID_INDEX;
        }

        // validate
        entity_mngr_validate();
    }

    IFB_INTERNAL void
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
};