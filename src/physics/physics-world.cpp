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

            world->arena               = arena_alloc();
            world->entity_list_dynamic = entity_list_create(world->arena);
            world->entity_list_static  = entity_list_create(world->arena);

            assert(world->arena               != NULL);
            assert(world->entity_list_dynamic != NULL);
            assert(world->entity_list_static  != NULL);
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
    }

    IFB_INTERNAL bool
    physics_world_add_entity_static(
        physics_world*  world,
        const entity_id id) {

    }


};