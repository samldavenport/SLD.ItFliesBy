#pragma once

#include "components.hpp"

namespace ifb {

    IFB_INTERNAL component_list_quad*
    component_quad_list_create(
        arena* a) {

        assert(a);

        const u32 save = arena_save(a);

        auto list   = arena_push<component_list_quad> (a);
        auto id     = arena_push<entity_id>           (a);
        auto width  = arena_push<u32>                 (a, _entity_mngr->capacity.dense);
        auto height = arena_push<u32>                 (a, _entity_mngr->capacity.dense);

        const bool did_create = (
            list   != NULL &&
            id     != NULL &&
            width  != NULL &&
            height != NULL            
        );

        if (!did_create) {
            arena_revert(a, save);
        }

        arena_commit(a, save);

        list->count       = 0;
        list->data.id     = id;
        list->data.width  = width;
        list->data.height = height;
    
        return(list);
    }

    IFB_INTERNAL void
    component_quad_list_validate(
        const component_list_quad* list_quad) {

        assert(
            list_quad              != NULL &&
            list_quad->data.id     != NULL &&
            list_quad->data.width  != NULL &&
            list_quad->data.height != NULL &&
            list_quad->count       <= _cmpnt_mngr->capacity
        );
    }

    IFB_INTERNAL void
    component_quad_table_update(
        const component_list_quad* list_quad) {

        component_quad_list_validate(list_quad);
        assert(
            _cmpnt_mngr->capacity    != 0    &&
            _cmpnt_mngr->tables.quad != NULL
        );
    }

    IFB_INTERNAL void
    component_quad_table_lookup(
        component_list_quad* list_quad,
        const entity_list*   list_entity) {

        component_quad_list_validate(list_quad);
        assert(
            _cmpnt_mngr->capacity    != 0 &&
            _cmpnt_mngr->tables.quad != NULL
        );

    }

};