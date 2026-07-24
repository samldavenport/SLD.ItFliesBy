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

            physics_world_validate(world);
        }
        return(world); 
    }

    IFB_INTERNAL void 
    physics_world_validate(
        const physics_world* world)  {

        assert(world);
        assert(world->arena);
        world->entity_list_dynamic.validate();
        world->entity_list_static.validate();
    }


    IFB_INTERNAL void
    physics_world_destroy(
        physics_world* world) {

        physics_manager_validate();
        physics_world_validate(world);

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
        const u32      dt_ms) {

        physics_world_validate(world);

        //TODO(SAM)
    }

    IFB_INTERNAL bool
    physics_world_add_entity_dynamic(
        physics_world*  world,
        const entity_id id) {

        physics_world_validate(world);

        assert(world);
        assert(id != ENTITY_ID_INVALID);

        // make sure we can add the entity
        // to the dynamic list
        const bool can_add = (
            !world->entity_list_dynamic.contains(id) &&
            !world->entity_list_dynamic.is_full()
        );

        if (!can_add) {
            return(false);
        }

        // add the dynamic components to the 
        // entity
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

        // ensure this entity is not in the static list
        u32 static_index = 0;
        if (world->entity_list_static.index_of(id, static_index)) {
            world->entity_list_static.remove_at(static_index);
        }

        // add the entity to the dynamic list
        // and return
        assert(world->entity_list_dynamic.add(id));
        return(true);
    }

    IFB_INTERNAL bool
    physics_world_add_entity_static(
        physics_world*  world,
        const entity_id id) {

        physics_world_validate(world);
        
        assert(world);
        assert(id != ENTITY_ID_INVALID);

        // make sure we can add the entity
        // to the static list
        const bool can_add = (
            !world->entity_list_static.contains(id) &&
            !world->entity_list_static.is_full()
        );

        // add the static components
        // and remove the dynamic components
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

        // ensure this entity is not in the dynamic list
        u32 dynamic_index = 0;
        if (world->entity_list_dynamic.index_of(id, dynamic_index)) {
            world->entity_list_dynamic.remove_at(dynamic_index);
        }        

        // add the entity to the dynamic list
        // and return
        assert(world->entity_list_dynamic.add(id));
        return(true);
    }

    IFB_INTERNAL bool
    physics_world_remove_entity(
        physics_world*  world,
        const entity_id id) {

        physics_world_validate(world);
        assert(id != ENTITY_ID_INVALID);

        bool did_remove = false;


        u32 index = 0;

        // remove the entity from the dynamic list if it exists
        // and ensure it is not in the static list
        if (world->entity_list_dynamic.index_of(id, index)) {
            world->entity_list_dynamic.remove_at(index);
            assert(!world->entity_list_static.contains(id));
            did_remove = true;
        }

        // remove the entity from the static list if it exists
        // and ensure it is not in the dynamic list
        else if (world->entity_list_static.index_of(id, index)) {
            world->entity_list_static.remove_at(index);
            assert(!world->entity_list_dynamic.contains(id));
            did_remove = true;
        }

        // if the entity did not exist in either list, we're done
        if (!did_remove) {
            return(false);
        }

        // ensure the physics components are removed
        const component_type cmpnts = (
            cmpnt_type_e_rigid_body   |
            cmpnt_type_e_velocity     |
            cmpnt_type_e_acceleration
        );
        assert(entity_component_remove(id, cmpnts));

        return(true);
    }

};