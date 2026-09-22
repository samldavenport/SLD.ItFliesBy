#pragma once

#include "ifb-collections.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "physics-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
    struct phys_frc_accmltr {
        struct {
            entity_id* ids; 
            vec3*      forces;
        } world_data;
        struct {
            entity_id* ids;
            vec3*      forces;
        } map_data;
        u32 world_count;
        u32 map_count;
    };

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL phys_frc_accmltr*
    phys_frc_accmltr_create(
        void) {

        const auto& cfg = config_instance();

        const u32 size_accum        = sizeof(phys_frc_accmltr);  
        const u32 size_array_ids    = cfg.entity_capacity * sizeof(entity_id);
        const u32 size_array_forces = cfg.entity_capacity * sizeof(vec3);
        const u32 size_min          = size_accum + size_array_ids + size_array_forces;

        addr mem_addr = (addr)phys_mngr_res_alloc(size_min);
        assert(mem_addr != 0); 

        auto accum        = (phys_frc_accmltr*)(mem_addr);
        auto array_ids    =        (entity_id*)(mem_addr += size_accum);
        auto array_forces =             (vec3*)(mem_addr += size_array_ids);
        
        accum->world_data.ids    = array_ids;
        accum->world_data.forces = array_forces;
        accum->world_count       = 0;
  
        phys_frc_accmltr_validate(accum, cfg.entity_capacity);

        return(accum);
    }

    IFB_INTERNAL void
    phys_frc_accmltr_validate(
        phys_frc_accmltr* const accum,
        const u32               capacity) {

        assert(accum              != NULL);
        assert(capacity           != 0);
        assert(accum->world_data.ids    != NULL);
        assert(accum->world_data.forces != NULL);
        assert(accum->world_count       <= capacity);
    }

    IFB_INTERNAL void
    phys_frc_accmltr_add(
        phys_frc_accmltr* const accum,
        const entity_id            id,
        const vec3&                v) {
       
        const auto& cfg = config_instance();

        phys_frc_accmltr_validate(accum, cfg.entity_capacity);
        assert(id != ENTITY_ID_INVALID);

        // if the entity has a vector already,
        // update it
        for (
            u32 i = 0;
            i < accum->world_count;
            ++i
        ) {
            if (id == accum->world_data.ids[i]) {
                vec3& v_new = accum->world_data.forces[i];
                v_new.x += v.x;
                v_new.y += v.y;
                v_new.z += v.z;
                return;
            }
        }

        // get the index and update the count
        const u32 index_new = accum->world_count;
        ++accum->world_count;

        // set the id and vector
        accum->world_data.ids    [index_new] = id;
        accum->world_data.forces [index_new] = v;
    }

    IFB_INTERNAL bool
    phys_frc_accmltr_lookup(
        phys_frc_accmltr* const accum,
        const entity_id            id,
        vec3&                      v) {

        const auto& cfg = config_instance();
        phys_frc_accmltr_validate(accum, cfg.entity_capacity);
        assert(id != ENTITY_ID_INVALID);

        for (
            u32 i = 0;
            i < accum->world_count;
            ++i
        ) {
            if (id == accum->world_data.ids[i]) {
                v = accum->world_data.forces[i];
                return(true);
            }
        }

        return(false);
    }

    IFB_INTERNAL bool
    phys_frc_accmltr_remove(
        phys_frc_accmltr* const accum,
        const entity_id            id) {


        const u32 last = accum->world_count - 1;
        if (id == accum->world_data.ids[last]) {
            --accum->world_count;
            return(true);
        }

        for (
            u32 i = 0;
            i < accum->world_count;
            ++i
        ) {
            if (id == accum->world_data.ids[i]) {
                if (accum->world_count > 1) {
                    accum->world_data.ids     [i] = accum->world_data.ids    [last];
                    accum->world_data.forces  [i] = accum->world_data.forces [last];
                }
                --accum->world_count;
                return(true);
            }
        }

        return(false);
    }
    
    IFB_INTERNAL void
    phys_frc_accmltr_reset(
        phys_frc_accmltr* const accum) {


        const auto& cfg = config_instance();
        phys_frc_accmltr_validate(accum, cfg.entity_capacity);
        accum->world_count = 0;
    }
    
    IFB_INTERNAL u32
    phys_frc_accmltr_get_count(
        const phys_frc_accmltr* accum) {

        assert(accum);
        return(accum->world_count);
    }
    
    IFB_INTERNAL entity_id
    phys_frc_accmltr_get_entity_id(
        const phys_frc_accmltr* accum,
        const u32               index) {

        assert(accum != NULL);
        assert(index < accum->world_count);

        return(accum->world_data.ids[index]);
    }

    IFB_INTERNAL const vec3&
    phys_frc_accmltr_get_force(
        const phys_frc_accmltr* accum,
        const u32               index) {

        assert(accum != NULL);
        assert(index < accum->world_count);

        return(accum->world_data.forces[index]);
    }
};
