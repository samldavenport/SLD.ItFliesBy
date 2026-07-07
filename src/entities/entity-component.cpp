#pragma once

#include "entity.hpp"
#include "component.hpp"

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

    IFB_INTERNAL bool
    entity_component_update_color(
        arena*                a,
        const cchar**         tag_cstr,
        const color_rgba_u32* clr,
        const u32             count) {

        assert(
            a        != NULL &&
            tag_cstr != NULL &&
            clr      != NULL &&
            count    != 0
        );

        const u32 save = arena_save(a);

        component_list_color* list = component_color_list_create(a);
        if (list == NULL) {
            arena_revert(a, save);
            return(false);
        }

        entity          curr_entity = {0};
        component_color curr_cmpnt  = {0};
        bool            did_update  = true;
        for (
            u32 index = 0;
                index < count;
              ++index
        ) {

            // get current tag and color
            const cchar*          curr_tag   = tag_cstr [index];
            const color_rgba_u32& curr_color = clr      [index];
            assert(curr_tag != NULL);

            // look up the entity
            const bool did_find = entity_lookup_by_tag(curr_entity, curr_tag);
            if (!did_find) {
                continue;
            }

            // add the component to the list
            curr_cmpnt.id           = curr_entity.id;
            curr_cmpnt.sparse_index = curr_entity.index_sparse;
            curr_cmpnt.hex_rgba     = curr_color.hex;
            did_update &= component_color_list_add(list, curr_cmpnt);
        }

        component_color_table_update(list);
        arena_revert(a, save);
        return(did_update);
    }

    IFB_INTERNAL bool
    entity_component_update_position(
        arena              a,
        const cchar**      tag_cstr,
        const position_3d* pos,
        const u32          count) {

    }

    IFB_INTERNAL bool
    entity_component_update_quad(
        arena                a,
        const cchar**        tag_cstr,
        const dimensions_2d* dim,
        const u32            count) {

    }


};