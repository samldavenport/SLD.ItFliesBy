#pragma once

#include "entity.cpp"
#include "ifb-types.hpp"
#include "physics.hpp"
#include "component.hpp"

namespace ifb {

    IFB_INTERNAL void
    physics_entity_add_force(
        const entity_id id,
        const vec3&     force) {

        assert(id != ENTITY_ID_INVALID);
        physics_accumulator_add(
            _phys_mngr->force_accumulator,
            id,
            force
        );
    }

    IFB_INTERNAL bool 
    physics_entity_remove_all_force(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);
    
        const bool did_remove = physics_accumulator_remove(
            _phys_mngr->force_accumulator,
            id
        );

        return(did_remove);
    }
    
    IFB_INTERNAL void 
    physics_entity_set_mass(
        const entity_id id,
        const f32       mass) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);

        inv_mass im;
        im.normal_val = 1.0f / mass;

        cmpnt_table_inv_mass_update(sparse_index, im);
    }
    
    IFB_INTERNAL void 
    physics_entity_set_inv_mass(
        const entity_id id,
        const f32       i_mass) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);

        inv_mass im;
        im.normal_val = 1.0f / i_mass;

        cmpnt_table_inv_mass_update(sparse_index, im);
    }
    
    IFB_INTERNAL void 
    physics_entity_set_rigid_body(
        const entity_id   id,
        const rigid_body& rb) {

        assert(id != ENTITY_ID_INVALID);
   
        bool did_update = false;

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);
    
        cmpnt_table_rigid_body_update(sparse_index,rb);
    }
};
