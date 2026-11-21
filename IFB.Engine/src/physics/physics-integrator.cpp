#pragma once

#include <math.h>
#include "eng-entity.hpp"
#include "eng-map.hpp"

#include "eng-component.hpp"
#include "ifb-config.hpp"
#include "ifb.hpp"
#include "physics-force-accumulator.cpp"
#include "eng-physics.hpp"
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
        f32*                 map_offset_x; // the collision box's center is this far along x from the position
    };

    // the last few drag values seen in one integration, and drag to the power of dt for each
    // the drags are compared by their bits, so -0 and +0 aren't mixed up, and a NaN is a miss
    static constexpr u32 PHYS_DRAG_CACHE_SIZE = 4;
    struct phys_frc_intgrtr_drag_cache {
        u32 drag_bits   [PHYS_DRAG_CACHE_SIZE];
        f32 drag_pow_dt [PHYS_DRAG_CACHE_SIZE];
        u32 count;
        u32 next;     // the entry replaced on a miss once it's full
    };

    inline f32  phys_frc_intgrtr_drag_pow_dt              (phys_frc_intgrtr_drag_cache& cache, const f32 drag, const f32 dt);
    inline bool phys_frc_intgrtr_lookup_global_components (phys_frc_intgrtr* i, const phys_frc_accmltr* a);
    inline void phys_frc_intgrtr_exec                     (phys_frc_intgrtr* i, const f32 dt);
    inline f32  phys_frc_intgrtr_clamp_term_velocity      (const f32 vel, const f32 tv);
    inline void phys_frc_intgrtr_constrain_to_maps        (phys_frc_intgrtr* i);
    inline void phys_frc_intgrtr_update_global_components (phys_frc_intgrtr* i);
    inline void phys_frc_intgrtr_reset                    (phys_frc_intgrtr* i);

    IFB_INTERNAL void
    phys_frc_intgrtr_run(
        phys_frc_intgrtr*       integrator,
        const phys_frc_accmltr* accum,
        const f32               dt) {

        IFB_PROFILE_FUNCTION();

        // load every entity with the physics archetype into the integrator,
        // with the force it was given this step, or none
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
        integrator->map_offset_x  =                (f32*)phys_mngr_res_alloc(cfg.entity_capacity * sizeof(f32));

        assert(integrator->force         != NULL);
        assert(integrator->position      != NULL);
        assert(integrator->velocity      != NULL);
        assert(integrator->acceleration  != NULL);
        assert(integrator->term_velocity != NULL);
        assert(integrator->inv_mass      != NULL);
        assert(integrator->drag          != NULL);
        assert(integrator->map           != NULL);
        assert(integrator->map_inset     != NULL);
        assert(integrator->map_offset_x  != NULL);

        return(integrator);
    }

    // every body is integrated, not just the ones with a force this step,
    // so a body that stops being pushed still has drag slow it down,
    // instead of keeping its velocity and freezing in place
    inline bool
    phys_frc_intgrtr_lookup_global_components(
        phys_frc_intgrtr*       i,
        const phys_frc_accmltr* a) {

        const cmpnt_type_u32 physics_types = (
            cmpnt_type_e_position       |
            cmpnt_type_e_velocity       |
            cmpnt_type_e_acceleration   |
            cmpnt_type_e_inv_mass       |
            cmpnt_type_e_drag           |
            cmpnt_type_e_term_velocity
        );

        const u32 entity_count = entity_mngr_get_count();
        for (
            u32 entity_index = 0;
                entity_index < entity_count;
              ++entity_index
        ) {

            // look up the entity
            entity e;
            const bool did_lookup = entity_lookup_by_index_dense(e, entity_index);
            assert(did_lookup);

            // make sure it matches the archetype for integration
            const bool should_integrate = e.archetype.has_all(physics_types);
            if (!should_integrate) continue;

            // a paused scene keeps its entities where they are
            if (!entity_is_updating(e)) continue;


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
            i->map_inset    [integrator_index] = 0.0f;
            i->map_offset_x [integrator_index] = 0.0f;
            if (e.archetype.has_all(cmpnt_type_e_map_coords)) {
                cmpnt_map_coords map_coords;
                cmpnt_lookup_map_coords(e.index_sparse, map_coords);
                i->map[integrator_index] = map_coords.h_map;
            }
            // keep the collision box's footprint inside the map
            vec3 box_offset;
            vec3 box_half_extent;
            if (phys_aabb_get_body_box(e, box_offset, box_half_extent)) {
                i->map_inset    [integrator_index] = box_half_extent.x;
                i->map_offset_x [integrator_index] = box_offset.x;
            }

            i->force        [integrator_index] = phys_frc_accmltr_get(a, e.index_sparse);
            i->sparse_index [integrator_index] = e.index_sparse;

            ++i->count;
        }

        return(i->count > 0);
    }

    // the simd loop reads the vec3 arrays as packed floats, 4 vec3s to 3 registers,
    // and the scalar components 4 to a register
    static_assert(sizeof(vec3)           == sizeof(f32) * 3, "the simd integration needs packed vec3s");
    static_assert(sizeof(cmpnt_inv_mass) == sizeof(f32),     "the simd integration needs packed inv masses");
    static_assert(sizeof(cmpnt_drag)     == sizeof(f32),     "the simd integration needs packed drags");

    // the same as phys_frc_intgrtr_clamp_term_velocity, for 4 lanes
    inline f128
    phys_frc_intgrtr_clamp_term_velocity_f128(
        const f128& vel,
        const f128& tv,
        const f128& zero,
        const f128& infinity) {

        const f128 limit       = simd_f128_select  (simd_f128_a_gt_b(tv, zero), tv, infinity);
        const f128 vel_clamped = simd_f128_a_min_b (limit, vel);
        return(simd_f128_a_max_b(simd_f128_negate(limit), vel_clamped));
    }

    // integrates 4 lanes of the packed vec3 arrays, starting at the float offset
    // the inv mass and drag registers hold each lane's entity's value
    // the math is in the same order as the scalar code, so the results match it
    inline void
    phys_frc_intgrtr_exec_f128(
        const f32* __restrict force,
        const f32* __restrict term_velocity,
        f32*       __restrict position,
        f32*       __restrict velocity,
        f32*       __restrict acceleration,
        const u32             offset,
        const f128&           inv_mass,
        const f128&           drag_pow_dt,
        const f128&           dt,
        const f128&           dt_pow_2_over_2,
        const f128&           zero,
        const f128&           infinity) {

        const f128 acc = simd_f128_a_mul_b(simd_f128_load(&force[offset]), inv_mass);
        f128       vel = simd_f128_load(&velocity[offset]);
        f128       pos = simd_f128_load(&position[offset]);

        // position, then velocity from the velocity before this step
        pos = simd_f128_a_add_b(pos, simd_f128_a_add_b(simd_f128_a_mul_b(vel, dt), simd_f128_a_mul_b(acc, dt_pow_2_over_2)));
        vel = simd_f128_a_mul_b(simd_f128_a_add_b(vel, simd_f128_a_mul_b(acc, dt)), drag_pow_dt);
        vel = phys_frc_intgrtr_clamp_term_velocity_f128(vel, simd_f128_load(&term_velocity[offset]), zero, infinity);

        simd_f128_store(&acceleration [offset], acc);
        simd_f128_store(&position     [offset], pos);
        simd_f128_store(&velocity     [offset], vel);
    }

    inline void
    phys_frc_intgrtr_exec(
        phys_frc_intgrtr* i, const f32 dt) {

        // calculate dt constants
        const f32 dt_pow_2        = dt * dt;
        const f32 dt_pow_2_over_2 = dt_pow_2 * 0.5f;

        // the entities share a few drag values, and dt is the same for all of them,
        // so drag to the power of dt is the same for most of them
        phys_frc_intgrtr_drag_cache drag_cache;
        drag_cache.count = 0;
        drag_cache.next  = 0;

        // replace each drag with drag to the power of dt before the simd loop, which can't call powf
        // the drags are the integrator's copies, they're never written back to the components
        const u32 count = i->count;
        for (
            u32 integrator_index = 0;
                integrator_index < count;
              ++integrator_index
        ) {
            cmpnt_drag& drag = i->drag[integrator_index];
            drag.normal_val  = phys_frc_intgrtr_drag_pow_dt(drag_cache, drag.normal_val, dt);
        }

        // 4 entities at a time, their vec3s are 12 floats, 3 registers
        const f32* __restrict force         = i->force[0].v;
        const f32* __restrict term_velocity = i->term_velocity[0].v;
        const f32* __restrict inv_mass      = &i->inv_mass[0].normal_val;
        const f32* __restrict drag_pow_dt   = &i->drag[0].normal_val;
        f32*       __restrict position      = i->position[0].v;
        f32*       __restrict velocity      = i->velocity[0].v;
        f32*       __restrict acceleration  = i->acceleration[0].v;

        const f128 dt_f128              = simd_f128_set_all(dt);
        const f128 dt_pow_2_over_2_f128 = simd_f128_set_all(dt_pow_2_over_2);
        const f128 zero_f128            = simd_f128_set_all(0.0f);
        const f128 infinity_f128        = simd_f128_set_all(INFINITY);

        const u32 count_simd = count & ~3u;
        for (
            u32 integrator_index = 0;
                integrator_index < count_simd;
                integrator_index += 4
        ) {

            f128 inv_mass_f128    [3];
            f128 drag_pow_dt_f128 [3];
            simd_f128_spread_x3(simd_f128_load(&inv_mass    [integrator_index]), inv_mass_f128[0],    inv_mass_f128[1],    inv_mass_f128[2]);
            simd_f128_spread_x3(simd_f128_load(&drag_pow_dt [integrator_index]), drag_pow_dt_f128[0], drag_pow_dt_f128[1], drag_pow_dt_f128[2]);

            const u32 offset = integrator_index * 3;
            for (
                u32 reg = 0;
                    reg < 3;
                  ++reg
            ) {
                phys_frc_intgrtr_exec_f128(
                    force, term_velocity, position, velocity, acceleration,
                    offset + (reg * 4),
                    inv_mass_f128    [reg],
                    drag_pow_dt_f128 [reg],
                    dt_f128, dt_pow_2_over_2_f128, zero_f128, infinity_f128
                );
            }
        }

        // the last 0-3 entities one at a time
        for (
            u32 integrator_index = count_simd;
                integrator_index < count;
              ++integrator_index
        ) {

            const f32 drag_pow_dt = i->drag[integrator_index].normal_val;

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

            // hold the velocity to the terminal velocity
            cmpnt_velocity&            vel = i->velocity      [integrator_index];
            const cmpnt_term_velocity& tv  = i->term_velocity [integrator_index];
            vel.x = phys_frc_intgrtr_clamp_term_velocity(vel.x, tv.x);
            vel.y = phys_frc_intgrtr_clamp_term_velocity(vel.y, tv.y);
            vel.z = phys_frc_intgrtr_clamp_term_velocity(vel.z, tv.z);
        }
    }

    // drag to the power of dt, from the cache if the drag was seen already this integration
    // the cache is only good for one dt
    inline f32
    phys_frc_intgrtr_drag_pow_dt(
        phys_frc_intgrtr_drag_cache& cache,
        const f32                    drag,
        const f32                    dt) {

        u32 drag_bits;
        memcpy(&drag_bits, &drag, sizeof(drag_bits));

        for (
            u32 index = 0;
                index < cache.count;
              ++index) {

            if (cache.drag_bits[index] == drag_bits) return(cache.drag_pow_dt[index]);
        }

        // a miss fills an empty entry, or replaces the oldest one
        const f32 drag_pow_dt = powf(drag, dt);
        u32       index       = cache.count;
        if (cache.count < PHYS_DRAG_CACHE_SIZE) {
            ++cache.count;
        }
        else {
            index      = cache.next;
            cache.next = (cache.next + 1) % PHYS_DRAG_CACHE_SIZE;
        }
        cache.drag_bits   [index] = drag_bits;
        cache.drag_pow_dt [index] = drag_pow_dt;
        return(drag_pow_dt);
    }

    // the velocity along one axis, held between -tv and tv
    // a terminal velocity of 0 or less is no limit, so the limit is infinite
    // the compares are in the order of minss and maxss, so they compile without branches,
    // and a NaN velocity passes through the same as it did with branches
    inline f32
    phys_frc_intgrtr_clamp_term_velocity(
        const f32 vel,
        const f32 tv) {

        const f32 limit       = (tv > 0.0f) ? tv : INFINITY;
        const f32 vel_clamped = (limit < vel) ? limit : vel;
        return((vel_clamped < -limit) ? -limit : vel_clamped);
    }

    inline void
    phys_frc_intgrtr_constrain_to_maps(
        phys_frc_intgrtr* i) {

        for (
            u32 index = 0;
                index < i->count;
              ++index) {

            if (i->map[index] == INVALID_HANDLE) continue;

            // constrain the box's center, then move back to the position
            cmpnt_position& pos = i->position[index];
            pos.x += i->map_offset_x[index];
            (void)map_constrain_movement(
                i->map       [index],
                i->map_inset [index],
                pos,
                i->velocity  [index]
            );
            pos.x -= i->map_offset_x[index];
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
              ++index) {
        
            cmpnt_update_position      (i->sparse_index[index], i->position[index]);
            cmpnt_update_velocity      (i->sparse_index[index], i->velocity[index]);     
            cmpnt_update_acceleration  (i->sparse_index[index], i->acceleration[index]);
        }
    }

    inline void
    phys_frc_intgrtr_reset(
        phys_frc_intgrtr* i) {

        i->count = 0;
    }
}; 
