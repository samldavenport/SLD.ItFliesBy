#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL component_list_color*
    component_color_list_create(
        arena* a) {

        assert(a);

        const u32 save = arena_save(a);

        auto list    = arena_push<component_list_color> (a);
        auto ids     = arena_push<entity_id>            (a, _entity_mngr->capacity.dense); 
        auto indexes = arena_push<u32>                  (a, _entity_mngr->capacity.dense);
        auto colors  = arena_push<color_rgba_u32>       (a, _entity_mngr->capacity.dense);

        const bool did_create = (
            list    != NULL &&
            ids     != NULL &&            
            indexes != NULL &&            
            colors  != NULL            
        );

        if (!did_create) {
            arena_revert(a, save);
            return(NULL);
        }

        arena_commit(a, save);

        list->count             = 0;
        list->data.id           = ids;
        list->data.sparse_index = indexes;
        list->data.color        = colors;
        return(list); 
    }

    IFB_INTERNAL void
    component_color_list_validate(
        const component_list_color* list_color) {

        assert(
            list_color                    != NULL                         &&
            list_color->count             <= _entity_mngr->capacity.dense &&
            list_color->data.id           != NULL                         &&
            list_color->data.sparse_index != NULL                         &&
            list_color->data.color        != NULL
        );
    }

    IFB_INTERNAL bool
    component_color_list_add(
        component_list_color* list_color,
        const component_color&      color) {

        component_color_list_validate(list_color);

        if (list_color->count == _cmpnt_mngr->capacity) {
            return(false);
        }

        u32 index;
        for (
              index = 0;
              index < list_color->count;
            ++index 
        ) {
            if (color.id == list_color->data.id[index]) {
                assert(color.sparse_index == list_color->data.sparse_index[index]);
                break;
            }
        }

        assert(index <= list_color->count);

        list_color->data.id           [index]     = color.id;
        list_color->data.sparse_index [index]     = color.sparse_index;
        list_color->data.color        [index].hex = color.hex_rgba;
        ++list_color->count;

        return(true);
    }

    IFB_INTERNAL void
    component_color_table_update(
        const component_list_color* list_color) {

        component_color_list_validate(list_color);
        assert(list_color->count <= _cmpnt_mngr->capacity);

        auto& tbl = _cmpnt_mngr->tables.color;

        for (
            u32 index = 0;
                index < list_color->count;
              ++index
        ) {
            const u32 sparse_index = list_color->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            tbl->rgba_hex[sparse_index] = list_color->data.color[index].hex;
        }
    }

    IFB_INTERNAL void
    component_color_table_lookup(
        component_list_color* list_color,
        const entity_list*    list_entity) {

        component_color_list_validate (list_color);
        entity_list_validate          (list_entity);
        assert(
            _cmpnt_mngr->capacity     != 0 &&
            _cmpnt_mngr->tables.color != NULL
        );

        const auto tbl = _cmpnt_mngr->tables.color;

        for (
            u32 index = 0;
                index < list_entity->count;
              ++index
        ) {

            const u32 sparse_index = list_entity->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);
            
            list_color->data.id           [index]     = list_entity->data.id[index];
            list_color->data.sparse_index [index]     = sparse_index;
            list_color->data.color        [index].hex = tbl->rgba_hex[sparse_index];
        }

        list_color->count = list_entity->count;
    }
};