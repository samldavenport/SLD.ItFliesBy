#pragma once

#include "collection-entity-list.cpp"
#include "ifb-collections.hpp"
#include "component-tables.cpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "physics.hpp"
#include "memory.hpp"
#include "memory-arena.cpp"
#include "entity.hpp"
#include "sld-math-vec3.hpp"
#include "physics-entity.cpp"
#include "physics-internal.hpp"

namespace ifb {

    struct physics_spring_calculator {
        entity_list* entt_list;
        u32          capacity;
        u32          count;
        entity_id*   spring_id;
        f32*         pos_spring_x;
        f32*         pos_spring_y;
        f32*         pos_spring_z;
        f32*         pos_anchor_x;
        f32*         pos_anchor_y;
        f32*         pos_anchor_z;
        f32*         vel_spring_x;
        f32*         vel_spring_y;
        f32*         vel_spring_z;
        f32*         stiffness;
        f32*         damping;
        f32*         rest_length;
    };

    inline bool spring_calculator_load_components   (physics_spring_calculator* calc);
    inline void spring_calculator_load_exec         (physics_spring_calculator* calc);

    IFB_INTERNAL void 
    physics_spring_calculator_run(
        physics_spring_calculator* calc) {

        assert(calc);

        if(!spring_calculator_load_components(calc)) {
            return;
        }

        spring_calculator_load_exec(calc);

        calc->count = 0;
        entity_list_reset(calc->entt_list);
        return;
    }
   
    IFB_INTERNAL physics_spring_calculator*
    physics_spring_calculator_create(
        void) {

        const auto& cfg = config_instance();
   
        const u32 size_struct      = sizeof(physics_spring_calculator);
        const u32 size_array_ids   = cfg.entity_capacity * sizeof(entity_id);
        const u32 size_array_props = cfg.entity_capacity * sizeof(u32);
        const u32 size_total       = size_struct + size_array_ids + (size_array_props * 12); 

        auto mem_addr = (addr)physics_mngr_res_alloc(size_total);
        assert(mem_addr != 0);

        auto calc = (physics_spring_calculator*)mem_addr;
        calc->capacity     = cfg.entity_capacity;
        calc->count        = 0;
        calc->spring_id    = (entity_id*)(mem_addr += size_struct);
        calc->pos_spring_x =       (f32*)(mem_addr += size_array_ids);
        calc->pos_spring_y =       (f32*)(mem_addr += size_array_props);
        calc->pos_spring_z =       (f32*)(mem_addr += size_array_props);
        calc->pos_anchor_x =       (f32*)(mem_addr += size_array_props);
        calc->pos_anchor_y =       (f32*)(mem_addr += size_array_props);
        calc->pos_anchor_z =       (f32*)(mem_addr += size_array_props);
        calc->vel_spring_x =       (f32*)(mem_addr += size_array_props);
        calc->vel_spring_y =       (f32*)(mem_addr += size_array_props);
        calc->vel_spring_z =       (f32*)(mem_addr += size_array_props);
        calc->stiffness    =       (f32*)(mem_addr += size_array_props);
        calc->damping      =       (f32*)(mem_addr += size_array_props);
        calc->rest_length  =       (f32*)(mem_addr += size_array_props);
        assert(calc->spring_id    != NULL);
        assert(calc->pos_spring_x != NULL);
        assert(calc->pos_spring_y != NULL);
        assert(calc->pos_spring_z != NULL);
        assert(calc->pos_anchor_x != NULL);
        assert(calc->pos_anchor_y != NULL);
        assert(calc->pos_anchor_z != NULL);
        assert(calc->vel_spring_x != NULL);
        assert(calc->vel_spring_y != NULL);
        assert(calc->vel_spring_z != NULL);
        assert(calc->stiffness    != NULL);
        assert(calc->damping      != NULL);
        assert(calc->rest_length  != NULL);

        // create the entity list
        memory list_mem;
        list_mem.size = entity_list_mem_req();
        list_mem.ptr  = physics_mngr_res_alloc(list_mem.size);
        assert(list_mem.ptr != NULL);
        calc->entt_list = entity_list_memory_create(list_mem);
        assert(calc->entt_list);

        return(calc);
    }
    
    inline bool 
    spring_calculator_load_components(
        physics_spring_calculator* calc) {

        entity_query query = { 0 };
        query.has_all = (
            cmpnt_type_e_position |
            cmpnt_type_e_spring
        );

        if (!entity_lookup_list(calc->entt_list, query) || entity_list_count(calc->entt_list) == 0) {
            return(false);
        }

        // we have entities with springs
        // so now we have to find their
        // anchor entities
        cmpnt_spring   spr;     
        cmpnt_position pos_spr;
        cmpnt_position pos_anchor;
        cmpnt_velocity vel_spr;

        const u32 list_count = entity_list_count(calc->entt_list);
        for (
            u32 entity_index = 0;
                entity_index < list_count;
              ++entity_index 
        ) {
            // get the spring id and sparse index
            const entity_id spring_id           = entity_list_index(calc->entt_list, entity_index);
            const u32       spring_sparse_index = entity_lookup_sparse_index(spring_id);
           
            // look up the spring info
            cmpnt_lookup_position (spring_sparse_index, pos_spr);
            cmpnt_lookup_spring   (spring_sparse_index, spr);
            cmpnt_lookup_velocity (spring_sparse_index, vel_spr);
           
            // look up the anchor info
            const u32 anchor_sparse_index = entity_lookup_sparse_index(spr.anchor);
            assert(anchor_sparse_index != INVALID_INDEX);
            cmpnt_lookup_position(anchor_sparse_index, pos_anchor);

            // save the info in the calculator
            const u32 spring_index = calc->count++;
            calc->spring_id    [spring_index] = spring_id;
            calc->pos_spring_x [spring_index] = pos_spr.x;
            calc->pos_spring_y [spring_index] = pos_spr.y;    
            calc->pos_spring_z [spring_index] = pos_spr.z;
            calc->pos_anchor_x [spring_index] = pos_anchor.x;
            calc->pos_anchor_y [spring_index] = pos_anchor.y;
            calc->pos_anchor_z [spring_index] = pos_anchor.z;
            calc->vel_spring_x [spring_index] = vel_spr.x;
            calc->vel_spring_y [spring_index] = vel_spr.y;
            calc->vel_spring_z [spring_index] = vel_spr.z;
            calc->stiffness    [spring_index] = spr.stiffness;
            calc->damping      [spring_index] = spr.damping;
            calc->rest_length  [spring_index] = spr.rest_length;
        }
    
        assert(
            calc->count <= calc->capacity && 
            calc->count <= entity_list_count(calc->entt_list) 
        );

        return(true);
    }

    inline void
    spring_calculator_load_exec(
        physics_spring_calculator* calc) {

        vec3 delta;
        vec3 direction;
        vec3 spring_force;
        vec3 vel;
        
        for (
            u32 index = 0;
                index < calc->count;
              ++index) {
    
            // calculate difference between anchor and spring positions
            delta.x = calc->pos_anchor_x[index] - calc->pos_spring_x[index];
            delta.y = calc->pos_anchor_y[index] - calc->pos_spring_y[index];
            delta.z = calc->pos_anchor_z[index] - calc->pos_spring_z[index];

            // calculate the distance between anchor and spring
            const f32 distance  = vec3_magnitude(delta);
            if (distance < 0.001) continue;

            // calculate the direction and velocity along the spring
            direction = vec3_scalar_multiply(delta, 1.0f / distance);
            vel.x     = calc->vel_spring_x[index];
            vel.y     = calc->vel_spring_y[index];
            vel.z     = calc->vel_spring_z[index];
            const f32 vel_along_spring = vec3_dot(vel, direction); 

            // calculate the damping and force scalars
            const f32 extension      = distance - calc->rest_length[index]; 
            const f32 spring_scalar  =  calc->stiffness[index] * extension;
            const f32 damping_scalar = -calc->damping  [index] * vel_along_spring;
            const f32 force_scalar   = spring_scalar + damping_scalar;  

            // calculate the spring force and add it to the entity
            spring_force = vec3_scalar_multiply(direction, force_scalar); 
            physics_entity_add_force(calc->spring_id[index], spring_force);
        }
    }
};
