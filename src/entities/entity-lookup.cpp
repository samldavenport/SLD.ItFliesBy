#pragma once

#include "entity.hpp"
#include "collections-internal.hpp"
#include "entity-manager.cpp"
#include "ifb-types.hpp"

namespace ifb {

    IFB_INTERNAL bool
    entity_lookup_list(
        entity_id_list&     list,
        const entity_query& query) {

        entity_mngr_validate();
        list.validate();

        for (
            u32 dense_index = 0;
                dense_index < _entity_mngr->count;
              ++dense_index ) {

            const entity_archetype curr_atype = _entity_mngr->data.dense.archetype [dense_index];
            const bool             has_all    = ((curr_atype & query.has_all) == query.has_all);
            const bool             has_any    = ((curr_atype & query.has_any) != 0 || (query.has_any == 0));
            const bool             has_none   = ((curr_atype & query.has_none) == 0);
            const bool             is_match   = (
                has_all &&
                has_any &&
                has_none                
            );

            if (is_match) {
                list.add(dense_index);
            }
        }

        return(entity_list.count() > 0);
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

    IFB_INTERNAL bool
    entity_lookup_by_tag(
        entity&      e,
        const cchar* tag_cstr) {
        
        assert(tag_cstr);

        const entity_tag tag          = tag_cstr;
        const entity_id  id           = tag.to_id();
        u32              sparse_index = (_entity_mngr->capacity.sparse - 1) & id; 
        bool             did_find     = false;

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
    
    IFB_INTERNAL entity_archetype
    entity_lookup_archetype(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        entity_archetype atype = 0;

        for (
            u32 i = 0;
            i < _entity_mngr->count;
            ++i
        ) {
            if (id == _entity_mngr->data.dense.id[i]) {
                atype = _entity_mngr->data.dense.archetype[i]; 
                break;
            }
        }

        return(atype);
    }

    IFB_INTERNAL const cchar*
    entity_lookup_tag(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        cchar* tag = NULL;

        for (
            u32 i = 0;
            i < _entity_mngr->count;
            ++i
        ) {
            if (id == _entity_mngr->data.dense.id[i]) {
                tag = _entity_mngr->data.dense.tag[i].cstr;
                break;
            }
        }

        return(tag);
    }
};
