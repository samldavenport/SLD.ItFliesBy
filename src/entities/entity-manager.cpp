#pragma once

#include "entity.hpp"
#include "eng-internal.hpp"
#include "memory-reservation.cpp"

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
        _entity_mngr->res                     = NULL;
        
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
        reservation* res) {

        assert(res);
        
        const config& cfg = config_instance();
        _entity_mngr->capacity.dense  = cfg.entity_capacity;
        _entity_mngr->capacity.sparse = cfg.entity_capacity / cfg.sparse_set_max_load_p100;
        assert(_entity_mngr->capacity.dense  != 0);
        assert(_entity_mngr->capacity.sparse != 0);
        
        // cast pointers
        const u32  size_entity_ids   = (sizeof(entity_id)        * _entity_mngr->capacity.dense);
        const u32  size_entity_tags  = (sizeof(entity_tag)       * _entity_mngr->capacity.dense);
        const u32  size_entity_arch  = (sizeof(entity_archetype) * _entity_mngr->capacity.dense); 
        const u32  size_index_sparse = (sizeof(u32)              * _entity_mngr->capacity.dense); 
        const u32  size_index_dense  = (sizeof(u32)              * _entity_mngr->capacity.sparse); 

        // set the properties
        _entity_mngr->data.dense.id           =        (entity_id*)reservation_push_bytes(res, size_entity_ids);
        _entity_mngr->data.dense.tag          =       (entity_tag*)reservation_push_bytes(res, size_entity_tags);
        _entity_mngr->data.dense.archetype    = (entity_archetype*)reservation_push_bytes(res, size_entity_arch);
        _entity_mngr->data.dense.sparse_index =              (u32*)reservation_push_bytes(res, size_index_sparse);
        _entity_mngr->data.sparse.dense_index =              (u32*)reservation_push_bytes(res, size_index_dense);

        assert(_entity_mngr->data.dense.id);
        assert(_entity_mngr->data.dense.tag);
        assert(_entity_mngr->data.dense.archetype);
        assert(_entity_mngr->data.dense.sparse_index);
        assert(_entity_mngr->data.sparse.dense_index);

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
    
        reservation_decommit(_entity_mngr->res);

        _entity_mngr->capacity.dense          = 0;
        _entity_mngr->count                   = 0;
        _entity_mngr->data.dense.id           = NULL;
        _entity_mngr->data.dense.tag          = NULL;
        _entity_mngr->data.dense.archetype    = NULL;
        _entity_mngr->data.dense.sparse_index = NULL;
        _entity_mngr->data.sparse.dense_index = NULL;
        _entity_mngr->res                     = NULL;
    }

    IFB_INTERNAL u32
    entity_mngr_capacity_dense(
        void) {

        entity_mngr_validate();
        
        return(_entity_mngr->capacity.dense);
    }

    IFB_INTERNAL u32
    entity_mngr_capacity_sparse(
        void) {

        entity_mngr_validate();
        
        return(_entity_mngr->capacity.sparse);
    }

    IFB_INTERNAL u32
    entity_mngr_get_count(
        void) {

        entity_mngr_validate();
        return(_entity_mngr->count);
    }
};
