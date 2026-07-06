#pragma once

#include "components.hpp"
#include "entities.hpp"

namespace ifb {

    IFB_INTERNAL component_list_position*
    component_position_table_lookup(
        arena*             arena,
        const entity_list* entt_list,
        const u32          count
    ) {

        entity_list_validate(entt_list);
        assert(
            _cmpnt_mngr->capacity        != 0     &&
            _cmpnt_mngr->capacity        >= count &&
            _cmpnt_mngr->tables.position != NULL
        );

        auto list   = arena_push<component_list_position> (arena);
        auto elmnts = arena_push<component_position>      (arena, count);

        assert(
            list   != NULL &&
            elmnts != NULL            
        );

        const auto tbl = _cmpnt_mngr->tables.position;
        for (
            u32 index;
                index < count;
              ++index) {

            const u32 sparse_index = entt_list->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            list->data.id [index] = entt_list->data.id [index];
            list->data.x  [index] = tbl->x             [index];
            list->data.y  [index] = tbl->y             [index];
            list->data.z  [index] = tbl->z             [index];
        }

        list->capacity = count;
        list->count    = count;

    }

    IFB_INTERNAL void
    component_position_list_get(
        const component_list_position* list,
        const u32                      index,
        component_position&            pos) {

        assert(
            list->capacity != 0    &&
            list->data.id  != NULL &&
            list->data.x   != NULL && 
            list->data.y   != NULL && 
            list->data.z   != NULL && 
            index < list->count
        );

        pos.id = list->data.id [index];
        pos.x  = list->data.x  [index];
        pos.y  = list->data.y  [index];
        pos.z  = list->data.z  [index];
    }

    IFB_INTERNAL void
    component_position_list_update(
        component_list_position*  list,
        const u32                 index,
        const component_position& pos) {

        assert(
            list->capacity != 0    &&
            list->data.id  != NULL &&
            list->data.x   != NULL && 
            list->data.y   != NULL && 
            list->data.z   != NULL && 
            index < list->count
        );

        list->data.id [index] = pos.id;
        list->data.x  [index] = pos.x;
        list->data.y  [index] = pos.y;
        list->data.z  [index] = pos.z;
    }
};