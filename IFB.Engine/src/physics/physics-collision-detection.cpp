#pragma once

#include "ifb.hpp"
#include "memory-arena.cpp"
#include "eng-physics.hpp"
#include "ifb-engine.hpp"
#include "eng-entity.hpp"
#include "eng-map.hpp"
#include "eng-memory.hpp"
#include "eng-events.hpp"

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

    // builds an aabb for each entity in the list that has a position and quad
    // detection runs before integration, so moving entities are swept by
    // their velocity to catch the pairs that will touch during this step
    inline u32
    phys_cllsn_dtctr_get_bounds(
        const entity_list* list,
        entity_id*         ids,
        phys_aabb*         aabbs,
        const f32          dt) {

        const component_type bounds_types = (
            cmpnt_type_e_position |
            cmpnt_type_e_quad
        );

        u32 count = 0;
        const u32 entity_count = eng_entity_list_count(list);
        for (
            u32 index = 0;
                index < entity_count;
              ++index) {

            const entity_id id = eng_entity_list_index(list, index);

            entity e;
            if (!entity_lookup_by_id(e, id))        continue;
            if (!e.archetype.has_all(bounds_types)) continue;

            cmpnt_position pos;
            cmpnt_quad     quad;
            cmpnt_lookup_position (e.index_sparse, pos);
            cmpnt_lookup_quad     (e.index_sparse, quad);

            phys_aabb& aabb = aabbs[count];
            phys_aabb_from_center(aabb, pos, phys_aabb_half_extent_quad(quad));

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
                    phys_bvh*  bvh   = phys_bvh_create(entity_count, h_arena);
                    entity_id* ids   = (entity_id*)arena_push(h_arena, entity_count * sizeof(entity_id));
                    phys_aabb* aabbs = (phys_aabb*)arena_push(h_arena, entity_count * sizeof(phys_aabb));
                    if (bvh == NULL || ids == NULL || aabbs == NULL) {
                        arena_revert(h_arena, save);
                        continue;
                    }

                    // build the bvh and query for collision pairs
                    const u32 count_bounded = phys_cllsn_dtctr_get_bounds(list, ids, aabbs, dt);
                    phys_bvh_build       (bvh, ids, aabbs, count_bounded);
                    phys_bvh_query_pairs (bvh, dtctr->cllsn_list);

                    // revert the arena
                    arena_revert(h_arena, save);
              }
        }          

        // publish events
        const u32 cllsn_count = phys_cllsn_list_get_count(dtctr->cllsn_list);
        event_payload payload;
        for (
            u32 i_cllsn = 0;
                i_cllsn < cllsn_count; 
              ++i_cllsn) {

            const auto cllsn_pair = phys_cllsn_list_get_pair(dtctr->cllsn_list, i_cllsn);
            
            payload.data.physics_collision.a = cllsn_pair.a;
            payload.data.physics_collision.b = cllsn_pair.b;
        
            const bool did_publish = event_mngr_publish(event_type_e_physics_collision, payload);
            assert(did_publish);
        }

        // free the arena
        arena_free(h_arena);
    }
};
