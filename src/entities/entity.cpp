#pragma once

#include "entities.hpp"

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
        const entity_tag tag = entity_tag(tag_cstr);
        const entity_id  id  = tag.hash(); 
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
        if (!did_find_last) {
            return(false);
        }

        // by setting the current dense info to the last dense info
        // and reducing the count by 1,
        // we have effectively removed the current enitity
        _entity_mngr->data.dense.tag          [e_current.index_dense] = _entity_mngr->data.dense.tag [e_last.index_dense];
        _entity_mngr->data.dense.id           [e_current.index_dense] = e_last.id;
        _entity_mngr->data.dense.archetype    [e_current.index_dense] = e_last.archetype;
        _entity_mngr->data.dense.sparse_index [e_current.index_dense] = e_last.index_sparse; 

        // lastly, set the dense index of the last entity to invalid
        // reduce the count
        // and return
        _entity_mngr->data.sparse.dense_index [e_current.index_sparse] = INVALID_INDEX;
        --_entity_mngr->count;
        return(true);
    }

    IFB_INTERNAL bool
    entity_lookup_by_archetype(
        entity_list*           list,
        const entity_archetype atype ) {

        entity_mngr_validate();
        entity_list_validate(list);

        u32& list_index = list->count;
        list_index = 0;

        for (
            u32 dense_index = 0;
                dense_index < _entity_mngr->count;
              ++dense_index) {
            
            const entity_id        curr_id           = _entity_mngr->data.dense.id           [dense_index];
            const entity_archetype curr_atype        = _entity_mngr->data.dense.archetype    [dense_index];
            const u32              curr_sparse_index = _entity_mngr->data.dense.sparse_index [dense_index];

            if (atype == curr_atype) {
                list->data.id           [list_index] = curr_id;
                list->data.sparse_index [list_index] = curr_sparse_index;
                list->data.dense_index  [list_index] = dense_index;                 
                ++list_index;
            }
        }

        const bool did_find = (list->count > 0); 
        return(did_find);        
    }

    IFB_INTERNAL bool
    entity_lookup_by_index_dense(
        entity&   e,
        const u32 index) {

        entity_mngr_validate();
        
        if (index >= _entity_mngr->count) {
            return(false);
        }

        e.tag          = _entity_mngr->data.dense.tag          [index].cstr();
        e.id           = _entity_mngr->data.dense.id           [index];
        e.archetype    = _entity_mngr->data.dense.archetype    [index];
        e.index_sparse = _entity_mngr->data.dense.sparse_index [index];
        e.index_dense  = _entity_mngr->data.sparse.dense_index [e.index_sparse];  

        return(true);
    }

    IFB_INTERNAL entity_list*
    entity_list_arena_create(
        arena* a) {

        entity_mngr_validate();
        assert(a != NULL);
        
        auto* list = (entity_list*)arena_push(a, sizeof(entity_list));
        assert(list);

        const u32 size_array = _entity_mngr->capacity.dense * sizeof(entity_id);
        list->data.id           = (entity_id*)arena_push(a, size_array);
        list->data.sparse_index =       (u32*)arena_push(a, size_array);
        list->data.dense_index  =       (u32*)arena_push(a, size_array);
        list->capacity          = _entity_mngr->capacity.dense;
        list->count             = 0;
        assert(list->data.id           != NULL);
        assert(list->data.sparse_index != NULL);
        assert(list->data.dense_index  != NULL);

        return(list);
    }

    IFB_INTERNAL void
    entity_list_validate(
        const entity_list* list) {

        entity_mngr_validate();

        assert(
            list           != NULL                         &&
            list->data.id  != NULL                         &&
            list->capacity == _entity_mngr->capacity.dense &&
            list->count    <= list->capacity
        );
    }

    IFB_INTERNAL bool
    entity_lookup_by_tag(
        entity&      e,
        const cchar* tag_cstr) {
        
        assert(tag_cstr);

        const auto      tag          = entity_tag(tag_cstr);
        const entity_id id           = tag.hash();
        u32             sparse_index = (_entity_mngr->capacity.sparse - 1) & id; 
        bool            did_find     = false;

        for (
            u32 probe = 0;
                probe < _entity_mngr->capacity.sparse;
              ++probe) {
        
            // make sure the sparse index wraps around the array
            sparse_index += probe;
            sparse_index %= _entity_mngr->capacity.sparse;

            // get the dense index at this sparse location
            const u32 curr_dense_index = _entity_mngr->data.sparse.dense_index[sparse_index]; 
            
            // if there is no value, this entity does not exist
            if (curr_dense_index == INVALID_INDEX) {
                break;
            }

            // sanity check, the sparse indexes should match
            assert(sparse_index == _entity_mngr->data.dense.sparse_index[curr_dense_index]);

            // get the dense data at this location
            const entity_id        curr_id    = _entity_mngr->data.dense.id        [curr_dense_index];
            const entity_archetype curr_atype = _entity_mngr->data.dense.archetype [curr_dense_index];
            const char*            curr_tag   = _entity_mngr->data.dense.tag       [curr_dense_index].cstr(); 

            // if there is a entity different from what we expect,
            // go to the next entity
            if (id != curr_id) {
                continue;
            }

            // otherwise, we found the entity
            did_find = true;

            // set the values and return
            e.tag          = curr_tag;
            e.id           = curr_id;
            e.archetype    = curr_atype;
            e.index_sparse = sparse_index;
            e.index_dense  = curr_dense_index;
            break;
        }

        return(did_find);
    }



};