#pragma once

#include "component.hpp"
#include "entity.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_acceleration_list_validate(
        const cmpnt_list_acceleration* list) {

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

    IFB_INTERNAL cmpnt_list_acceleration*
    cmpnt_acceleration_list_create(
        arena* arena) {

        assert(arena != NULL);

        const u32 save     = arena_save(arena);
        const u32 capacity = _entity_mngr->capacity.dense;

        auto list        = arena_push<cmpnt_list_acceleration> (arena);
        auto array_id    = arena_push<entity_id>           (arena, capacity);
        auto array_index = arena_push<u32>                 (arena, capacity);
        auto array_x     = arena_push<f32>                 (arena, capacity);
        auto array_y     = arena_push<f32>                 (arena, capacity);
        auto array_z     = arena_push<f32>                 (arena, capacity);

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

        arena_commit(arena, save);

        list->data.id           = array_id; 
        list->data.sparse_index = array_index; 
        list->data.x            = array_x; 
        list->data.y            = array_y; 
        list->data.z            = array_z; 
        list->count             = 0;

        cmpnt_acceleration_list_validate(list);

        return(list);
    }

    IFB_INTERNAL bool
    cmpnt_acceleration_list_add(
        cmpnt_list_acceleration*  list_acceleration,
        const cmpnt_acceleration& acceleration) {

        cmpnt_acceleration_list_validate(list_acceleration);

        if (list_acceleration->count == _cmpnt_mngr->capacity) {
            return(false);
        }

        u32 index;
        for (
              index = 0;
              index < list_acceleration->count;
            ++index 
        ) {
            if (acceleration.id == list_acceleration->data.id[index]) {
                assert(acceleration.sparse_index == list_acceleration->data.sparse_index[index]);
                break;
            }
        }

        assert(index <= list_acceleration->count);

        list_acceleration->data.id           [index] = acceleration.id;
        list_acceleration->data.sparse_index [index] = acceleration.sparse_index;
        list_acceleration->data.x            [index] = acceleration.x;
        list_acceleration->data.y            [index] = acceleration.y;
        list_acceleration->data.z            [index] = acceleration.z;
        ++list_acceleration->count;

        return(true);
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_validate(
        void) {

        assert(
            _cmpnt_mngr                  != NULL &&
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.acceleration != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_lookup(
        cmpnt_list_acceleration* list_acceleration,
        const entity_list*       list_entity) {

        cmpnt_acceleration_list_validate (list_acceleration);
        entity_list_validate             (list_entity);
    
        assert(
            _cmpnt_mngr->capacity        != 0  &&
            _cmpnt_mngr->tables.acceleration != NULL
        );

        const auto tbl = _cmpnt_mngr->tables.acceleration;
        for (
            u32 index;
                index < list_entity->count;
              ++index) {

            const u32 sparse_index = list_entity->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            list_acceleration->data.id           [index] = list_entity->data.id [index];
            list_acceleration->data.sparse_index [index] = sparse_index;
            list_acceleration->data.x            [index] = tbl->x [sparse_index];
            list_acceleration->data.y            [index] = tbl->y [sparse_index];
            list_acceleration->data.z            [index] = tbl->z [sparse_index];
        }

        list_acceleration->count = list_entity->count;
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_update(
        cmpnt_list_acceleration* list_acceleration) {

        cmpnt_acceleration_list_validate (list_acceleration);

        assert(
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.acceleration != NULL
        );

        auto tbl = _cmpnt_mngr->tables.acceleration;
        for (
            u32 index = 0;
                index < list_acceleration->count;
              ++index) {

            const u32 sparse_index = list_acceleration->data.sparse_index[index];
            assert(sparse_index <= _entity_mngr->capacity.sparse);

            tbl->x[sparse_index] = list_acceleration->data.x[index];
            tbl->y[sparse_index] = list_acceleration->data.y[index];
            tbl->z[sparse_index] = list_acceleration->data.z[index];
        }
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_update(
        const cmpnt_acceleration& acceleration) {

        cmpnt_acceleration_table_validate();
        assert(acceleration.sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_acceleration* tbl = _cmpnt_mngr->tables.acceleration;

        tbl->x[acceleration.sparse_index] = acceleration.x; 
        tbl->y[acceleration.sparse_index] = acceleration.y; 
        tbl->z[acceleration.sparse_index] = acceleration.z; 
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_update(
        const acceleration_3d& acceleration,
        const u32          sparse_index) {

        cmpnt_acceleration_table_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_acceleration* tbl = _cmpnt_mngr->tables.acceleration;

        tbl->x[sparse_index] = acceleration.x; 
        tbl->y[sparse_index] = acceleration.y; 
        tbl->z[sparse_index] = acceleration.z; 
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_lookup(
        cmpnt_acceleration& acceleration,
        const entity&   e) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > e.index_sparse
        );

        auto tbl = _cmpnt_mngr->tables.acceleration;
        assert(tbl != NULL);

        acceleration.id           = e.id;
        acceleration.sparse_index = e.index_sparse;
        acceleration.x            = tbl->x[e.index_sparse];
        acceleration.y            = tbl->y[e.index_sparse];
        acceleration.z            = tbl->z[e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_acceleration_table_lookup(
        acceleration_3d& acceleration,
        const u32    sparse_index) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity >  sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.acceleration;
        assert(tbl != NULL);
        
        acceleration.x = tbl->x[sparse_index];
        acceleration.y = tbl->y[sparse_index];
        acceleration.z = tbl->z[sparse_index];
    }
};