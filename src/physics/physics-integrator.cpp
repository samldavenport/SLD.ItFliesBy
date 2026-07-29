#pragma once

#include "component.hpp"
#include "ifb-component.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "physics.hpp"
#include "entity.hpp"

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
        f32*       inv_mass;
        f32*       drag;
    };

    IFB_INTERNAL bool 
    physics_integrate_forces(
        const u32 dt_ms,
        arena*    a) {

        assert(a != NULL);     
    
        const auto& cfg  = config_instance();
        const u32   save = arena_save(a);

        // allocate force integrator 
        physics_force_integrator integrator;
        integrator.count        = 0;
        integrator.id           = arena_push<entity_id>(a, cfg.entity_capacity);
        integrator.sparse_index = arena_push<u32>      (a, cfg.entity_capacity);
        integrator.pos_x        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.pos_y        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.pos_z        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.vel_x        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.vel_y        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.vel_z        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.acc_x        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.acc_y        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.acc_z        = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.inv_mass     = arena_push<f32>      (a, cfg.entity_capacity);
        integrator.drag         = arena_push<f32>      (a, cfg.entity_capacity);
    
        const bool can_proceed = (
            integrator.id           != NULL &&
            integrator.sparse_index != NULL &&
            integrator.pos_x        != NULL &&
            integrator.pos_y        != NULL &&
            integrator.pos_z        != NULL &&
            integrator.vel_x        != NULL &&
            integrator.vel_y        != NULL &&
            integrator.vel_z        != NULL &&
            integrator.acc_x        != NULL &&
            integrator.acc_y        != NULL &&
            integrator.acc_z        != NULL &&
            integrator.inv_mass     != NULL &&
            integrator.drag         != NULL
        );

        if (!can_proceed) {
            arena_revert(a, save);
            return(false);
        }

        const entity_archetype archetype = (
            cmpnt_type_e_position     |
            cmpnt_type_e_velocity     |
            cmpnt_type_e_acceleration |
            cmpnt_type_e_inv_mass     |
            cmpnt_type_e_drag
        );

        for (
            u32 dense_index = 0;
                dense_index < _entity_mngr->count;
              ++dense_index
        ) {
            // check if the archetype of this entity
            // has all the necessary components
            const bool should_integrate = (
                    _entity_mngr->data.dense.archetype[dense_index] & 
                   archetype 
            ) == archetype;

            if (!should_integrate) {
                continue;
            }

            const u32   sparse_index     = _entity_mngr->data.dense.sparse_index[dense_index];
            const u32   integrator_index = integrator.count; 
            const auto& sparse_pos       = _cmpnt_mngr->tables.position;
            const auto& sparse_vel       = _cmpnt_mngr->tables.velocity;
            const auto& sparse_acc       = _cmpnt_mngr->tables.acceleration;
            const auto& sparse_im        = _cmpnt_mngr->tables.inv_mass;
            const auto& sparse_drag      = _cmpnt_mngr->tables.drag;

            // if so, add the components to the integrator
            integrator.id           [integrator_index] = _entity_mngr->data.dense.id           [dense_index];
            integrator.sparse_index [integrator_index] = _entity_mngr->data.dense.sparse_index [dense_index];
            integrator.pos_x        [integrator_index] = sparse_pos->x                         [sparse_index];
            integrator.pos_y        [integrator_index] = sparse_pos->y                         [sparse_index]; 
            integrator.pos_z        [integrator_index] = sparse_pos->z                         [sparse_index];
            integrator.vel_x        [integrator_index] = sparse_vel->x                         [sparse_index];
            integrator.vel_y        [integrator_index] = sparse_vel->y                         [sparse_index];
            integrator.vel_z        [integrator_index] = sparse_vel->z                         [sparse_index];
            integrator.acc_x        [integrator_index] = sparse_acc->x                         [sparse_index];
            integrator.acc_y        [integrator_index] = sparse_acc->y                         [sparse_index];
            integrator.acc_z        [integrator_index] = sparse_acc->z                         [sparse_index];
            integrator.inv_mass     [integrator_index] = sparse_im->normal_val                 [sparse_index];
            integrator.drag         [integrator_index] = sparse_drag->normal_val               [sparse_index];
       
            // update the count
            ++integrator.count;
        }

        for (
            u32 i= 0;
                i< integrator.count;
              ++i
        ) {
            
        }

        return(true);
    }
};
