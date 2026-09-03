#pragma once

#include <math.h>

#include "component.hpp"
#include "ifb-component.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "physics.hpp"
#include "entity.hpp"
#include "physics-accumulator.cpp"

namespace ifb {

    struct physics_force_integrator {
        u32        count;
        entity_id* id;
        u32*       sparse_index;
        f32*       pos_x;
        f32*       pos_y;
        f32*       pos_z;
        f32*       vel_x;
        f32*       vel_y;
        f32*       vel_z;
        f32*       acc_x;
        f32*       acc_y;
        f32*       acc_z;
        f32*       frc_x;
        f32*       frc_y;
        f32*       frc_z;
        f32*       tv_x;
        f32*       tv_y;
        f32*       tv_z;
        f32*       inv_mass;
        f32*       drag;
    };

    inline bool physics_force_integrator_init              (physics_force_integrator& i, const arena_handle a);
    inline bool physics_force_integrator_lookup_components (physics_force_integrator& i, physics_accumulator* a);
    inline void physics_force_integrator_exec              (physics_force_integrator& i, const f32 dt);
    inline void physics_force_integrator_update_components (physics_force_integrator& i);

    IFB_INTERNAL void 
    physics_integrate_forces(
        const f32 dt,
        const arena_handle a) {
        
        const u32 save   = arena_save(a);
        auto*     forces = _phys_mngr->force_accumulator;
        
        // initialize the integrator
        physics_force_integrator integrator = {0};
        if (!physics_force_integrator_init(integrator, a)) {
            arena_revert(a, save);
            return;
        }

        // load all components into the integrator
        // with forces and matching archetype
        if (!physics_force_integrator_lookup_components(integrator, forces)) {
            arena_revert(a, save);
            return;
        }

        // do the integration and update components
        physics_force_integrator_exec              (integrator, dt);             
        physics_force_integrator_update_components (integrator);            
        arena_revert(a, save);
    }

    inline bool 
    physics_force_integrator_init(
        physics_force_integrator& i,
        const arena_handle    a) {
    
        const auto& cfg  = config_instance();
        const u32   save = arena_save(a);

        i.count        = 0;
        i.id           = arena_push<entity_id>(a, cfg.entity_capacity);
        i.sparse_index = arena_push<u32>      (a, cfg.entity_capacity);
        i.pos_x        = arena_push<f32>      (a, cfg.entity_capacity);
        i.pos_y        = arena_push<f32>      (a, cfg.entity_capacity);
        i.pos_z        = arena_push<f32>      (a, cfg.entity_capacity);
        i.vel_x        = arena_push<f32>      (a, cfg.entity_capacity);
        i.vel_y        = arena_push<f32>      (a, cfg.entity_capacity);
        i.vel_z        = arena_push<f32>      (a, cfg.entity_capacity);
        i.acc_x        = arena_push<f32>      (a, cfg.entity_capacity);
        i.acc_y        = arena_push<f32>      (a, cfg.entity_capacity);
        i.acc_z        = arena_push<f32>      (a, cfg.entity_capacity);
        i.frc_x        = arena_push<f32>      (a, cfg.entity_capacity);
        i.frc_y        = arena_push<f32>      (a, cfg.entity_capacity);
        i.frc_z        = arena_push<f32>      (a, cfg.entity_capacity);
        i.tv_x         = arena_push<f32>      (a, cfg.entity_capacity);
        i.tv_y         = arena_push<f32>      (a, cfg.entity_capacity);
        i.tv_z         = arena_push<f32>      (a, cfg.entity_capacity);
        i.inv_mass     = arena_push<f32>      (a, cfg.entity_capacity);
        i.drag         = arena_push<f32>      (a, cfg.entity_capacity);
    
        const bool did_init = (
            i.id           != NULL &&
            i.sparse_index != NULL &&
            i.pos_x        != NULL &&
            i.pos_y        != NULL &&
            i.pos_z        != NULL &&
            i.vel_x        != NULL &&
            i.vel_y        != NULL &&
            i.vel_z        != NULL &&
            i.acc_x        != NULL &&
            i.acc_y        != NULL &&
            i.acc_z        != NULL &&
            i.frc_x        != NULL &&
            i.frc_y        != NULL &&
            i.frc_z        != NULL &&
            i.inv_mass     != NULL &&
            i.drag         != NULL
        );

        return(did_init);
    }

    inline bool 
    physics_force_integrator_lookup_components(
        physics_force_integrator& i,
        physics_accumulator*      a) {
        
        const component_type physics_types = (
            cmpnt_type_e_position       |
            cmpnt_type_e_velocity       |
            cmpnt_type_e_acceleration   |
            cmpnt_type_e_inv_mass       |
            cmpnt_type_e_drag           |
            cmpnt_type_e_term_velocity 
        );

        for (
            u32 force_index = 0;
                force_index < a->count;
              ++force_index
        ) {
        
            // look up the entity
            entity e;
            const bool did_lookup = entity_lookup_by_id(e, a->data.ids[force_index]);
            assert(did_lookup);

            // make sure it matches the archetype for integration
            const bool should_integrate = e.archetype.has_all(physics_types);
            if (!should_integrate) continue;

            // look up the components
            position_3d      pos;
            velocity_3d      vel;
            acceleration_3d  acc;
            inv_mass         inv;
            drag             drg;
            term_velocity_3d tv;
            cmpnt_lookup_position      (e.index_sparse, pos);            
            cmpnt_lookup_velocity      (e.index_sparse, vel);            
            cmpnt_lookup_acceleration  (e.index_sparse, acc);            
            cmpnt_lookup_inv_mass      (e.index_sparse, inv);
            cmpnt_lookup_drag          (e.index_sparse, drg);
            cmpnt_lookup_term_velocity (e.index_sparse, tv);

            // add the components to the intregrator 
            const u32 integrator_index = i.count;
            i.sparse_index [integrator_index] = e.index_sparse;
            i.pos_x        [integrator_index] = pos.x; 
            i.pos_y        [integrator_index] = pos.y; 
            i.pos_z        [integrator_index] = pos.z; 
            i.vel_x        [integrator_index] = vel.x;
            i.vel_y        [integrator_index] = vel.y;
            i.vel_z        [integrator_index] = vel.z;
            i.acc_x        [integrator_index] = acc.x;
            i.acc_y        [integrator_index] = acc.y;
            i.acc_z        [integrator_index] = acc.z;
            i.frc_x        [integrator_index] = a->data.vectors[force_index].x;
            i.frc_y        [integrator_index] = a->data.vectors[force_index].y;
            i.frc_z        [integrator_index] = a->data.vectors[force_index].z;
            i.tv_x         [integrator_index] = tv.x;
            i.tv_y         [integrator_index] = tv.y;
            i.tv_z         [integrator_index] = tv.z;
            i.inv_mass     [integrator_index] = inv.normal_val;
            i.drag         [integrator_index] = drg.normal_val;
            ++i.count;
        }

        return(i.count > 0);
    }

    inline void
    physics_force_integrator_exec(
        physics_force_integrator& i, const f32 dt) {
     
        // calculate dt constants
        const f32 dt_pow_2        = dt * dt; 
        const f32 dt_pow_2_over_2 = dt_pow_2 * 0.5f;    

        for (
            u32 integrator_index = 0;
                integrator_index < i.count;
              ++integrator_index
        ) {

            // calculate component constants 
            const f32 drag_pow_dt = powf(i.drag[integrator_index], dt); 

            // calculate acceleration 
            i.acc_x[integrator_index] = i.frc_x[integrator_index] * i.inv_mass[integrator_index];
            i.acc_y[integrator_index] = i.frc_y[integrator_index] * i.inv_mass[integrator_index];
            i.acc_z[integrator_index] = i.frc_z[integrator_index] * i.inv_mass[integrator_index];

            // position
            i.pos_x[integrator_index] += (i.vel_x[integrator_index] * dt) + (i.acc_x[integrator_index] * dt_pow_2_over_2);  
            i.pos_y[integrator_index] += (i.vel_y[integrator_index] * dt) + (i.acc_y[integrator_index] * dt_pow_2_over_2);  
            i.pos_z[integrator_index] += (i.vel_z[integrator_index] * dt) + (i.acc_z[integrator_index] * dt_pow_2_over_2);  
            
            // calculate velocity
            i.vel_x[integrator_index] = (i.vel_x[integrator_index] + i.acc_x[integrator_index] * dt) * drag_pow_dt;   
            i.vel_y[integrator_index] = (i.vel_y[integrator_index] + i.acc_y[integrator_index] * dt) * drag_pow_dt;  
            i.vel_z[integrator_index] = (i.vel_z[integrator_index] + i.acc_z[integrator_index] * dt) * drag_pow_dt;  

            const f32 tv_x = i.tv_x[integrator_index];
            const f32 tv_y = i.tv_y[integrator_index];
            const f32 tv_z = i.tv_z[integrator_index];
          
            // update terminal velocity
            const f32 vel_x_curr = i.vel_x[integrator_index];
            const f32 vel_y_curr = i.vel_y[integrator_index];
            const f32 vel_z_curr = i.vel_z[integrator_index];
            if(vel_x_curr > tv_x && tv_x > 0.0f) i.vel_x[integrator_index] = tv_x;
            if(vel_y_curr > tv_y && tv_y > 0.0f) i.vel_y[integrator_index] = tv_y;
            if(vel_z_curr > tv_z && tv_z > 0.0f) i.vel_z[integrator_index] = tv_z;
            if(vel_x_curr < -tv_x && tv_x > 0.0f) i.vel_x[integrator_index] = -tv_x;
            if(vel_y_curr < -tv_y && tv_y > 0.0f) i.vel_y[integrator_index] = -tv_y;
            if(vel_z_curr < -tv_z && tv_z > 0.0f) i.vel_z[integrator_index] = -tv_z;
        }
    }

    inline void
    physics_force_integrator_update_components(
        physics_force_integrator& i) {
        
        position_3d     pos;
        velocity_3d     vel;
        acceleration_3d acc;
        inv_mass        inv;
        drag            drg;

        for (
            u32 index = 0;
            index < i.count;
            ++index
        ) {

            pos.x          = i.pos_x    [index];
            pos.y          = i.pos_y    [index];
            pos.z          = i.pos_z    [index];
            vel.x          = i.vel_x    [index];
            vel.y          = i.vel_y    [index];
            vel.z          = i.vel_z    [index];
            acc.x          = i.acc_x    [index];
            acc.y          = i.acc_y    [index];
            acc.z          = i.acc_z    [index];
        
            cmpnt_update_position      (i.sparse_index[index], pos);
            cmpnt_update_velocity      (i.sparse_index[index], vel);     
            cmpnt_update_acceleration  (i.sparse_index[index], acc);   
        } 
    }
}; 
