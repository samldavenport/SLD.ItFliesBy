#pragma once

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "physics.hpp"
#include "physics-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL physics_accumulator*
    physics_accumulator_init(
        reservation* res) {

        const auto& cfg = config_instance();

        auto accum = global_alloc<physics_accumulator>();
        assert(accum != NULL);         

        accum->global_forces = physics_force_list_create();
        assert(accum->global_forces != NULL);
   
        physics_accumulator_validate(accum);

        return(accum);
    }

    IFB_INTERNAL void
    physics_accumulator_validate(
        physics_accumulator* const accum) {

        assert(accum                != NULL);
        assert(accum->global_forces != NULL);
    }

    IFB_INTERNAL void
    physics_accumulator_add(
        physics_accumulator* const accum,
        const entity_id            id,
        const vec3&                v) {
        
        physics_accumulator_validate(accum);
        assert(id != ENTITY_ID_INVALID);

        const bool did_add = physics_force_list_add(accum->global_forces, id, v);
        assert(did_add);
    }

    IFB_INTERNAL bool
    physics_accumulator_lookup(
        physics_accumulator* const accum,
        const entity_id            id,
        vec3&                      v) {

        physics_accumulator_validate(accum);
        assert(id != ENTITY_ID_INVALID);

        const bool found = physics_force_list_lookup(accum->global_forces, id, v);
        return(found);
    }

    IFB_INTERNAL bool
    physics_accumulator_remove(
        physics_accumulator* const accum,
        const entity_id            id) {

        physics_accumulator_validate(accum);
        assert(id != ENTITY_ID_INVALID);
       
        const bool did_remove = physics_force_list_remove(accum->global_forces, id);        
        return(did_remove);
    }
    
    IFB_INTERNAL void
    physics_accumulator_reset(
        physics_accumulator* const accum) {

        physics_accumulator_validate(accum);
        physics_force_list_reset(accum->global_forces);
    }
};
