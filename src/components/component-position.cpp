#pragma once

#include "components.hpp"
#include "entities.hpp"

namespace ifb {

    IFB_INTERNAL void
    component_position_list_validate(
        const component_list_position* list) {

        assert(
            list                    != NULL &&
            list->data.id           != NULL &&
            list->data.sparse_index != NULL &&
            list->data.x            != NULL &&
            list->data.y            != NULL &&
            list->data.z            != NULL &&
            list->count             <= _entity_mngr->capacity.dense
        );
    }

    IFB_INTERNAL component_list_position*
    component_position_list_create(
        arena* arena) {

        assert(arena != NULL);

        const u32 save     = arena_save(arena);
        const u32 capacity = _entity_mngr->capacity.dense;

        auto list        = arena_push<component_list_position> (arena);
        auto array_id    = arena_push<entity_id>               (arena, capacity);
        auto array_index = arena_push<u32>                     (arena, capacity);
        auto array_x     = arena_push<f32>                     (arena, capacity);
        auto array_y     = arena_push<f32>                     (arena, capacity);
        auto array_z     = arena_push<f32>                     (arena, capacity);

        const bool did_create = (
            list        != NULL &&
            array_id    != NULL &&
            array_index != NULL &&
            array_x     != NULL &&
            array_y     != NULL &&
            array_z     != NULL
        );

        if (!did_create) {
            arena_revert(arena, save);
            return(NULL);
        }

        arena_commit(arena);

        list->data.id           = array_id; 
        list->data.sparse_index = array_index; 
        list->data.x            = array_x; 
        list->data.y            = array_y; 
        list->data.z            = array_z; 
        list->count             = 0;

        component_position_list_validate(list);

        return(list);
    }

    IFB_INTERNAL void
    component_position_table_lookup(
        component_list_position* list_position,
        const entity_list*       list_entity) {

        component_position_list_validate (list_position);
        entity_list_validate             (list_entity);
    
        assert(
            _cmpnt_mngr->capacity        != 0  &&
            _cmpnt_mngr->tables.position != NULL
        );

        const auto tbl = _cmpnt_mngr->tables.position;
        for (
            u32 index;
                index < list_entity->count;
              ++index) {

            const u32 sparse_index = list_entity->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            list_position->data.id           [index] = list_entity->data.id [index];
            list_position->data.sparse_index [index] = sparse_index;
            list_position->data.x            [index] = tbl->x [sparse_index];
            list_position->data.y            [index] = tbl->y [sparse_index];
            list_position->data.z            [index] = tbl->z [sparse_index];
        }

        list_position->count = list_entity->count;
    }

    IFB_INTERNAL void
    component_position_table_update(
        component_list_position* list_position) {

        component_position_list_validate (list_position);

        assert(
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.position != NULL
        );

        auto tbl = _cmpnt_mngr->tables.position;
        for (
            u32 index = 0;
                index < list_position->count;
              ++index) {

            const u32 sparse_index = list_position->data.sparse_index[index];
            assert(sparse_index <= _entity_mngr->capacity.sparse);

            tbl->x[sparse_index] = list_position->data.x[index];
            tbl->y[sparse_index] = list_position->data.y[index];
            tbl->z[sparse_index] = list_position->data.z[index];
        }
    }
};