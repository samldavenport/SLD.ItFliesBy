#pragma once

#include <math.h>

#include "component.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "entity.hpp"
#include "map.hpp"
#include "physics-force-accumulator.cpp"
#include "physics-internal.hpp"
#include "sld-math-types.hpp"

namespace ifb {

    struct phys_frc_intgrtr {
        u32                  count;
        u32*                 sparse_index;
        vec3*                force; 
        cmpnt_position*      position;
        cmpnt_velocity*      velocity;
        cmpnt_acceleration*  acceleration;
        cmpnt_term_velocity* term_velocity;
        cmpnt_inv_mass*      inv_mass;
        cmpnt_drag*          drag;
        hnd_map*             map;
        f32*                 map_inset;
    };

    inline bool phys_frc_intgrtr_lookup_global_components (phys_frc_intgrtr* i, const phys_frc_accmltr* a);
    inline void phys_frc_intgrtr_exec                     (phys_frc_intgrtr* i, const f32 dt);
    inline void phys_frc_intgrtr_constrain_to_maps        (phys_frc_intgrtr* i);
    inline void phys_frc_intgrtr_update_global_components (phys_frc_intgrtr* i);

    IFB_INTERNAL void
    phys_frc_intgrtr_run(
        phys_frc_intgrtr*       integrator,
        const phys_frc_accmltr* accum,
        const f32               dt) {

        // load all components into the integrator
        // with forces and matching archetype
        // map entities integrate in world space like everything else,
        // their map coordinates are derived from the position
        if (!phys_frc_intgrtr_lookup_global_components(integrator, accum)) {
            return;
        }

        // do the integration and update components
        phys_frc_intgrtr_exec                     (integrator, dt);
        phys_frc_intgrtr_constrain_to_maps        (integrator);
        phys_frc_intgrtr_update_global_components (integrator);

        // reset
        integrator->count = 0;
    }

    IFB_INTERNAL phys_frc_intgrtr* 
    phys_frc_intgrtr_create(
        void) {
    
        const auto& cfg  = config_instance();

        // calculate size
        const u32 size_struct      = sizeof(phys_frc_intgrtr);
        const u32 size_array_maps  = cfg.entity_capacity * sizeof(hnd_map);
        const u32 size_array_props = cfg.entity_capacity * sizeof(u32);  
        const u32 size_total       = size_struct + size_array_maps + (size_array_props * 18); 
       
        // allocate memory
        addr mem_addr = (addr)phys_mngr_res_alloc(size_total);
        assert(mem_addr != 0);

        // cast pointers and initialize
        auto integrator = (phys_frc_intgrtr*)mem_addr;
        integrator->count         = 0;
        integrator->sparse_index  =                (u32*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(u32)); 
        integrator->force         =               (vec3*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(vec3)); 
        integrator->position      =     (cmpnt_position*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_position)); 
        integrator->velocity      =     (cmpnt_velocity*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_velocity)); 
        integrator->acceleration  = (cmpnt_acceleration*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_acceleration)); 
        integrator->term_velocity =(cmpnt_term_velocity*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_term_velocity)); 
        integrator->inv_mass      =     (cmpnt_inv_mass*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_inv_mass)); 
        integrator->drag          =         (cmpnt_drag*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_drag));
        integrator->map           =            (hnd_map*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(hnd_map));
        integrator->map_inset     =                (f32*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(f32));

        assert(integrator->force         != NULL);
        assert(integrator->position      != NULL);
        assert(integrator->velocity      != NULL);
        assert(integrator->acceleration  != NULL);
        assert(integrator->term_velocity != NULL);
        assert(integrator->inv_mass      != NULL);
        assert(integrator->drag          != NULL);
        assert(integrator->map           != NULL);
        assert(integrator->map_inset     != NULL);

        return(integrator);
    }

    inline bool 
    phys_frc_intgrtr_lookup_global_components(
        phys_frc_intgrtr*       i,
        const phys_frc_accmltr* a) {
        
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
            // add the components to the intregrator    
            const u32 integrator_index = i->count; 
            cmpnt_lookup_position      (e.index_sparse, i->position      [integrator_index]);         
            cmpnt_lookup_velocity      (e.index_sparse, i->velocity      [integrator_index]);       
            cmpnt_lookup_acceleration  (e.index_sparse, i->acceleration  [integrator_index]);            
            cmpnt_lookup_inv_mass      (e.index_sparse, i->inv_mass      [integrator_index]);
            cmpnt_lookup_drag          (e.index_sparse, i->drag          [integrator_index]);
            cmpnt_lookup_term_velocity (e.index_sparse, i->term_velocity [integrator_index]);

            // entities that belong to a map stay inside of it
            i->map       [integrator_index] = INVALID_HANDLE;
            i->map_inset [integrator_index] = 0.0f;
            if (e.archetype.has_all(cmpnt_type_e_map_coords)) {
                cmpnt_map_coords map_coords;
                cmpnt_lookup_map_coords(e.index_sparse, map_coords);
                i->map[integrator_index] = map_coords.h_map;
            }
            // the position is the center of the quad,
            // so keep half of it away from the edge
            if (e.archetype.has_all(cmpnt_type_e_quad)) {
                cmpnt_quad quad;
                cmpnt_lookup_quad(e.index_sparse, quad);
                i->map_inset[integrator_index] = quad.width * 0.5f;
            }

            i->force        [integrator_index] = a->data.forces[force_index];
            i->sparse_index [integrator_index] = e.index_sparse;

            ++i->count;
        }

        return(i->count > 0);
    }

    inline void
    phys_frc_intgrtr_exec(
        phys_frc_intgrtr* i, const f32 dt) {
     
        // calculate dt constants
        const f32 dt_pow_2        = dt * dt; 
        const f32 dt_pow_2_over_2 = dt_pow_2 * 0.5f;    

        for (
            u32 integrator_index = 0;
                integrator_index < i->count;
              ++integrator_index
        ) {

            // calculate component constants 
            const f32 drag_pow_dt = powf(i->drag[integrator_index].normal_val, dt); 

            // calculate acceleration
            i->acceleration[integrator_index].x = i->force[integrator_index].x * i->inv_mass[integrator_index].normal_val;
            i->acceleration[integrator_index].y = i->force[integrator_index].y * i->inv_mass[integrator_index].normal_val;
            i->acceleration[integrator_index].z = i->force[integrator_index].z * i->inv_mass[integrator_index].normal_val;

            // position
            i->position[integrator_index].x += (i->velocity[integrator_index].x * dt) + (i->acceleration[integrator_index].x * dt_pow_2_over_2);  
            i->position[integrator_index].y += (i->velocity[integrator_index].y * dt) + (i->acceleration[integrator_index].y * dt_pow_2_over_2);  
            i->position[integrator_index].z += (i->velocity[integrator_index].z * dt) + (i->acceleration[integrator_index].z * dt_pow_2_over_2);  
            
            // calculate velocity
            i->velocity[integrator_index].x = (i->velocity[integrator_index].x + i->acceleration[integrator_index].x * dt) * drag_pow_dt;   
            i->velocity[integrator_index].y = (i->velocity[integrator_index].y + i->acceleration[integrator_index].y * dt) * drag_pow_dt;  
            i->velocity[integrator_index].z = (i->velocity[integrator_index].z + i->acceleration[integrator_index].z * dt) * drag_pow_dt;  

            const f32 tv_x = i->term_velocity[integrator_index].x;
            const f32 tv_y = i->term_velocity[integrator_index].y;
            const f32 tv_z = i->term_velocity[integrator_index].z;
          
            // update terminal velocity
            const f32 vel_x_curr = i->velocity[integrator_index].x;
            const f32 vel_y_curr = i->velocity[integrator_index].y;
            const f32 vel_z_curr = i->velocity[integrator_index].z;
            if(vel_x_curr > tv_x  && tv_x > 0.0f) i->velocity[integrator_index].x =  tv_x;
            if(vel_y_curr > tv_y  && tv_y > 0.0f) i->velocity[integrator_index].y =  tv_y;
            if(vel_z_curr > tv_z  && tv_z > 0.0f) i->velocity[integrator_index].z =  tv_z;
            if(vel_x_curr < -tv_x && tv_x > 0.0f) i->velocity[integrator_index].x = -tv_x;
            if(vel_y_curr < -tv_y && tv_y > 0.0f) i->velocity[integrator_index].y = -tv_y;
            if(vel_z_curr < -tv_z && tv_z > 0.0f) i->velocity[integrator_index].z = -tv_z;
        }
    }

    inline void
    phys_frc_intgrtr_constrain_to_maps(
        phys_frc_intgrtr* i) {

        for (
            u32 index = 0;
            index < i->count;
            ++index
        ) {

            if (i->map[index] == INVALID_HANDLE) continue;

            (void)map_constrain_movement(
                i->map       [index],
                i->map_inset [index],
                i->position  [index],
                i->velocity  [index]
            );
        }
    }

    inline void
    phys_frc_intgrtr_update_global_components(
        phys_frc_intgrtr* i) {
        
        cmpnt_position     pos;
        cmpnt_velocity     vel;
        cmpnt_acceleration acc;

        for (
            u32 index = 0;
            index < i->count;
            ++index
        ) {
        
            cmpnt_update_position      (i->sparse_index[index], i->position[index]);
            cmpnt_update_velocity      (i->sparse_index[index], i->velocity[index]);     
            cmpnt_update_acceleration  (i->sparse_index[index], i->acceleration[index]);   
        } 
    }
}; 
