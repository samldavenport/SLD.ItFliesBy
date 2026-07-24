#pragma once

#include "physics.hpp"

namespace ifb {
    
    IFB_INTERNAL physics_world*
    physics_world_create(
        void) {

        physics_manager_validate();

        auto mem = _phys_mngr->memory;
    
        auto world = (physics_world*)block_alloc(&mem->world_allocator);
        if (world) {

            physics_world* next = _phys_mngr->world_list;

            world->next = _phys_mngr->world_list;
            world->prev = NULL;

            if (next != NULL) {
                next->prev = world;
            }

            world->next = next;
            _phys_mngr->world_list = world;

            world->arena = arena_alloc();
            assert(world->arena != NULL);

            const u32 entity_list_capacity = entity_mngr_capacity_dense();
            assert(entity_list_capacity != 0);

            auto dynamic_entities = arena_push<entity_id>(world->arena, entity_list_capacity);
            auto static_entities  = arena_push<entity_id>(world->arena, entity_list_capacity);
            assert(dynamic_entities);
            assert(static_entities);

            world->entity_list_dynamic.init(dynamic_entities, entity_list_capacity);
            world->entity_list_dynamic.init(static_entities,  entity_list_capacity);
        }
        return(world); 
    }

    IFB_INTERNAL void
    physics_world_destroy(
        physics_world* world) {

        assert(world);
        physics_manager_validate();

        physics_world* next = world->next;
        physics_world* prev = world->prev;

        arena_free(world->arena);
        world->next  = NULL;
        world->prev  = NULL;
        world->arena = NULL;

        if (next) next->prev = prev;
        if (prev) prev->next = next;
        if (_phys_mngr->world_list == world) {
            assert(prev == NULL);
            _phys_mngr->world_list = next; 
        }
    }

    IFB_INTERNAL void
    physics_world_simulate(
        physics_world* world,
        const u32 dt_ms) {

        //TODO(SAM)
    }

    IFB_INTERNAL bool
    physics_world_add_entity_dynamic(
        physics_world*  world,
        const entity_id id) {

        assert(world);
        assert(id != ENTITY_ID_INVALID);

        const bool can_add = (
            !world->entity_list_dynamic.contains(id) &&
            !world->entity_list_dynamic.is_full()
        );

        if (!can_add) {
            return(false);
        }

        const component_type dynamic_type = (
            cmpnt_type_e_position   |
            cmpnt_type_e_rigid_body |
            cmpnt_type_e_velocity   |
            cmpnt_type_e_acceleration
        );
        const bool did_add = entity_component_add(id, dynamic_type);
        if (!did_add) {
            return(false);
        }

        assert(world->entity_list_dynamic.add(id));

        return(true);
    }

    IFB_INTERNAL bool
    physics_world_add_entity_static(
        physics_world*  world,
        const entity_id id) {

        assert(world);
        assert(id != ENTITY_ID_INVALID);

        const bool can_add = (
            !world->entity_list_static.contains(id) &&
            !world->entity_list_static.is_full()
        );

        const component_type dynamic_type = (cmpnt_type_e_velocity | cmpnt_type_e_acceleration);
        const component_type static_type  = (cmpnt_type_e_position | cmpnt_type_e_rigid_body);

        // we want to ensure there are no dynamic components
        // only static components should be on this enitity
        bool did_add = true;
        did_add &= entity_component_remove (id, dynamic_type);
        did_add &= entity_component_add    (id, static_type);
        if (!did_add) {
            return(false);
        }

        

        return(did_add);
    }


};