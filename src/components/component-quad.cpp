#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL cmpnt_list_quad*
    cmpnt_quad_list_create(
        arena* a) {

        assert(a);

        const u32 save = a->save();

        auto list   = a->push<cmpnt_list_quad> ();
        auto id     = a->push<entity_id>       ();
        auto index  = a->push<u32>             (_entity_mngr->capacity.dense);
        auto width  = a->push<u32>             (_entity_mngr->capacity.dense);
        auto height = a->push<u32>             (_entity_mngr->capacity.dense);

        const bool did_create = (
            list   != NULL &&
            id     != NULL &&
            index  != NULL &&
            width  != NULL &&
            height != NULL
        );

        if (!did_create) {
            a->revert(save);
            return(NULL);
        }

        a->commit(save);

        list->count             = 0;
        list->data.id           = id;
        list->data.sparse_index = index;
        list->data.width        = width;
        list->data.height       = height;

        return(list);
    }

    IFB_INTERNAL void
    cmpnt_quad_list_validate(
        const cmpnt_list_quad* list_quad) {

        assert(
            list_quad              != NULL &&
            list_quad->data.id     != NULL &&
            list_quad->data.width  != NULL &&
            list_quad->data.height != NULL &&
            list_quad->count       <= _cmpnt_mngr->capacity
        );
    }

    IFB_INTERNAL bool
    cmpnt_quad_list_add(
        cmpnt_list_quad*  list_quad,
        const component_quad& quad) {

        cmpnt_quad_list_validate(list_quad);

        if (list_quad->count == _cmpnt_mngr->capacity) {
            return(false);
        }
        
        u32 index;
        for (
              index = 0;
              index < list_quad->count;
            ++index 
        ) {
            if (quad.id == list_quad->data.id[index]) {
                assert(quad.sparse_index == list_quad->data.sparse_index[index]);
                break;
            }
        }

        assert(index <= list_quad->count);

        list_quad->data.id           [index] = quad.id;
        list_quad->data.sparse_index [index] = quad.sparse_index;
        list_quad->data.width        [index] = quad.width;
        list_quad->data.height       [index] = quad.height;
        ++list_quad->count;

        return(true);
    }

    IFB_INTERNAL void
    cmpnt_quad_table_update(
        const cmpnt_list_quad* list_quad) {

        cmpnt_quad_list_validate(list_quad);
        assert(
            _cmpnt_mngr->capacity    != 0    &&
            _cmpnt_mngr->tables.quad != NULL
        );

        auto& tbl = _cmpnt_mngr->tables.quad;

        for (
            u32 index = 0;
                index < list_quad->count;
              ++index) {

            const u32 sparse_index = list_quad->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            tbl->height [sparse_index] = list_quad->data.height [index];
            tbl->width  [sparse_index] = list_quad->data.width  [index];
        }
    }

    IFB_INTERNAL void
    cmpnt_quad_table_lookup(
        cmpnt_list_quad* list_quad,
        const entity_list*   list_entity) {

        cmpnt_quad_list_validate(list_quad);
        assert(
            _cmpnt_mngr->capacity    != 0 &&
            _cmpnt_mngr->tables.quad != NULL
        );

        auto& tbl = _cmpnt_mngr->tables.quad;

        for (
            u32 index = 0;
                index < list_quad->count;
              ++index) {

            const u32 sparse_index = list_entity->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            list_quad->data.id           [index] = list_entity->data.id[index];
            list_quad->data.sparse_index [index] = sparse_index; 
            list_quad->data.height       [index] = tbl->height [sparse_index];
            list_quad->data.width        [index] = tbl->width  [sparse_index];
        }

        list_quad->count = list_entity->count;
    }
};