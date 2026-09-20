#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "physics-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
    struct physics_accumulator {
        struct {
            entity_id* ids; 
            vec3*      forces;
        } data;
        u32 capacity;
        u32 count;
    };

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL physics_accumulator*
    physics_accumulator_create(
        reservation* res) {

        const auto& cfg = config_instance();

        const u32 size_accum        = sizeof(physics_accumulator);  
        const u32 size_array_ids    = cfg.entity_capacity * sizeof(entity_id);
        const u32 size_array_forces = cfg.entity_capacity * sizeof(vec3);
        const u32 size_min          = size_accum + size_array_ids + size_array_forces;

        addr mem_addr = (addr)physics_mngr_res_alloc(size_min);
        assert(mem_addr != 0); 

        auto accum        = (physics_accumulator*)(mem_addr);
        auto array_ids    =           (entity_id*)(mem_addr += size_accum);
        auto array_forces =                (vec3*)(mem_addr += size_array_ids);
        
        accum->data.ids    = array_ids;
        accum->data.forces = array_forces;
        accum->capacity    = cfg.entity_capacity;
        accum->count       = 0;
  
        physics_accumulator_validate(accum);

        return(accum);
    }

    IFB_INTERNAL void
    physics_accumulator_validate(
        physics_accumulator* const accum) {

        assert(accum              != NULL);
        assert(accum->data.ids    != NULL);
        assert(accum->data.forces != NULL);
        assert(accum->capacity    != 0);
        assert(accum->count       <= accum->capacity);
    }

    IFB_INTERNAL void
    physics_accumulator_add(
        physics_accumulator* const accum,
        const entity_id            id,
        const vec3&                v) {
        
        physics_accumulator_validate(accum);
        assert(id != ENTITY_ID_INVALID);

        // if the entity has a vector already,
        // update it
        for (
            u32 i = 0;
            i < accum->count;
            ++i
        ) {
            if (id == accum->data.ids[i]) {
                vec3& v_new = accum->data.forces[i];
                v_new.x += v.x;
                v_new.y += v.y;
                v_new.z += v.z;
                return;
            }
        }

        // get the index and update the count
        const u32 index_new = accum->count;
        ++accum->count;

        // set the id and vector
        accum->data.ids    [index_new] = id;
        accum->data.forces [index_new] = v;
    }

    IFB_INTERNAL bool
    physics_accumulator_lookup(
        physics_accumulator* const accum,
        const entity_id            id,
        vec3&                      v) {

        physics_accumulator_validate(accum);
        assert(id != ENTITY_ID_INVALID);

        for (
            u32 i = 0;
            i < accum->count;
            ++i
        ) {
            if (id == accum->data.ids[i]) {
                v = accum->data.forces[i];
                return(true);
            }
        }

        return(false);
    }

    IFB_INTERNAL bool
    physics_accumulator_remove(
        physics_accumulator* const accum,
        const entity_id            id) {


        const u32 last = accum->count - 1;
        if (id == accum->data.ids[last]) {
            --accum->count;
            return(true);
        }

        for (
            u32 i = 0;
            i < accum->count;
            ++i
        ) {
            if (id == accum->data.ids[i]) {
                if (accum->count > 1) {
                    accum->data.ids     [i] = accum->data.ids    [last];
                    accum->data.forces  [i] = accum->data.forces [last];
                }
                --accum->count;
                return(true);
            }
        }

        return(false);
    }
    
    IFB_INTERNAL void
    physics_accumulator_reset(
        physics_accumulator* const accum) {

        physics_accumulator_validate(accum);
        accum->count = 0;
    }
};
