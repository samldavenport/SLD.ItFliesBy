#pragma once

#include <math.h>

#include "component.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "entity.hpp"
#include "map.cpp"
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
        cmpnt_map_coords*    map_coords;
    };

    inline bool phys_frc_intgrtr_lookup_global_components (phys_frc_intgrtr* i, const phys_frc_accmltr* a);
    inline bool phys_frc_intgrtr_lookup_map_components    (phys_frc_intgrtr* i, const phys_frc_accmltr* a);
    inline void phys_frc_intgrtr_exec                     (phys_frc_intgrtr* i, const f32 dt);
    inline void phys_frc_intgrtr_update_global_components (phys_frc_intgrtr* i);
    inline void phys_frc_intgrtr_update_map_components    (phys_frc_intgrtr* i);

    IFB_INTERNAL void 
    phys_frc_intgrtr_run(
        phys_frc_intgrtr*       integrator,
        const phys_frc_accmltr* accum,
        const f32               dt) {

        /////////////////////
        // GLOBAL FORCES
        ////////////////////

        // load all components into the integrator
        // with forces and matching archetype
        if (!phys_frc_intgrtr_lookup_global_components(integrator, accum)) {
            return;
        }

        // do the integration and update components
        phys_frc_intgrtr_exec                     (integrator, dt);             
        phys_frc_intgrtr_update_global_components (integrator);            
  
        // reset
        integrator->count = 0;
        
        /////////////////////
        // MAP FORCES
        ////////////////////

        // load all physics components on the map
        phys_frc_intgrtr_lookup_map_components(integrator, accum);

        // do the integration and update components
        phys_frc_intgrtr_exec(integrator, dt);
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
        integrator->force         =               (vec3*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(vec3)); 
        integrator->position      =     (cmpnt_position*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_position)); 
        integrator->velocity      =     (cmpnt_velocity*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_velocity)); 
        integrator->acceleration  = (cmpnt_acceleration*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_acceleration)); 
        integrator->term_velocity =(cmpnt_term_velocity*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_term_velocity)); 
        integrator->inv_mass      =     (cmpnt_inv_mass*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_inv_mass)); 
        integrator->drag          =         (cmpnt_drag*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_drag)); 
        integrator->map_coords    =   (cmpnt_map_coords*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(cmpnt_map_coords)); 

        assert(integrator->count         != NULL);
        assert(integrator->force         != NULL);
        assert(integrator->position      != NULL);
        assert(integrator->velocity      != NULL);
        assert(integrator->acceleration  != NULL);
        assert(integrator->term_velocity != NULL);
        assert(integrator->inv_mass      != NULL);
        assert(integrator->drag          != NULL);
        assert(integrator->map_coords    != NULL);

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
            // for now, we are excluding map entities
            const bool should_integrate = 
                e.archetype.has_all(physics_types) &&
                e.archetype.has_none(cmpnt_type_e_map_coords);
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
            cmpnt_lookup_map_coords    (e.index_sparse, i->map_coords    [integrator_index]);
            
            ++i->count;
        }

        return(i->count > 0);
    }

    inline bool 
    phys_frc_intgrtr_lookup_map_components(
        phys_frc_intgrtr*       i,
        const phys_frc_accmltr* a) {

        const component_type physics_types = (
            cmpnt_type_e_position      |
            cmpnt_type_e_velocity      |
            cmpnt_type_e_acceleration  |
            cmpnt_type_e_inv_mass      |
            cmpnt_type_e_drag          |
            cmpnt_type_e_term_velocity | 
            cmpnt_type_e_map_coords 
        );

        for (
            u32 force_index = 0;
                force_index < a->count;
              ++force_index) {
        
            // look up the entity
            entity e;
            const bool did_lookup = entity_lookup_by_id(e, a->data.ids[force_index]);
            assert(did_lookup);

            // make sure it matches the archetype for integration
            // for now, we are excluding map entities
            const bool should_integrate = e.archetype.has_all(physics_types);
            if (!should_integrate) continue;


            // add the components to the intregrator 
            const u32 integrator_index = i->count;
            // look up the components
            cmpnt_lookup_velocity      (e.index_sparse, i->velocity     [integrator_index]);            
            cmpnt_lookup_acceleration  (e.index_sparse, i->acceleration [integrator_index]);            
            cmpnt_lookup_inv_mass      (e.index_sparse, i->inv_mass     [integrator_index]);
            cmpnt_lookup_drag          (e.index_sparse, i->drag         [integrator_index]);
            cmpnt_lookup_term_velocity (e.index_sparse, i->term_velocity[integrator_index]);
            cmpnt_lookup_map_coords    (e.index_sparse, i->map_coords   [integrator_index]);

            // calculate the position from the map coordinates
            map_get_pos_from_coords(i->map_coords[integrator_index], i->position[integrator_index]);
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
            const f32 drag_pow_dt = powf(i->drag[integrator_index], dt); 

            // calculate acceleration
            
            i->acceleration[integrator_index].x = i->force[integrator_index].x * i->inv_mass[integrator_index];
            i->acceleration[integrator_index].y = i->force[integrator_index].y * i->inv_mass[integrator_index];
            i->acceleration[integrator_index].z = i->force[integrator_index].z * i->inv_mass[integrator_index];

            // position
            i->pos_x[integrator_index].x += (i->velocity[integrator_index].x * dt) + (i->acceleration[integrator_index].x * dt_pow_2_over_2);  
            i->pos_y[integrator_index].y += (i->velocity[integrator_index].y * dt) + (i->acceleration[integrator_index].y * dt_pow_2_over_2);  
            i->pos_z[integrator_index].z += (i->velocity[integrator_index].z * dt) + (i->acceleration[integrator_index].z * dt_pow_2_over_2);  
            
            // calculate velocity
            i->vel_x[integrator_index] = (i->vel_x[integrator_index] + i->acc_x[integrator_index] * dt) * drag_pow_dt;   
            i->vel_y[integrator_index] = (i->vel_y[integrator_index] + i->acc_y[integrator_index] * dt) * drag_pow_dt;  
            i->vel_z[integrator_index] = (i->vel_z[integrator_index] + i->acc_z[integrator_index] * dt) * drag_pow_dt;  

            const f32 tv_x = i->tv_x[integrator_index];
            const f32 tv_y = i->tv_y[integrator_index];
            const f32 tv_z = i->tv_z[integrator_index];
          
            // update terminal velocity
            const f32 vel_x_curr = i->vel_x[integrator_index];
            const f32 vel_y_curr = i->vel_y[integrator_index];
            const f32 vel_z_curr = i->vel_z[integrator_index];
            if(vel_x_curr > tv_x  && tv_x > 0.0f) i->vel_x[integrator_index] =  tv_x;
            if(vel_y_curr > tv_y  && tv_y > 0.0f) i->vel_y[integrator_index] =  tv_y;
            if(vel_z_curr > tv_z  && tv_z > 0.0f) i->vel_z[integrator_index] =  tv_z;
            if(vel_x_curr < -tv_x && tv_x > 0.0f) i->vel_x[integrator_index] = -tv_x;
            if(vel_y_curr < -tv_y && tv_y > 0.0f) i->vel_y[integrator_index] = -tv_y;
            if(vel_z_curr < -tv_z && tv_z > 0.0f) i->vel_z[integrator_index] = -tv_z;
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

            pos.x          = i->pos_x    [index];
            pos.y          = i->pos_y    [index];
            pos.z          = i->pos_z    [index];
            vel.x          = i->vel_x    [index];
            vel.y          = i->vel_y    [index];
            vel.z          = i->vel_z    [index];
            acc.x          = i->acc_x    [index];
            acc.y          = i->acc_y    [index];
            acc.z          = i->acc_z    [index];
        
            cmpnt_update_position      (i->sparse_index[index], pos);
            cmpnt_update_velocity      (i->sparse_index[index], vel);     
            cmpnt_update_acceleration  (i->sparse_index[index], acc);   
        } 
    }
    
    inline void
    phys_frc_intgrtr_update_map_components(
        phys_frc_intgrtr* i) {
        
        cmpnt_position     pos;
        cmpnt_velocity     vel;
        cmpnt_acceleration acc;
        cmpnt_map_coords   map;
        
        for (
            u32 index = 0;
            index < i->count;
            ++index
        ) {

            pos.x          = i->pos_x [index];
            pos.y          = i->pos_y [index];
            pos.z          = i->pos_z [index];
            vel.x          = i->vel_x [index];
            vel.y          = i->vel_y [index];
            vel.z          = i->vel_z [index];
            acc.x          = i->acc_x [index];
            acc.y          = i->acc_y [index];
            acc.z          = i->acc_z [index];
            map.h_map      = i->h_map [index]; 

            const u32 sparse_index = i->sparse_index[index];

            //TODO(SLD): we need to detect for map boundaries

            map_get_coords_from_pos    (pos, map);
            cmpnt_update_position      (sparse_index, pos);
            cmpnt_update_velocity      (sparse_index, vel);     
            cmpnt_update_acceleration  (sparse_index, acc);   
            cmpnt_update_map_coords    (sparse_index, map);
        } 
    }
}; 
