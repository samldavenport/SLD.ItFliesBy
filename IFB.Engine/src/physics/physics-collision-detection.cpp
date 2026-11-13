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

    // builds an aabb for each entity in the list that has a position, and a bounds rect or quad
    // detection runs before integration, so moving entities are swept by
    // their velocity to catch the pairs that will touch during this step
    inline u32
    phys_cllsn_dtctr_get_bounds(
        const entity_list* list,
        entity_id*         ids,
        phys_aabb*         aabbs,
        phys_cllsn_mask*   masks,
        const f32          dt) {

        u32 count = 0;
        const u32 entity_count = eng_entity_list_count(list);
        for (
            u32 i_entity = 0;
                i_entity < entity_count;
              ++i_entity) {

            // get the current entity id
            const entity_id id = eng_entity_list_index(list, i_entity);

            // look up the stuff we need
            entity e;
            vec3 offset;
            vec3 half_extent;
            if (!entity_lookup_by_id(e, id))                     continue;
            if (!e.archetype.has_all(cmpnt_type_e_position))     continue;
            if (!entity_is_updating(e))                          continue;
            if (!phys_aabb_get_body_box(e, offset, half_extent)) continue;

            // get the position
            cmpnt_position pos;
            cmpnt_lookup_position(e.index_sparse, pos);

            // get the mask, if we have one
            phys_cllsn_mask& mask = masks[count];
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
            phys_aabb& aabb = aabbs[count];
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

            // add the id and update the count
            ids[count] = id;
            ++count;
        }

        return(count);
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

        assert(dtctr             != NULL);
        assert(dtctr->cllsn_list != NULL);

        // pairs are rebuilt every run
        phys_cllsn_list_reset(dtctr->cllsn_list);

        // allocate an arena
        const hnd_arena h_arena = arena_alloc();
        if (h_arena == INVALID_HANDLE) return;

        // iterate over each map
        const u32 map_count = map_mngr_get_count();                         
        for (
            u32 i_map = 0;
                i_map < map_count;
              ++i_map) {

              const hnd_map h_map = map_mngr_get_map(i_map);

              map_dimensions dims;
              map_get_dimensions(h_map, dims);
           
              // iterate over each chunk in the map
              for (
                  u32 i_chunk = 0;
                      i_chunk < dims.count_chunks;
                    ++i_chunk) {

                    // save the place in the arena 
                    const u32 save = arena_save(h_arena);

                    // get the entities in the chunk
                    entity_list* list = map_get_entities_in_chunk(h_map, i_chunk, h_arena);
                    if (list == NULL) {
                        arena_revert(h_arena, save); 
                        continue; 
                    } 

                    // make sure we have entities to process
                    const u32 entity_count = eng_entity_list_count(list);
                    if (entity_count == 0) {
                        arena_revert(h_arena, save);
                        continue;
                    }

                    // build bounding box hierarchy / broad phase from list
                    auto* bvh   = phys_bvh_create(entity_count, h_arena);
                    auto* ids   =       (entity_id*)arena_push(h_arena, entity_count * sizeof(entity_id));
                    auto* aabbs =       (phys_aabb*)arena_push(h_arena, entity_count * sizeof(phys_aabb));
                    auto* masks = (phys_cllsn_mask*)arena_push(h_arena, entity_count * sizeof(phys_cllsn_mask));
                    if (bvh == NULL || ids == NULL || aabbs == NULL || masks == NULL) {
                        arena_revert(h_arena, save);
                        continue;
                    }

                    // build the bvh and query for collision pairs
                    const u32 count_bounded = phys_cllsn_dtctr_get_bounds(list, ids, aabbs, masks, dt);
                    phys_bvh_build       (bvh, ids, aabbs, masks, count_bounded);
                    phys_bvh_query_pairs (bvh, dtctr->cllsn_list);

                    // revert the arena
                    arena_revert(h_arena, save);
              }
        }

        // these are only candidates, the events are published from the contacts after the step

        // free the arena
        arena_free(h_arena);
    }
};
