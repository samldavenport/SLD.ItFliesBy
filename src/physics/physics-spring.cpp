#pragma once

#include "collections-internal.hpp"
#include "component-tables.cpp"
#include "ifb-component.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "physics.hpp"
#include "memory.hpp"
#include "memory-arena.cpp"
#include "entity.hpp"
#include "sld-math-vec3.hpp"
#include "physics-entity.cpp"

namespace ifb {

    struct spring_calculator {
        u32        capacity;
        u32        count;
        entity_id* spring_id;
        f32*       pos_spring_x;
        f32*       pos_spring_y;
        f32*       pos_spring_z;
        f32*       pos_anchor_x;
        f32*       pos_anchor_y;
        f32*       pos_anchor_z;
        f32*       stiffness;
        f32*       damping;
        f32*       rest_length;
    };

    inline bool spring_calculator_create            (spring_calculator& calc, arena* a);
    inline bool spring_calculator_load_components   (spring_calculator& calc, arena* a);
    inline void spring_calculator_load_exec         (spring_calculator& calc);

    IFB_INTERNAL void 
    physics_spring_calculate_forces(
        arena* a) {

        assert(a);

        const u32 save = arena_save(a);

        spring_calculator calc;
        if (!spring_calculator_create(calc, a)) {
            arena_revert(a, save);
            return;
        }
    
        if(!spring_calculator_load_components(calc, a)) {
            arena_revert(a, save);
            return;
        }

        spring_calculator_load_exec(calc);
        arena_reset(a);
        return;
    }
    
    inline bool 
    spring_calculator_create(
        spring_calculator& calc,
        arena* a) {

        const auto& cfg = config_instance();
    
        calc.capacity     = cfg.entity_capacity;
        calc.count        = 0;
        calc.spring_id    = arena_push<entity_id> (a, calc.capacity);
        calc.pos_spring_x = arena_push<f32>       (a, calc.capacity);
        calc.pos_spring_y = arena_push<f32>       (a, calc.capacity);
        calc.pos_spring_z = arena_push<f32>       (a, calc.capacity);
        calc.pos_anchor_x = arena_push<f32>       (a, calc.capacity);
        calc.pos_anchor_y = arena_push<f32>       (a, calc.capacity);
        calc.pos_anchor_z = arena_push<f32>       (a, calc.capacity);
        calc.stiffness    = arena_push<f32>       (a, calc.capacity);
        calc.damping      = arena_push<f32>       (a, calc.capacity);
        calc.rest_length  = arena_push<f32>       (a, calc.capacity);
    
        const bool did_create = (
            calc.spring_id    != NULL && 
            calc.pos_spring_x != NULL && 
            calc.pos_spring_y != NULL && 
            calc.pos_spring_z != NULL && 
            calc.pos_anchor_x != NULL && 
            calc.pos_anchor_y != NULL && 
            calc.pos_anchor_z != NULL && 
            calc.stiffness    != NULL && 
            calc.damping      != NULL && 
            calc.rest_length  != NULL 
        );

        return(did_create);
    }
    
    inline bool 
    spring_calculator_load_components(
        spring_calculator& calc,
        arena* a) {

        const entity_archetype atype = (
            cmpnt_type_e_position &&
            cmpnt_type_e_spring
        );

        entity_list list;
        if (!list.arena_init(a)) {
            return(false);
        }

        entity_query query = { 0 };
        query.has_all = (
            cmpnt_type_e_position &&
            cmpnt_type_e_spring
        );

        if (!entity_lookup_list(list, query) || list.count() == 0) {
            return(false);
        }

        // we have entities with springs
        // so now we have to find their
        // anchor entities
        spring spr;     
        position_3d pos_spr;
        position_3d pos_anchor;
        for (
            u32 entity_index = 0;
                entity_index < list.count();
              ++entity_index 
        ) {
            // get the spring id and sparse index
            const entity_id spring_id           = list[entity_index];
            const u32       spring_sparse_index = entity_lookup_sparse_index(spring_id);
           
            // look up the spring info
            cmpnt_lookup_position (spring_sparse_index, pos_spr);
            cmpnt_lookup_spring   (spring_sparse_index, spr);
           
            // look up the anchor info
            const u32 anchor_sparse_index = entity_lookup_sparse_index(spr.anchor);
            assert(anchor_sparse_index != INVALID_INDEX);
            cmpnt_lookup_position(anchor_sparse_index, pos_anchor);

            // save the info in the calculator
            const u32 spring_index = calc.count;
            calc.spring_id    [spring_index] = spring_id;
            calc.pos_spring_x [spring_index] = pos_spr.x;
            calc.pos_spring_y [spring_index] = pos_spr.y;    
            calc.pos_spring_z [spring_index] = pos_spr.z;
            calc.pos_anchor_x [spring_index] = pos_anchor.x;
            calc.pos_anchor_y [spring_index] = pos_anchor.y;
            calc.pos_anchor_z [spring_index] = pos_anchor.z;
            calc.stiffness    [spring_index] = spr.stiffness;
            calc.damping      [spring_index] = spr.damping;
            calc.rest_length  [spring_index] = spr.rest_length;

            // update the count
            ++calc.count;
        }
    
        assert(
            calc.count <= calc.capacity && 
            calc.count <= list.count()
        );

        return(true);
    }

    inline void
    spring_calculator_load_exec(
        spring_calculator& calc) {

        vec3 delta;
        vec3 direction;
        vec3 spring_force;
        for (
            u32 index = 0;
                index < calc.count;
              ++index
        ) {
     
            delta.x = calc.pos_anchor_x[index] - calc.pos_spring_x[index];
            delta.y = calc.pos_anchor_y[index] - calc.pos_spring_y[index];
            delta.z = calc.pos_anchor_z[index] - calc.pos_spring_z[index];

            const f32 distance  = vec3_magnitude(delta);
            if (distance < 0.001) {
                continue;
            }
            const f32 extension = distance - calc.rest_length[index]; 
            const f32 scalar    = calc.stiffness[index] * extension;
            
            direction    = vec3_scalar_multiply(delta, 1.0f / distance);
            spring_force = vec3_scalar_multiply(direction, scalar); 
       
            physics_entity_add_force(calc.spring_id[index], spring_force);
        }
    }

    inline void
    spring_calculator_update_components(
        spring_calculator& calc) {
        
    }
};
