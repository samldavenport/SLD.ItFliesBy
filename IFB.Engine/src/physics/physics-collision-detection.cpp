#pragma once

#include "eng-component.hpp"
#include "ifb.hpp"
#include "memory-arena.cpp"
#include "eng-physics.hpp"
#include "ifb-engine.hpp"
#include "eng-entity.hpp"
#include "eng-map.hpp"
#include "eng-memory.hpp"

namespace ifb {

    struct phys_cllsn_dtctr {
        phys_cllsn_list* cllsn_list;
    };

    IFB_INTERNAL phys_cllsn_dtctr*
    phys_cllsn_dtctr_create(
        void) {

        auto cllsn_dtctr = (phys_cllsn_dtctr*)phys_mngr_res_alloc(sizeof(phys_cllsn_dtctr));
        assert(cllsn_dtctr != NULL);

        cllsn_dtctr->cllsn_list = phys_cllsn_list_create();
        assert(cllsn_dtctr->cllsn_list != NULL);

        return(cllsn_dtctr);
    }

    // builds the aabb and mask for an entity that has a position, map coords, and a bounds rect or quad
    // detection runs before integration, so moving entities are swept by
    // their velocity to catch the pairs that will touch during this step
    // false if the entity can't collide
    inline bool
    phys_cllsn_dtctr_get_entity_bounds(
        const entity&    e,
        hnd_map&         h_map,
        phys_aabb&       aabb,
        phys_cllsn_mask& mask,
        const f32        dt) {

        vec3 offset;
        vec3 half_extent;
        if (!e.archetype.has_all(cmpnt_type_e_position | cmpnt_type_e_map_coords)) return(false);
        if (!entity_is_updating(e))                                                  return(false);
        if (!phys_aabb_get_body_box(e, offset, half_extent))                         return(false);

        // the map the entity is on
        cmpnt_map_coords mc;
        cmpnt_lookup_map_coords(e.index_sparse, mc);
        h_map = mc.h_map;

        // get the position
        cmpnt_position pos;
        cmpnt_lookup_position(e.index_sparse, pos);

        // get the mask, if we have one
        if (e.archetype.has_all(cmpnt_type_e_collider)) {
            cmpnt_collider clldr;
            cmpnt_lookup_collider(e.index_sparse, clldr);
            mask.category      = clldr.mask_category;
            mask.collides_with = clldr.mask_collides_with;
        }
        else {
            mask.category      = PHYS_CLLSN_MASK_ALL;
            mask.collides_with = PHYS_CLLSN_MASK_ALL;
        }

        // grab the aabb
        // grown by half the skin on each side, so boxes within the skin of each other still pair up
        const f32  skin_half = PHYS_CNTCT_SKIN * 0.5f;
        const vec3 half_extent_skin = {
            half_extent.x + skin_half,
            half_extent.y + skin_half,
            half_extent.z + skin_half
        };
        phys_aabb_from_center(aabb, vec3_add(pos, offset), half_extent_skin);

        // if we have a velocity, add that to the aabb
        if (e.archetype.has_all(cmpnt_type_e_velocity)) {
            cmpnt_velocity vel;
            cmpnt_lookup_velocity(e.index_sparse, vel);

            phys_aabb aabb_swept;
            aabb_swept.min.x = aabb.min.x + (vel.x * dt);
            aabb_swept.min.y = aabb.min.y + (vel.y * dt);
            aabb_swept.min.z = aabb.min.z + (vel.z * dt);
            aabb_swept.max.x = aabb.max.x + (vel.x * dt);
            aabb_swept.max.y = aabb.max.y + (vel.y * dt);
            aabb_swept.max.z = aabb.max.z + (vel.z * dt);
            phys_aabb_grow(aabb, aabb_swept);
        }

        return(true);
    }

    IFB_INTERNAL phys_cllsn_list*
    phys_cllsn_dtctr_get_list(
        const phys_cllsn_dtctr* dtctr) {

        assert(dtctr             != NULL);
        assert(dtctr->cllsn_list != NULL);
        return(dtctr->cllsn_list);
    }

    IFB_INTERNAL void
    phys_cllsn_dtctr_run(
        phys_cllsn_dtctr* dtctr,
        const f32         dt) {

        IFB_PROFILE_FUNCTION();

        assert(dtctr             != NULL);
        assert(dtctr->cllsn_list != NULL);

        // pairs are rebuilt every run
        phys_cllsn_list_reset(dtctr->cllsn_list);

        const u32 entity_count = entity_mngr_get_count();
        const u32 map_count    = map_mngr_get_count();
        if (entity_count == 0 || map_count == 0) return;

        // allocate an arena
        const hnd_arena h_arena = arena_alloc();
        if (h_arena == INVALID_HANDLE) return;

        // the bounds of every entity that can collide, and the map it's on
        auto* ids      =       (entity_id*)arena_push(h_arena, entity_count * sizeof(entity_id));
        auto* aabbs    =       (phys_aabb*)arena_push(h_arena, entity_count * sizeof(phys_aabb));
        auto* masks    = (phys_cllsn_mask*)arena_push(h_arena, entity_count * sizeof(phys_cllsn_mask));
        auto* h_maps   =         (hnd_map*)arena_push(h_arena, entity_count * sizeof(hnd_map));

        // the entities on the map being processed
        auto* map_ids   =       (entity_id*)arena_push(h_arena, entity_count * sizeof(entity_id));
        auto* map_aabbs =       (phys_aabb*)arena_push(h_arena, entity_count * sizeof(phys_aabb));
        auto* map_masks = (phys_cllsn_mask*)arena_push(h_arena, entity_count * sizeof(phys_cllsn_mask));

        // sized for every entity, it gets rebuilt for each map
        auto* bvh = phys_bvh_create(entity_count, h_arena);

        const bool is_valid = (
            ids       != NULL &&
            aabbs     != NULL &&
            masks     != NULL &&
            h_maps    != NULL &&
            map_ids   != NULL &&
            map_aabbs != NULL &&
            map_masks != NULL &&
            bvh       != NULL
        );
        if (!is_valid) {
            arena_free(h_arena);
            return;
        }

        // a single pass over the entities builds the bounds for all of them
        // this used to be done per chunk, and each chunk scanned every entity
        u32 count_bounded = 0;
        {
            IFB_PROFILE_SCOPE("phys_cllsn_dtctr_bounds");

            entity e;
            for (
                u32 i_entity = 0;
                    i_entity < entity_count;
                  ++i_entity) {

                const bool did_find = entity_lookup_by_index_dense(e, i_entity);
                assert(did_find);

                const bool has_bounds = phys_cllsn_dtctr_get_entity_bounds(
                    e,
                    h_maps [count_bounded],
                    aabbs  [count_bounded],
                    masks  [count_bounded],
                    dt
                );
                if (!has_bounds) continue;

                ids[count_bounded] = e.id;
                ++count_bounded;
            }
        }

        // one bvh per map, entities on different maps never collide
        // every entity is in exactly one bvh, so every pair is found once
        // and pairs that cross a chunk border aren't missed
        for (
            u32 i_map = 0;
                i_map < map_count;
              ++i_map) {

            const hnd_map h_map = map_mngr_get_map(i_map);

            // gather the entities on this map
            u32 map_entity_count = 0;
            for (
                u32 index = 0;
                    index < count_bounded;
                  ++index) {

                if (h_maps[index] != h_map) continue;

                map_ids   [map_entity_count] = ids   [index];
                map_aabbs [map_entity_count] = aabbs [index];
                map_masks [map_entity_count] = masks [index];
                ++map_entity_count;
            }

            if (map_entity_count < 2) continue;

            // build the bvh and query for collision pairs
            {
                IFB_PROFILE_SCOPE("phys_cllsn_dtctr_bvh_build");
                phys_bvh_build(bvh, map_ids, map_aabbs, map_masks, map_entity_count);
            }
            {
                IFB_PROFILE_SCOPE("phys_cllsn_dtctr_bvh_query");
                phys_bvh_query_pairs(bvh, dtctr->cllsn_list, h_arena);
            }
        }

        // these are only candidates, the events are published from the contacts after the step

        // free the arena
        arena_free(h_arena);
    }
};
