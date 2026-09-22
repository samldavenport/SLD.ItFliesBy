#pragma once

#include "entity.cpp"
#include "ifb-types.hpp"
#include "physics.hpp"
#include "component.hpp"
#include "physics-internal.hpp"

namespace ifb {

    IFB_INTERNAL void
    phys_entity_add_force(
        const entity_id id,
        const vec3&     force) {

        assert(id != ENTITY_ID_INVALID);

        phys_frc_accmltr* accum = phys_mngr_get_frc_accmltr();
        
        phys_frc_accmltr_add(
            accum,
            id,
            force
        );
    }

    IFB_INTERNAL bool 
    phys_entity_remove_all_force(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);
    
        phys_frc_accmltr* accum = phys_mngr_get_frc_accmltr();
        
        const bool did_remove = phys_frc_accmltr_remove(
            accum,
            id
        );

        return(did_remove);
    }
    
    IFB_INTERNAL void 
    phys_entity_set_mass(
        const entity_id id,
        const f32       mass) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);

        cmpnt_inv_mass im;
        im.normal_val = 1.0f / mass;

        cmpnt_lookup_inv_mass(sparse_index, im);
    }
    
    IFB_INTERNAL void 
    phys_entity_set_inv_mass(
        const entity_id id,
        const f32       i_mass) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);

        cmpnt_inv_mass im;
        im.normal_val = 1.0f / i_mass;

        cmpnt_update_inv_mass(sparse_index, im);
    }
    
    IFB_INTERNAL void 
    phys_entity_set_rigid_body(
        const entity_id   id,
        const cmpnt_rigid_body& rb) {

        assert(id != ENTITY_ID_INVALID);
   
        bool did_update = false;

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);
    
        cmpnt_update_rigid_body(sparse_index,rb);
    }
};
