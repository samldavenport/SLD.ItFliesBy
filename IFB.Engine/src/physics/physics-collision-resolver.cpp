#pragma once

#include <math.h>
#include "eng-entity.hpp"
#include "eng-map.hpp"

#include "eng-component.hpp"
#include "component-tables.cpp"
#include "entity.cpp"
#include "ifb.hpp"
#include "physics-collision-detection.cpp"
#include "eng-physics.hpp"

namespace ifb {

    struct phys_cllsn_rslvr {
        u32 count_resolved; // pairs resolved on the last run
    };

    // one side of a collision pair
    // bodies without an inverse mass, or tagged immovable, are treated as immovable
    // the tag only affects collisions, the integrator still moves the body by its inverse mass
    // triggers report their contacts, but never push or get pushed
    struct phys_cllsn_body {
        entity         entt;
        cmpnt_position pos;
        cmpnt_velocity vel;
        vec3           offset;      // the box's center is the position plus this
        vec3           half_extent;
        f32            inv_mass;
        bool           has_velocity;
        bool           is_trigger;
    };

    inline bool
    phys_cllsn_rslvr_get_body(
        const entity_id  id,
        phys_cllsn_body& body) {

        if (!entity_lookup_by_id(body.entt, id))                                     return(false);
        if (!body.entt.archetype.has_all(cmpnt_type_e_position))                     return(false);
        if (!phys_aabb_get_body_box(body.entt, body.offset, body.half_extent))       return(false);

        cmpnt_lookup_position(body.entt.index_sparse, body.pos);

        body.vel          = {};
        body.has_velocity = body.entt.archetype.has_all(cmpnt_type_e_velocity);
        if (body.has_velocity) {
            cmpnt_lookup_velocity(body.entt.index_sparse, body.vel);
        }

        body.inv_mass = 0.0f;
        const bool is_immovable = body.entt.archetype.has_all(cmpnt_type_e_immovable);
        if (!is_immovable && body.entt.archetype.has_all(cmpnt_type_e_inv_mass)) {
            cmpnt_inv_mass inv_mass;
            cmpnt_lookup_inv_mass(body.entt.index_sparse, inv_mass);
            body.inv_mass = inv_mass.normal_val;
        }

        // entities without a collider are solid
        body.is_trigger = false;
        if (body.entt.archetype.has_all(cmpnt_type_e_collider)) {
            cmpnt_collider clldr;
            cmpnt_lookup_collider(body.entt.index_sparse, clldr);
            body.is_trigger = (clldr.is_trigger != 0);
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
            cmpnt_lookup_map_coords(body.entt.index_sparse, map_coords);

            // the map keeps the box's footprint inside of it,
            // so constrain the box's center, then move back to the position
            cmpnt_position box_center = body.pos;
            box_center.x += body.offset.x;
            (void)map_constrain_movement(
                map_coords.h_map,
                body.half_extent.x,
                box_center,
                body.vel
            );
            body.pos.x = box_center.x - body.offset.x;
            body.pos.z = box_center.z;
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
        phys_cntct_list*       cntct_list,
        const f32              dt) {

        assert(rslvr);
        assert(cllsn_list);
        assert(cntct_list);

        // resolution is positional, it doesn't depend on the step size
        (void)dt;

        // contacts are rebuilt every run, even when there's nothing to resolve
        rslvr->count_resolved = 0;
        phys_cntct_list_reset(cntct_list);

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

            // the pair came from the swept broad phase,
            // check that they're actually touching after integration
            // and find the axis with the least penetration
            // within the skin counts as touching, the overlap is negative by up to the skin then
            u32  axis        = 0;
            f32  overlap_min = 0.0f;
            bool is_touching = true;
            for (
                u32 axis_index = 0;
                    axis_index < 3;
                  ++axis_index) {

                const f32 center_a = body_a.pos.v[axis_index] + body_a.offset.v[axis_index];
                const f32 center_b = body_b.pos.v[axis_index] + body_b.offset.v[axis_index];
                const f32 delta    = center_b - center_a;
                const f32 overlap = (body_a.half_extent.v[axis_index] + body_b.half_extent.v[axis_index]) - fabsf(delta);
                if (overlap <= -PHYS_CNTCT_SKIN) {
                    is_touching = false;
                    break;
                }
                if (axis_index == 0 || overlap < overlap_min) {
                    axis        = axis_index;
                    overlap_min = overlap;
                }
            }
            if (!is_touching) continue;

            // the normal points from a to b
            const f32 normal = (
                (body_b.pos.v[axis] + body_b.offset.v[axis]) >=
                (body_a.pos.v[axis] + body_a.offset.v[axis])) ? 1.0f : -1.0f;

            // record the contact before anything decides not to push
            // so triggers and immovable pairs are still reported
            phys_cntct cntct;
            cntct.a              = cllsn_pair.a;
            cntct.b              = cllsn_pair.b;
            cntct.normal         = {};
            cntct.normal.v[axis] = normal;
            cntct.depth          = (overlap_min > 0.0f) ? overlap_min : 0.0f; // touching in the skin isn't overlapping
            // the pairs are unique, so this only fails when the list is full
            // a dropped contact would show up as a false end, then a begin
            const bool did_insert = phys_cntct_list_insert(cntct_list, cntct);
            assert(did_insert);
            (void)did_insert;

            // triggers don't push, and don't get pushed
            if (body_a.is_trigger || body_b.is_trigger) continue;

            // only touching, there's nothing to push out of
            if (overlap_min <= 0.0f) continue;

            // if nothing has mass, nothing can move
            const f32 inv_mass_total = body_a.inv_mass + body_b.inv_mass;
            if (inv_mass_total <= 0.0f) continue;

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
