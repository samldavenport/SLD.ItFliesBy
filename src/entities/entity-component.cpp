#pragma once

#include "entity.hpp"

namespace ifb {

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
