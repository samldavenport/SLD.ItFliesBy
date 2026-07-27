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
    
    IFB_INTERNAL bool 
    physics_entity_set_mass(
        const entity_id id,
        const f32       mass) {

        assert(id != ENTITY_ID_INVALID);
    
        return(false);
    }
    
    IFB_INTERNAL bool 
    physics_entity_set_inv_mass(
        const entity_id id,
        const f32       inv_mass) {

        assert(id != ENTITY_ID_INVALID);
        return(false);
    }
    
    IFB_INTERNAL bool 
    physics_entity_set_rigid_body(
        const entity_id   id,
        const rigid_body& rb) {

        assert(id != ENTITY_ID_INVALID);
   
        bool did_update = false;

        entity e;
        if (entity_lookup_by_id(e, id)) {
            cmpnt_rigid_body_table_update(
                rb,
                e.index_sparse
            );
            did_update = true;
        }
        return(did_update);
    }
};
