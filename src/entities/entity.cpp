#pragma once

#include "entity.hpp"

namespace ifb {

    IFB_INTERNAL entity_id
    entity_create(
        const cchar*           tag_cstr,
        const entity_archetype atype) {

        entity_mngr_validate();
        assert(tag_cstr != NULL);

        // return invalid if we are at capactiy
        if (_entity_mngr->count == _entity_mngr->capacity.dense) {
            return(ENTITY_ID_INVALID);
        }

        // create the id and tag
        entity_tag tag;
        entity_tag_init(tag, tag_cstr);

        const entity_id id  = entity_tag_hash(tag); 
        assert(id != ENTITY_ID_INVALID);

        // make sure this isn't a duplicate
        for (
            u32 entity = 0;
                entity < _entity_mngr->count;
              ++entity
        ) {
            assert(id != _entity_mngr->data.dense.id[entity]);
        }

        // get the sparse and dense_index
        const u32 dense_index  = _entity_mngr->count;
        u32       sparse_index = (_entity_mngr->capacity.sparse - 1) & id; 
        bool      did_insert   = false;       
        for (
            u32 probe = 0;
                probe < _entity_mngr->capacity.sparse;
              ++probe) {

            // make sure the sparse index wraps around the array
            sparse_index += probe;
            sparse_index %= _entity_mngr->capacity.sparse;

            // get the dense index at this sparse location
            const u32 tmp_dense_index = _entity_mngr->data.sparse.dense_index[sparse_index]; 

            // there is a value here,
            // make sure its not a collision before continuing
            if (tmp_dense_index != INVALID_INDEX) {
                const bool is_collision = (id == _entity_mngr->data.dense.id[tmp_dense_index]);
                assert(!is_collision);
                continue;
            }

            // otherwise, we have a valid index
            did_insert = true;
            
            // add the values 
            _entity_mngr->data.dense.id           [dense_index]  = id;
            _entity_mngr->data.dense.tag          [dense_index]  = tag;
            _entity_mngr->data.dense.archetype    [dense_index]  = atype;
            _entity_mngr->data.dense.sparse_index [dense_index]  = sparse_index;
            _entity_mngr->data.sparse.dense_index [sparse_index] = dense_index;

            // update the count
            ++_entity_mngr->count;
            break;
        }

        // update the count and return the id
        return(did_insert ? id : ENTITY_ID_INVALID);
    }

    IFB_INTERNAL bool
    entity_destroy(
        const cchar* tag_cstr) {

        entity_mngr_validate();

        if (_entity_mngr->count == 0) {
            return(false);
        }

        // look up the current and last entities
        entity     e_current;
        entity     e_last;
        const u32  last_index       = (_entity_mngr->count - 1);
        const bool did_find_current = entity_lookup_by_tag         (e_current, tag_cstr);
        const bool did_find_last    = entity_lookup_by_index_dense (e_last,    last_index);   

        // we should always retrieve the last one
        assert(did_find_last);

        // we're done if we didn't find the requested one
        if (!did_find_current) {
            return(false);
        }

        // if the last entity and current entity not are the same,
        // we need to set the current info to last info
        // otherwise, skip straight to invalidating and reducing the count
        if (e_current.id != e_last.id) {

            // by setting the current dense info to the last dense info
            // and reducing the count by 1,
            // we have effectively removed the current enitity
            entity_tag_init(_entity_mngr->data.dense.tag[e_current.index_dense], _entity_mngr->data.dense.tag [e_last.index_dense].cstr);
            _entity_mngr->data.dense.id           [e_current.index_dense] = e_last.id;
            _entity_mngr->data.dense.archetype    [e_current.index_dense] = e_last.archetype;
            _entity_mngr->data.dense.sparse_index [e_current.index_dense] = e_last.index_sparse; 
            _entity_mngr->data.sparse.dense_index [e_last.index_sparse]   = e_current.index_dense;
        }

        // lastly, set the dense index of the last entity to invalid
        // reduce the count
        // and return
        _entity_mngr->data.sparse.dense_index [e_current.index_sparse] = INVALID_INDEX;
        --_entity_mngr->count;
        return(true);
    }
};