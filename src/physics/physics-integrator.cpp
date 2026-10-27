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
        vec3*                half_extent;
        u32*                 index_by_sparse; // entity sparse index -> integrator index
        u32                  capacity_sparse;
    };

    // one side of a collision pair
    // index is INVALID_INDEX when the entity isn't being integrated this step,
    // it doesn't move so it's treated as immovable
    struct phys_frc_intgrtr_body {
        u32  index;
        vec3 pos;
        vec3 vel;
        vec3 half_extent;
        f32  inv_mass;
    };

    inline bool phys_frc_intgrtr_lookup_global_components (phys_frc_intgrtr* i, const phys_frc_accmltr* a);
    inline void phys_frc_intgrtr_exec                     (phys_frc_intgrtr* i, const f32 dt);
    inline void phys_frc_intgrtr_constrain_to_collisions  (phys_frc_intgrtr* i, const phys_cllsn_list* list);
    inline void phys_frc_intgrtr_constrain_to_maps        (phys_frc_intgrtr* i);
    inline void phys_frc_intgrtr_update_global_components (phys_frc_intgrtr* i);
    inline void phys_frc_intgrtr_reset                    (phys_frc_intgrtr* i);

    IFB_INTERNAL void
    phys_frc_intgrtr_run(
        phys_frc_intgrtr*       integrator,
        const phys_frc_accmltr* accum,
        const phys_cllsn_list*  cllsn_list,
        const f32               dt) {

        // load all components into the integrator
        // with forces and matching archetype
        // map entities integrate in world space like everything else,
        // their map coordinates are derived from the position
        if (!phys_frc_intgrtr_lookup_global_components(integrator, accum)) {
            return;
        }

        // do the integration and update components
        // collisions are resolved before the map so the map has the final say
        phys_frc_intgrtr_exec                     (integrator, dt);
        phys_frc_intgrtr_constrain_to_collisions  (integrator, cllsn_list);
        phys_frc_intgrtr_constrain_to_maps        (integrator);
        phys_frc_intgrtr_update_global_components (integrator);

        // reset
        phys_frc_intgrtr_reset(integrator);
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
        integrator->half_extent   =               (vec3*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(vec3));

        // the entity manager is started first, so the sparse capacity is known
        integrator->capacity_sparse = entity_mngr_capacity_sparse();
        integrator->index_by_sparse = (u32*)phys_mngr_res_alloc(integrator->capacity_sparse * sizeof(u32));
        assert(integrator->capacity_sparse != 0);
        assert(integrator->index_by_sparse != NULL);
        for (
            u32 sparse_index = 0;
                sparse_index < integrator->capacity_sparse;
              ++sparse_index) {

            integrator->index_by_sparse[sparse_index] = INVALID_INDEX;
        }

        assert(integrator->force         != NULL);
        assert(integrator->position      != NULL);
        assert(integrator->velocity      != NULL);
        assert(integrator->acceleration  != NULL);
        assert(integrator->term_velocity != NULL);
        assert(integrator->inv_mass      != NULL);
        assert(integrator->drag          != NULL);
        assert(integrator->map           != NULL);
        assert(integrator->map_inset     != NULL);
        assert(integrator->half_extent   != NULL);

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
            i->map         [integrator_index] = INVALID_HANDLE;
            i->map_inset   [integrator_index] = 0.0f;
            i->half_extent [integrator_index] = {};
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
                i->map_inset   [integrator_index] = quad.width * 0.5f;
                i->half_extent [integrator_index] = phys_aabb_half_extent_quad(quad);
            }

            i->force        [integrator_index] = a->data.forces[force_index];
            i->sparse_index [integrator_index] = e.index_sparse;

            assert(e.index_sparse < i->capacity_sparse);
            i->index_by_sparse[e.index_sparse] = integrator_index;

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

    inline bool
    phys_frc_intgrtr_get_body(
        const phys_frc_intgrtr* i,
        const entity_id         id,
        phys_frc_intgrtr_body&  body) {

        entity e;
        if (!entity_lookup_by_id(e, id)) return(false);
        if (e.index_sparse >= i->capacity_sparse) return(false);

        // integrated this step, use the new position and velocity
        const u32 index = i->index_by_sparse[e.index_sparse];
        if (index != INVALID_INDEX) {
            body.index       = index;
            body.pos         = i->position    [index];
            body.vel         = i->velocity    [index];
            body.half_extent = i->half_extent [index];
            body.inv_mass    = i->inv_mass    [index].normal_val;
            return(true);
        }

        // not integrated, so it stays where it is
        if (!e.archetype.has_all(cmpnt_type_e_position | cmpnt_type_e_quad)) return(false);

        cmpnt_quad quad;
        cmpnt_lookup_position (e.index_sparse, body.pos);
        cmpnt_lookup_quad     (e.index_sparse, quad);
        body.index       = INVALID_INDEX;
        body.vel         = {};
        body.half_extent = phys_aabb_half_extent_quad(quad);
        body.inv_mass    = 0.0f;
        return(true);
    }

    inline void
    phys_frc_intgrtr_constrain_to_collisions(
        phys_frc_intgrtr*      i,
        const phys_cllsn_list* list) {

        if (list == NULL) return;

        const u32 pair_count = phys_cllsn_list_get_count(list);
        for (
            u32 pair_index = 0;
                pair_index < pair_count;
              ++pair_index) {

            const phys_cllsn_pair pair = phys_cllsn_list_get_pair(list, pair_index);

            phys_frc_intgrtr_body a;
            phys_frc_intgrtr_body b;
            if (!phys_frc_intgrtr_get_body(i, pair.a, a)) continue;
            if (!phys_frc_intgrtr_get_body(i, pair.b, b)) continue;

            // nothing can move
            const f32 inv_mass_total = a.inv_mass + b.inv_mass;
            if (inv_mass_total <= 0.0f) continue;

            // the pair came from the swept broad phase,
            // check that they actually overlap after integration
            // and find the axis with the least penetration
            u32 axis        = 0;
            f32 overlap_min = 0.0f;
            bool is_overlap = true;
            for (
                u32 axis_index = 0;
                    axis_index < 3;
                  ++axis_index) {

                const f32 delta   = b.pos.v[axis_index] - a.pos.v[axis_index];
                const f32 overlap = (a.half_extent.v[axis_index] + b.half_extent.v[axis_index]) - fabsf(delta);
                if (overlap <= 0.0f) {
                    is_overlap = false;
                    break;
                }
                if (axis_index == 0 || overlap < overlap_min) {
                    axis        = axis_index;
                    overlap_min = overlap;
                }
            }
            if (!is_overlap) continue;

            // the normal points from a to b
            const f32 normal = (b.pos.v[axis] >= a.pos.v[axis]) ? 1.0f : -1.0f;

            // push them apart, the lighter one moves more
            const f32 push_a = overlap_min * (a.inv_mass / inv_mass_total);
            const f32 push_b = overlap_min * (b.inv_mass / inv_mass_total);
            a.pos.v[axis] -= normal * push_a;
            b.pos.v[axis] += normal * push_b;

            // if they're moving toward each other, cancel the closing velocity
            // so they stop at the contact instead of pushing back in next step
            const f32 vel_closing = (b.vel.v[axis] - a.vel.v[axis]) * normal;
            if (vel_closing < 0.0f) {
                const f32 impulse = -vel_closing / inv_mass_total;
                a.vel.v[axis] -= normal * impulse * a.inv_mass;
                b.vel.v[axis] += normal * impulse * b.inv_mass;
            }

            // write back to the integrator, immovable bodies are left alone
            if (a.index != INVALID_INDEX) {
                i->position[a.index] = a.pos;
                i->velocity[a.index] = a.vel;
            }
            if (b.index != INVALID_INDEX) {
                i->position[b.index] = b.pos;
                i->velocity[b.index] = b.vel;
            }
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

    inline void
    phys_frc_intgrtr_reset(
        phys_frc_intgrtr* i) {

        for (
            u32 index = 0;
            index < i->count;
            ++index
        ) {

            i->index_by_sparse[i->sparse_index[index]] = INVALID_INDEX;
        }

        i->count = 0;
    }
}; 
