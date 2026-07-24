#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL cmpnt_list_rigid_body*
    cmpnt_rigid_body_list_create(
        arena* a) {
        
        const u32 save     = arena_save(a);
        const u32 capacity = _cmpnt_mngr->capacity;

        auto list          = arena_push<cmpnt_list_rigid_body> (a);
        auto id            = arena_push<entity_id>             (a, capacity);
        auto sparse_index  = arena_push<u32>                   (a, capacity);
        auto origin_x      = arena_push<f32>                   (a, capacity);
        auto origin_y      = arena_push<f32>                   (a, capacity);
        auto origin_z      = arena_push<f32>                   (a, capacity);
        auto width         = arena_push<f32>                   (a, capacity);
        auto height        = arena_push<f32>                   (a, capacity);

        const bool did_create = (
            list         != NULL &&
            id           != NULL &&
            sparse_index != NULL &&
            origin_x     != NULL &&
            origin_y     != NULL &&
            origin_z     != NULL &&
            width        != NULL &&
            height       != NULL            
        );

        if (!did_create) {
            arena_revert(a, save);
            return(NULL);
        }

        arena_commit(a, save);
        
        list->data.id           = id;  
        list->data.sparse_index = sparse_index;  
        list->data.origin_x     = origin_x;  
        list->data.origin_y     = origin_y;  
        list->data.origin_z     = origin_z;  
        list->data.width        = width;  
        list->data.height       = height;  

        cmpnt_rigid_body_list_validate(list);

        return(list);
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_list_validate(
        const cmpnt_list_rigid_body* list_rb) {

        assert(list_rb                    != NULL);
        assert(list_rb->data.id           != NULL);  
        assert(list_rb->data.sparse_index != NULL);  
        assert(list_rb->data.origin_x     != NULL);  
        assert(list_rb->data.origin_y     != NULL);  
        assert(list_rb->data.origin_z     != NULL);  
        assert(list_rb->data.width        != NULL);  
        assert(list_rb->data.height       != NULL);  
    }

    IFB_INTERNAL bool
    cmpnt_rigid_body_list_add(
        cmpnt_list_rigid_body*  list_rb,
        const cmpnt_rigid_body& rb) {

        cmpnt_rigid_body_list_validate(list_rb);

        if (list_rb->count == _cmpnt_mngr->capacity) {
            return(false);
        }

        u32 index;
        for (
              index = 0;
              index < list_rb->count;
            ++index 
        ) {
            if (rb.id == list_rb->data.id[index]) {
                assert(rb.sparse_index == list_rb->data.sparse_index[index]);
                break;
            }
        }

        assert(index <= list_rb->count);

        list_rb->data.id           [index] = rb.id;
        list_rb->data.sparse_index [index] = rb.sparse_index;
        list_rb->data.origin_x     [index] = rb.origin_x;
        list_rb->data.origin_y     [index] = rb.origin_y;
        list_rb->data.origin_z     [index] = rb.origin_z;
        list_rb->data.width        [index] = rb.width;
        list_rb->data.height       [index] = rb.height;

        return(true);
    }
     
    IFB_INTERNAL void
    cmpnt_rigid_body_table_validate(
        void) {

        assert(_cmpnt_mngr                    != NULL);
        assert(_cmpnt_mngr->capacity          != 0);
        assert(_cmpnt_mngr->tables.rigid_body != NULL);

        auto tbl = _cmpnt_mngr->tables.rigid_body;

        assert(tbl->origin_x != NULL);
        assert(tbl->origin_y != NULL);
        assert(tbl->origin_z != NULL);
        assert(tbl->width    != NULL);
        assert(tbl->height   != NULL);
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_update(
        const cmpnt_rigid_body& rb) {

        cmpnt_rigid_body_table_validate();
        assert(rb.sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;

        tbl->origin_x [rb.sparse_index] = rb.origin_x;
        tbl->origin_y [rb.sparse_index] = rb.origin_y;
        tbl->origin_z [rb.sparse_index] = rb.origin_z;
        tbl->width    [rb.sparse_index] = rb.width;
        tbl->height   [rb.sparse_index] = rb.height;
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_update(
        rigid_body& rb,
        const u32   sparse_index) {

        cmpnt_rigid_body_table_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;

        tbl->origin_x [sparse_index] = rb.origin.x;
        tbl->origin_y [sparse_index] = rb.origin.y;
        tbl->origin_z [sparse_index] = rb.origin.z;
        tbl->width    [sparse_index] = rb.dimensions.width;
        tbl->height   [sparse_index] = rb.dimensions.height;
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_update(
        const cmpnt_list_rigid_body* list_rb) {

        cmpnt_rigid_body_table_validate();
        cmpnt_rigid_body_list_validate(list_rb);        

        auto tbl = _cmpnt_mngr->tables.rigid_body;
        for (
            u32 index = 0;
                index < list_rb->count;
              ++index) {

            const u32 sparse_index = list_rb->data.sparse_index[index];
            assert(sparse_index <= _entity_mngr->capacity.sparse);

            tbl->origin_x [sparse_index] = list_rb->data.origin_x [index];
            tbl->origin_y [sparse_index] = list_rb->data.origin_y [index];
            tbl->origin_z [sparse_index] = list_rb->data.origin_z [index];
            tbl->width    [sparse_index] = list_rb->data.width    [index];
            tbl->height   [sparse_index] = list_rb->data.height   [index];
        }
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_lookup(
        cmpnt_list_rigid_body* list_rb,
        const entity_list*     list_entity) {

        cmpnt_rigid_body_table_validate();
        entity_list_validate           (list_entity);
    

        const auto tbl = _cmpnt_mngr->tables.rigid_body;
        for (
            u32 index;
                index < list_entity->count;
              ++index) {

            const u32 sparse_index = list_entity->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            list_rb->data.id           [index] = list_entity->data.id [index];
            list_rb->data.sparse_index [index] = sparse_index;
            list_rb->data.origin_x     [index] = tbl->origin_x [sparse_index]; 
            list_rb->data.origin_y     [index] = tbl->origin_y [sparse_index];
            list_rb->data.origin_z     [index] = tbl->origin_z [sparse_index];
            list_rb->data.width        [index] = tbl->width    [sparse_index];
            list_rb->data.height       [index] = tbl->height   [sparse_index];
        } 

        list_rb->count = list_entity->count;
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_lookup(
        cmpnt_rigid_body& rb,
        const entity&     e) {

        cmpnt_rigid_body_table_validate();
        assert(e.index_sparse < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;
        assert(tbl != NULL);

        rb.id           = e.id;
        rb.sparse_index = e.index_sparse;
        rb.origin_x     = tbl->origin_x [e.index_sparse];
        rb.origin_y     = tbl->origin_y [e.index_sparse];
        rb.origin_z     = tbl->origin_z [e.index_sparse];
        rb.width        = tbl->width    [e.index_sparse];
        rb.height       = tbl->height   [e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_lookup(
        rigid_body& rb,
        const u32   sparse_index) {
            
        cmpnt_rigid_body_table_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        auto tbl = _cmpnt_mngr->tables.rigid_body;
        assert(tbl != NULL);

        rb.origin.x          = tbl->origin_x [sparse_index];
        rb.origin.y          = tbl->origin_y [sparse_index];
        rb.origin.z          = tbl->origin_z [sparse_index];
        rb.dimensions.width  = tbl->width    [sparse_index];
        rb.dimensions.height = tbl->height   [sparse_index];
    }
};