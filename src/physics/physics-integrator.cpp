#pragma once

#include <math.h>

#include "component.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "entity.hpp"
#include "physics-force-accumulator.cpp"
#include "physics-internal.hpp"

namespace ifb {

    struct phys_frc_intgrtr {
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

    inline bool phys_frc_intgrtr_lookup_components (phys_frc_intgrtr* i, const phys_frc_accmltr* a);
    inline void phys_frc_intgrtr_exec              (phys_frc_intgrtr* i, const f32 dt);
    inline void phys_frc_intgrtr_update_components (phys_frc_intgrtr* i);

    IFB_INTERNAL void 
    phys_frc_intgrtr_run(
        phys_frc_intgrtr*        integrator,
        const phys_frc_accmltr* accum,
        const f32                        dt) {

        // load all components into the integrator
        // with forces and matching archetype
        if (!phys_frc_intgrtr_lookup_components(integrator, accum)) {
            return;
        }

        // do the integration and update components
        phys_frc_intgrtr_exec              (integrator, dt);             
        phys_frc_intgrtr_update_components (integrator);            
   
        // reset
        integrator->count = 0;
    }

    IFB_INTERNAL phys_frc_intgrtr* 
    phys_frc_intgrtr_create(
        void) {
    
        const auto& cfg  = config_instance();

        // calculate size
        const u32 size_struct      = sizeof(phys_frc_intgrtr);
        const u32 size_array_ids   = cfg.entity_capacity * sizeof(entity_id);
        const u32 size_array_props = cfg.entity_capacity * sizeof(u32);  
        const u32 size_total       = size_struct + size_array_ids + (size_array_props * 18); 
       
        // allocate memory
        addr mem_addr = (addr)phys_mngr_res_alloc(size_total);
        assert(mem_addr != 0);

        // cast pointers and initialize
        auto integrator = (phys_frc_intgrtr*)mem_addr;

        integrator->count        = 0;
        integrator->id           = (entity_id*)phys_mngr_res_alloc(size_array_props);
        integrator->sparse_index =       (u32*)phys_mngr_res_alloc(size_array_props);
        integrator->pos_x        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->pos_y        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->pos_z        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->vel_x        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->vel_y        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->vel_z        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->acc_x        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->acc_y        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->acc_z        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->frc_x        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->frc_y        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->frc_z        =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->tv_x         =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->tv_y         =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->tv_z         =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->inv_mass     =       (f32*)phys_mngr_res_alloc(size_array_props);
        integrator->drag         =       (f32*)phys_mngr_res_alloc(size_array_props);
    
        assert(integrator->id           != NULL);
        assert(integrator->sparse_index != NULL);
        assert(integrator->pos_x        != NULL);
        assert(integrator->pos_y        != NULL);
        assert(integrator->pos_z        != NULL);
        assert(integrator->vel_x        != NULL);
        assert(integrator->vel_y        != NULL);
        assert(integrator->vel_z        != NULL);
        assert(integrator->acc_x        != NULL);
        assert(integrator->acc_y        != NULL);
        assert(integrator->acc_z        != NULL);
        assert(integrator->frc_x        != NULL);
        assert(integrator->frc_y        != NULL);
        assert(integrator->frc_z        != NULL);
        assert(integrator->inv_mass     != NULL);
        assert(integrator->drag         != NULL);

        return(integrator);
    }

    inline bool 
    phys_frc_intgrtr_lookup_components(
        phys_frc_intgrtr*        i,
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
            cmpnt_position      pos;
            cmpnt_velocity      vel;
            cmpnt_acceleration  acc;
            cmpnt_inv_mass      inv;
            cmpnt_drag             drg;
            cmpnt_term_velocity tv;
            cmpnt_lookup_position      (e.index_sparse, pos);            
            cmpnt_lookup_velocity      (e.index_sparse, vel);            
            cmpnt_lookup_acceleration  (e.index_sparse, acc);            
            cmpnt_lookup_inv_mass      (e.index_sparse, inv);
            cmpnt_lookup_drag          (e.index_sparse, drg);
            cmpnt_lookup_term_velocity (e.index_sparse, tv);

            // add the components to the intregrator 
            const u32 integrator_index = i->count;
            i->sparse_index [integrator_index] = e.index_sparse;
            i->pos_x        [integrator_index] = pos.x; 
            i->pos_y        [integrator_index] = pos.y; 
            i->pos_z        [integrator_index] = pos.z; 
            i->vel_x        [integrator_index] = vel.x;
            i->vel_y        [integrator_index] = vel.y;
            i->vel_z        [integrator_index] = vel.z;
            i->acc_x        [integrator_index] = acc.x;
            i->acc_y        [integrator_index] = acc.y;
            i->acc_z        [integrator_index] = acc.z;
            i->frc_x        [integrator_index] = a->data.forces[force_index].x;
            i->frc_y        [integrator_index] = a->data.forces[force_index].y;
            i->frc_z        [integrator_index] = a->data.forces[force_index].z;
            i->tv_x         [integrator_index] = tv.x;
            i->tv_y         [integrator_index] = tv.y;
            i->tv_z         [integrator_index] = tv.z;
            i->inv_mass     [integrator_index] = inv.normal_val;
            i->drag         [integrator_index] = drg.normal_val;
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
            i->acc_x[integrator_index] = i->frc_x[integrator_index] * i->inv_mass[integrator_index];
            i->acc_y[integrator_index] = i->frc_y[integrator_index] * i->inv_mass[integrator_index];
            i->acc_z[integrator_index] = i->frc_z[integrator_index] * i->inv_mass[integrator_index];

            // position
            i->pos_x[integrator_index] += (i->vel_x[integrator_index] * dt) + (i->acc_x[integrator_index] * dt_pow_2_over_2);  
            i->pos_y[integrator_index] += (i->vel_y[integrator_index] * dt) + (i->acc_y[integrator_index] * dt_pow_2_over_2);  
            i->pos_z[integrator_index] += (i->vel_z[integrator_index] * dt) + (i->acc_z[integrator_index] * dt_pow_2_over_2);  
            
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
    phys_frc_intgrtr_update_components(
        phys_frc_intgrtr* i) {
        
        cmpnt_position     pos;
        cmpnt_velocity     vel;
        cmpnt_acceleration acc;
        cmpnt_inv_mass     inv;
        cmpnt_drag         drg;

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
}; 
