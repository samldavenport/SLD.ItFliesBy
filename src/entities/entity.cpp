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

    IFB_INTERNAL bool
    entity_lookup_by_archetype_exclusive(
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
    entity_lookup_by_archetype_inclusive(
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

            if ((curr_atype & atype) == atype) {
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

        e.tag          = _entity_mngr->data.dense.tag          [index].cstr;
        e.id           = _entity_mngr->data.dense.id           [index];
        e.archetype    = _entity_mngr->data.dense.archetype    [index];
        e.index_sparse = _entity_mngr->data.dense.sparse_index [index];
        e.index_dense  = _entity_mngr->data.sparse.dense_index [e.index_sparse];  

        return(true);
    }

    IFB_INTERNAL entity_list*
    entity_list_create(
        arena* a) {

        entity_mngr_validate();
        assert(a != NULL);
        
        const u32 save     = arena_save(a);
        const u32 capacity = _entity_mngr->capacity.dense; 

        auto* list              = arena_push<entity_list> (a);
        auto  data_id           = arena_push<entity_id>   (a, capacity);
        auto  data_sparse_index = arena_push<u32>         (a, capacity);
        auto  data_dense_index  = arena_push<u32>         (a, capacity);

        const bool did_alloc = (
            list              != NULL &&
            data_id           != NULL &&
            data_sparse_index != NULL &&
            data_dense_index  != NULL
        );

        if (!did_alloc) {
            arena_revert(a,save);
            return(NULL);
        }

        arena_commit(a, save);

        list->data.id           = data_id;
        list->data.sparse_index = data_sparse_index;
        list->data.dense_index  = data_dense_index;
        list->count             = 0;
        return(list);
    }

    IFB_INTERNAL void
    entity_list_validate(
        const entity_list* list) {

        entity_mngr_validate();

        assert(
            list                    != NULL &&
            list->data.id           != NULL &&
            list->data.sparse_index != NULL &&
            list->count             <= _entity_mngr->capacity.dense
        );
    }

    IFB_INTERNAL bool
    entity_lookup_by_tag(
        entity&      e,
        const cchar* tag_cstr) {
        
        assert(tag_cstr);

        entity_tag tag;
        entity_tag_init(tag, tag_cstr);
        const entity_id id           = entity_tag_hash(tag);
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
            
            // if there is no value, go to the next one 
            if (curr_dense_index == INVALID_INDEX) {
                continue;
            }

            // sanity check, the sparse indexes should match
            assert(sparse_index == _entity_mngr->data.dense.sparse_index[curr_dense_index]);

            // get the dense data at this location
            const entity_id        curr_id    = _entity_mngr->data.dense.id        [curr_dense_index];
            const entity_archetype curr_atype = _entity_mngr->data.dense.archetype [curr_dense_index];
            const char*            curr_tag   = _entity_mngr->data.dense.tag       [curr_dense_index].cstr; 

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

    IFB_INTERNAL bool
    entity_component_add(
        const entity_id      id,
        const component_type types) {

        bool did_update = false;
        for (
            u32 index = 0;
                index < _entity_mngr->count;
              ++index) {

            did_update = (id == _entity_mngr->data.dense.id[index]);
            if (did_update) {

                _entity_mngr->data.dense.archetype[index] |= types;
                break;
            }
        }
        return(did_update);
    }

    IFB_INTERNAL bool
    entity_component_add(
        const cchar*         tag_cstr,
        const component_type types) {
        
        assert(tag_cstr != NULL);

        const entity_id id         = hash_u32((void*)tag_cstr, ENTITY_TAG_SIZE);
        const bool      did_update = entity_component_add(id, types);
        return(did_update);
    }

    IFB_INTERNAL bool
    entity_component_remove(
        const entity_id      id,
        const component_type types) {

        bool did_update = false;
        for (
            u32 index = 0;
                index < _entity_mngr->count;
              ++index) {

            did_update = (id == _entity_mngr->data.dense.id[index]);
            if (did_update) {

                _entity_mngr->data.dense.archetype[index] &= ~types.val;
                break;
            }
        }
        return(did_update);
    }
    
    IFB_INTERNAL bool
    entity_component_remove(
        const cchar*         tag_cstr,
        const component_type types) {
        
        assert(tag_cstr != NULL);

        const entity_id id         = hash_u32((void*)tag_cstr, ENTITY_TAG_SIZE);
        const bool      did_update = entity_component_remove(id, types);
        return(did_update);
    }

};