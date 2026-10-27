#pragma once

#include "component-tables.cpp"
#include "entity.cpp"
#include "ifb-types.hpp"
#include "physics-collision-detection.cpp"
#include "physics-internal.hpp"

namespace ifb {

    struct phys_cllsn_rslvr {
        u32*            sparse_index;
        cmpnt_velocity* velocity;
        cmpnt_inv_mass* inv_mass;
    };
  
    struct phys_cllsn_body {
        entity         entt;
        cmpnt_velocity velocity;
        cmpnt_inv_mass inv_mass; 
    };

    inline bool
    phys_cllsn_rslvr_get_body(
        const entity_id  id,
        phys_cllsn_body& body) {

        static const component_type cllsn_types = (
            cmpnt_type_e_position |  
            cmpnt_type_e_velocity |
            cmpnt_type_e_inv_mass
        );

        const bool did_lookup =
            entity_lookup_by_id(body.entt, id) && 
            body.entt.archetype.has_all(cllsn_types); 

        if (did_lookup) {
            cmpnt_lookup_velocity(body.entt.index_sparse, body.velocity);
            cmpnt_lookup_inv_mass(body.entt.index_sparse, body.inv_mass);
        }
        return(did_lookup);
    };
    

    IFB_INTERNAL phys_cllsn_rslvr*
    phys_cllsn_rslvr_create(
        void) {

        auto rslvr = (phys_cllsn_rslvr*)phys_mngr_res_alloc(sizeof(phys_cllsn_rslvr));
        assert(rslvr != NULL);    
        rslvr->sparse_index =            (u32*)phys_mngr_res_alloc(config::entity_capacity * sizeof(u32));
        rslvr->velocity     = (cmpnt_velocity*)phys_mngr_res_alloc(config::entity_capacity * sizeof(u32));
        rslvr->inv_mass     = (cmpnt_inv_mass*)phys_mngr_res_alloc(config::entity_capacity * sizeof(u32));
    
        assert(rslvr->sparse_index);
        assert(rslvr->velocity);
        assert(rslvr->inv_mass);
    }

    IFB_INTERNAL void 
    phys_cllsn_rslvr_run(
        phys_cllsn_rslvr*      rslvr,
        const phys_cllsn_list* cllsn_list,
        const f32              dt) {

        assert(rslvr);
        assert(cllsn_list);

        // check if we have anything to resolve
        const u32 cllsn_count = phys_cllsn_list_get_count(cllsn_list);
        if (cllsn_count == 0) return;
  
        // the two bodies that are colliding
        phys_cllsn_body body_a;
        phys_cllsn_body body_b;

        // iterate through each colliding pair
        for (
            u32 i_cllsn = 0;
                i_cllsn < cllsn_count;
              ++i_cllsn) {

            // get the colliding pair
            const phys_cllsn_pair cllsn_pair = phys_cllsn_list_get_pair(cllsn_list, i_cllsn);
            
            // look up both entities 
            bool did_lookup = true;
            did_lookup &= phys_cllsn_rslvr_get_body(cllsn_pair.a, body_a);
            did_lookup &= phys_cllsn_rslvr_get_body(cllsn_pair.b, body_b);
            if (!did_lookup) continue;

            // if nothing has mass, nothing can move
            const f32 inv_mass_total = body_a.inv_mass.normal_val + body_b.inv_mass.normal_val;
            if (inv_mass_total <= 0.0f) continue;
        }

    }
};
