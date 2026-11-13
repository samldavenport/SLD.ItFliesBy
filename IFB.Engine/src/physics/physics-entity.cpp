#pragma once

#include "entity.cpp"
#include "ifb.hpp"
#include "eng-component.hpp"
#include "eng-physics.hpp"
#include "eng-entity.hpp"

namespace ifb {

    // the forces are kept by sparse index, so a caller that has the entity already doesn't look it up again
    IFB_INTERNAL void
    phys_entity_add_force(
        const u32   sparse_index,
        const vec3& force) {

        assert(sparse_index != INVALID_INDEX);

        phys_frc_accmltr* accum = phys_mngr_get_frc_accmltr();
        phys_frc_accmltr_add(accum, sparse_index, force);
    }

    IFB_INTERNAL void
    phys_entity_clear_force(
        const u32 sparse_index) {

        assert(sparse_index != INVALID_INDEX);

        phys_frc_accmltr* accum = phys_mngr_get_frc_accmltr();
        phys_frc_accmltr_clear(accum, sparse_index);
    }

    // returns false if there's no entity with the id
    IFB_INTERNAL bool
    phys_entity_remove_all_force(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        if (sparse_index == INVALID_INDEX) return(false);

        phys_entity_clear_force(sparse_index);
        return(true);
    }
    
    IFB_INTERNAL void 
    phys_entity_set_mass(
        const entity_id id,
        const f32       mass) {

        assert(id != ENTITY_ID_INVALID);

        // an immovable entity has an inverse mass of 0, see phys_entity_set_inv_mass
        assert(mass > 0.0f);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);

        cmpnt_inv_mass im;
        im.normal_val = 1.0f / mass;

        cmpnt_update_inv_mass(sparse_index, im);
    }
    
    IFB_INTERNAL void 
    phys_entity_set_inv_mass(
        const entity_id id,
        const f32       i_mass) {

        assert(id != ENTITY_ID_INVALID);
        assert(i_mass >= 0.0f);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);

        // the value is already inverted, 0 is an immovable entity
        cmpnt_inv_mass im;
        im.normal_val = i_mass;

        cmpnt_update_inv_mass(sparse_index, im);
    }
};
