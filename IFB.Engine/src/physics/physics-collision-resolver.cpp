#pragma once

#include <math.h>
#include "entity-internal.hpp"
#include "map-internal.hpp"
#include "eng-components.hpp"

#include "component.hpp"
#include "component-tables.cpp"
#include "entity.cpp"
#include "ifb-types.hpp"
#include "ifb-map.hpp"
#include "physics-collision-detection.cpp"
#include "physics-internal.hpp"

namespace ifb {

    struct phys_cllsn_rslvr {
        u32 count_resolved; // pairs resolved on the last run
    };

    // one side of a collision pair
    // bodies without an inverse mass are treated as immovable
    struct phys_cllsn_body {
        entity         entt;
        cmpnt_position pos;
        cmpnt_velocity vel;
        vec3           half_extent;
        f32            inv_mass;
        bool           has_velocity;
    };

    inline bool
    phys_cllsn_rslvr_get_body(
        const entity_id  id,
        phys_cllsn_body& body) {

        static const component_type cllsn_types = (
            cmpnt_type_e_position |
            cmpnt_type_e_quad
        );

        if (!entity_lookup_by_id(body.entt, id))         return(false);
        if (!body.entt.archetype.has_all(cllsn_types))   return(false);

        cmpnt_quad quad;
        cmpnt_lookup_position (body.entt.index_sparse, body.pos);
        cmpnt_lookup_quad     (body.entt.index_sparse, quad);
        body.half_extent = phys_aabb_half_extent_quad(quad);

        body.vel          = {};
        body.has_velocity = body.entt.archetype.has_all(cmpnt_type_e_velocity);
        if (body.has_velocity) {
            cmpnt_lookup_velocity(body.entt.index_sparse, body.vel);
        }

        body.inv_mass = 0.0f;
        if (body.entt.archetype.has_all(cmpnt_type_e_inv_mass)) {
            cmpnt_inv_mass inv_mass;
            cmpnt_lookup_inv_mass(body.entt.index_sparse, inv_mass);
            body.inv_mass = inv_mass.normal_val;
        }

        return(true);
    }

    // writes the resolved body back to its components
    // entities that belong to a map are kept inside of it, so the map has the final say
    inline void
    phys_cllsn_rslvr_update_body(
        phys_cllsn_body& body) {

        // immovable bodies are left alone
        if (body.inv_mass <= 0.0f) return;

        if (body.entt.archetype.has_all(cmpnt_type_e_map_coords)) {
            cmpnt_map_coords map_coords;
            cmpnt_quad       quad;
            cmpnt_lookup_map_coords (body.entt.index_sparse, map_coords);
            cmpnt_lookup_quad       (body.entt.index_sparse, quad);

            // the position is the center of the quad,
            // so keep half of it away from the edge
            (void)map_constrain_movement(
                map_coords.h_map,
                quad.width * 0.5f,
                body.pos,
                body.vel
            );
        }

        cmpnt_update_position(body.entt.index_sparse, body.pos);
        if (body.has_velocity) {
            cmpnt_update_velocity(body.entt.index_sparse, body.vel);
        }
    }

    IFB_INTERNAL phys_cllsn_rslvr*
    phys_cllsn_rslvr_create(
        void) {

        auto rslvr = (phys_cllsn_rslvr*)phys_mngr_res_alloc(sizeof(phys_cllsn_rslvr));
        assert(rslvr != NULL);

        rslvr->count_resolved = 0;
        return(rslvr);
    }

    IFB_INTERNAL void
    phys_cllsn_rslvr_run(
        phys_cllsn_rslvr*      rslvr,
        const phys_cllsn_list* cllsn_list,
        const f32              dt) {

        assert(rslvr);
        assert(cllsn_list);

        // resolution is positional, it doesn't depend on the step size
        (void)dt;

        rslvr->count_resolved = 0;

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
            if (!phys_cllsn_rslvr_get_body(cllsn_pair.a, body_a)) continue;
            if (!phys_cllsn_rslvr_get_body(cllsn_pair.b, body_b)) continue;

            // if nothing has mass, nothing can move
            const f32 inv_mass_total = body_a.inv_mass + body_b.inv_mass;
            if (inv_mass_total <= 0.0f) continue;

            // the pair came from the swept broad phase,
            // check that they actually overlap after integration
            // and find the axis with the least penetration
            u32  axis        = 0;
            f32  overlap_min = 0.0f;
            bool is_overlap  = true;
            for (
                u32 axis_index = 0;
                    axis_index < 3;
                  ++axis_index) {

                const f32 delta   = body_b.pos.v[axis_index] - body_a.pos.v[axis_index];
                const f32 overlap = (body_a.half_extent.v[axis_index] + body_b.half_extent.v[axis_index]) - fabsf(delta);
                if (overlap <= 0.0f) {
                    is_overlap = false;
                    break;
                }
                if (axis_index == 0 || overlap < overlap_min) {
                    axis        = axis_index;
                    overlap_min = overlap;
                }
            }
            if (!is_overlap) continue;

            // the normal points from a to b
            const f32 normal = (body_b.pos.v[axis] >= body_a.pos.v[axis]) ? 1.0f : -1.0f;

            // push them apart, the lighter one moves more
            const f32 push_a = overlap_min * (body_a.inv_mass / inv_mass_total);
            const f32 push_b = overlap_min * (body_b.inv_mass / inv_mass_total);
            body_a.pos.v[axis] -= normal * push_a;
            body_b.pos.v[axis] += normal * push_b;

            // if they're moving toward each other, cancel the closing velocity
            // so they stop at the contact instead of pushing back in next step
            const f32 vel_closing = (body_b.vel.v[axis] - body_a.vel.v[axis]) * normal;
            if (vel_closing < 0.0f) {
                const f32 impulse = -vel_closing / inv_mass_total;
                body_a.vel.v[axis] -= normal * impulse * body_a.inv_mass;
                body_b.vel.v[axis] += normal * impulse * body_b.inv_mass;
            }

            // write back to the components
            phys_cllsn_rslvr_update_body(body_a);
            phys_cllsn_rslvr_update_body(body_b);
            ++rslvr->count_resolved;
        }
    }
};
