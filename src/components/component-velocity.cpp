#pragma once

#include "component.hpp"
#include "entity.hpp"

namespace ifb {

    IFB_INTERNAL void
    cmpnt_velocity_list_validate(
        const cmpnt_list_velocity* list) {

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

    IFB_INTERNAL cmpnt_list_velocity*
    cmpnt_velocity_list_create(
        arena* arena) {

        assert(arena != NULL);

        const u32 save     = arena_save(arena);
        const u32 capacity = _entity_mngr->capacity.dense;

        auto list        = arena_push<cmpnt_list_velocity> (arena);
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

        cmpnt_velocity_list_validate(list);

        return(list);
    }

    IFB_INTERNAL bool
    cmpnt_velocity_list_add(
        cmpnt_list_velocity*  list_velocity,
        const cmpnt_velocity& velocity) {

        cmpnt_velocity_list_validate(list_velocity);

        if (list_velocity->count == _cmpnt_mngr->capacity) {
            return(false);
        }

        u32 index;
        for (
              index = 0;
              index < list_velocity->count;
            ++index 
        ) {
            if (velocity.id == list_velocity->data.id[index]) {
                assert(velocity.sparse_index == list_velocity->data.sparse_index[index]);
                break;
            }
        }

        assert(index <= list_velocity->count);

        list_velocity->data.id           [index] = velocity.id;
        list_velocity->data.sparse_index [index] = velocity.sparse_index;
        list_velocity->data.x            [index] = velocity.x;
        list_velocity->data.y            [index] = velocity.y;
        list_velocity->data.z            [index] = velocity.z;
        ++list_velocity->count;

        return(true);
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_validate(
        void) {

        assert(
            _cmpnt_mngr                  != NULL &&
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.velocity != NULL
        );
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_lookup(
        cmpnt_list_velocity* list_velocity,
        const entity_list*       list_entity) {

        cmpnt_velocity_list_validate (list_velocity);
        entity_list_validate             (list_entity);
    
        assert(
            _cmpnt_mngr->capacity        != 0  &&
            _cmpnt_mngr->tables.velocity != NULL
        );

        const auto tbl = _cmpnt_mngr->tables.velocity;
        for (
            u32 index;
                index < list_entity->count;
              ++index) {

            const u32 sparse_index = list_entity->data.sparse_index[index];
            assert(sparse_index < _cmpnt_mngr->capacity);

            list_velocity->data.id           [index] = list_entity->data.id [index];
            list_velocity->data.sparse_index [index] = sparse_index;
            list_velocity->data.x            [index] = tbl->x [sparse_index];
            list_velocity->data.y            [index] = tbl->y [sparse_index];
            list_velocity->data.z            [index] = tbl->z [sparse_index];
        }

        list_velocity->count = list_entity->count;
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_update(
        cmpnt_list_velocity* list_velocity) {

        cmpnt_velocity_list_validate (list_velocity);

        assert(
            _cmpnt_mngr->capacity        != 0    &&
            _cmpnt_mngr->tables.velocity != NULL
        );

        auto tbl = _cmpnt_mngr->tables.velocity;
        for (
            u32 index = 0;
                index < list_velocity->count;
              ++index) {

            const u32 sparse_index = list_velocity->data.sparse_index[index];
            assert(sparse_index <= _entity_mngr->capacity.sparse);

            tbl->x[sparse_index] = list_velocity->data.x[index];
            tbl->y[sparse_index] = list_velocity->data.y[index];
            tbl->z[sparse_index] = list_velocity->data.z[index];
        }
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_update(
        const cmpnt_velocity& velocity) {

        cmpnt_velocity_table_validate();
        assert(velocity.sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_velocity* tbl = _cmpnt_mngr->tables.velocity;

        tbl->x[velocity.sparse_index] = velocity.x; 
        tbl->y[velocity.sparse_index] = velocity.y; 
        tbl->z[velocity.sparse_index] = velocity.z; 
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_update(
        const velocity_3d& velocity,
        const u32          sparse_index) {

        cmpnt_velocity_table_validate();
        assert(sparse_index < _cmpnt_mngr->capacity);

        cmpnt_table_velocity* tbl = _cmpnt_mngr->tables.velocity;

        tbl->x[sparse_index] = velocity.x; 
        tbl->y[sparse_index] = velocity.y; 
        tbl->z[sparse_index] = velocity.z; 
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_lookup(
        cmpnt_velocity& velocity,
        const entity&   e) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity > e.index_sparse
        );

        auto tbl = _cmpnt_mngr->tables.velocity;
        assert(tbl != NULL);

        velocity.id           = e.id;
        velocity.sparse_index = e.index_sparse;
        velocity.x            = tbl->x[e.index_sparse];
        velocity.y            = tbl->y[e.index_sparse];
        velocity.z            = tbl->z[e.index_sparse];
    }

    IFB_INTERNAL void
    cmpnt_velocity_table_lookup(
        velocity_3d& velocity,
        const u32    sparse_index) {

        assert(
            _cmpnt_mngr           != NULL &&
            _cmpnt_mngr->capacity != 0    &&
            _cmpnt_mngr->capacity >  sparse_index
        );

        auto tbl = _cmpnt_mngr->tables.velocity;
        assert(tbl != NULL);
        
        velocity.x = tbl->x[sparse_index];
        velocity.y = tbl->y[sparse_index];
        velocity.z = tbl->z[sparse_index];
    }
};