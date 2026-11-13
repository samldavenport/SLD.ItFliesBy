#pragma once

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "memory-reservation.cpp"
#include "eng-entity.hpp"
#include "eng-physics.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    // the force on each entity this step, by its sparse index
    // an entity that wasn't given a force has none, the slots are zeroed every step
    struct phys_frc_accmltr {
        vec3* forces;
        u32   capacity;
    };

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the entity manager has to be started, the slots match its sparse set
    IFB_INTERNAL phys_frc_accmltr*
    phys_frc_accmltr_create(
        void) {

        const u32 capacity          = entity_mngr_capacity_sparse();
        const u32 size_accum        = sizeof(phys_frc_accmltr);
        const u32 size_array_forces = capacity * sizeof(vec3);
        const u32 size_min          = size_accum + size_array_forces;

        addr mem_addr = (addr)phys_mngr_res_alloc(size_min);
        assert(mem_addr != 0);

        auto accum      = (phys_frc_accmltr*)(mem_addr);
        accum->forces   = (vec3*)(mem_addr + size_accum);
        accum->capacity = capacity;
        zero_memory((void*)accum->forces, size_array_forces);

        phys_frc_accmltr_validate(accum);

        return(accum);
    }

    IFB_INTERNAL void
    phys_frc_accmltr_validate(
        const phys_frc_accmltr* const accum) {

        assert(accum           != NULL);
        assert(accum->forces   != NULL);
        assert(accum->capacity != 0);
    }

    IFB_INTERNAL void
    phys_frc_accmltr_add(
        phys_frc_accmltr* const accum,
        const u32               sparse_index,
        const vec3&             v) {

        phys_frc_accmltr_validate(accum);
        assert(sparse_index < accum->capacity);

        vec3& force = accum->forces[sparse_index];
        force.x += v.x;
        force.y += v.y;
        force.z += v.z;
    }

    // the force the entity was given this step, zero if it wasn't given one
    IFB_INTERNAL const vec3&
    phys_frc_accmltr_get(
        const phys_frc_accmltr* const accum,
        const u32                     sparse_index) {

        phys_frc_accmltr_validate(accum);
        assert(sparse_index < accum->capacity);

        return(accum->forces[sparse_index]);
    }

    IFB_INTERNAL void
    phys_frc_accmltr_clear(
        phys_frc_accmltr* const accum,
        const u32               sparse_index) {

        phys_frc_accmltr_validate(accum);
        assert(sparse_index < accum->capacity);

        accum->forces[sparse_index] = { 0.0f, 0.0f, 0.0f };
    }

    IFB_INTERNAL void
    phys_frc_accmltr_reset(
        phys_frc_accmltr* const accum) {

        phys_frc_accmltr_validate(accum);
        zero_memory((void*)accum->forces, accum->capacity * sizeof(vec3));
    }
};
