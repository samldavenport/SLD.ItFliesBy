#pragma once

#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "physics-internal.hpp"

namespace ifb {

    constexpr u32 PHYS_BVH_LEAF_MAX   = 2;
    constexpr u32 PHYS_BVH_STACK_SIZE = 64;

    //--------------------------------------------------------------------
    // HELPERS
    //--------------------------------------------------------------------

    inline f32
    phys_bvh_axis_val(
        const vec3& v,
        const u32   axis) {

        const f32 val = (axis == 0) ? v.x : (axis == 1) ? v.y : v.z;
        return(val);
    }

    inline f32
    phys_bvh_centroid_on_axis(
        const phys_aabb& aabb,
        const u32        axis) {

        const f32 centroid = (phys_bvh_axis_val(aabb.min, axis) + phys_bvh_axis_val(aabb.max, axis)) * 0.5f;
        return(centroid);
    }

    inline void
    phys_bvh_swap_entities(
        phys_bvh* bvh,
        const u32 index_a,
        const u32 index_b) {

        const entity_id tmp_id   = bvh->array_entity_ids   [index_a];
        const phys_aabb tmp_aabb = bvh->array_entity_aabbs [index_a];

        bvh->array_entity_ids   [index_a] = bvh->array_entity_ids   [index_b];
        bvh->array_entity_aabbs [index_a] = bvh->array_entity_aabbs [index_b];
        bvh->array_entity_ids   [index_b] = tmp_id;
        bvh->array_entity_aabbs [index_b] = tmp_aabb;
    }

    // quickselect on the entity centroids along the axis
    // afterwards, the entity at index k is in its sorted position,
    // everything in [lo, k) is <= it and everything in (k, hi] is >= it
    inline void
    phys_bvh_select(
        phys_bvh* bvh,
        u32       lo,
        u32       hi,
        const u32 k,
        const u32 axis) {

        const phys_aabb* aabbs = bvh->array_entity_aabbs;

        while (lo < hi) {

            // move the middle element to the end and use it as the pivot
            const u32 mid = lo + ((hi - lo) / 2);
            phys_bvh_swap_entities(bvh, mid, hi);
            const f32 pivot = phys_bvh_centroid_on_axis(aabbs[hi], axis);

            // partition around the pivot
            u32 store = lo;
            for (
                u32 index = lo;
                    index < hi;
                  ++index) {

                if (phys_bvh_centroid_on_axis(aabbs[index], axis) < pivot) {
                    phys_bvh_swap_entities(bvh, index, store);
                    ++store;
                }
            }
            phys_bvh_swap_entities(bvh, store, hi);

            if (store == k) return;
            if (k < store) hi = store - 1;
            else           lo = store + 1;
        }
    }

    inline void
    phys_bvh_node_update_bounds(
        phys_bvh* bvh,
        const u32 node_index) {

        phys_bvh_node& node = bvh->array_nodes[node_index];
        phys_aabb_reset(node.bnd_box);

        const u32 first = node.child_left_or_first;
        for (
            u32 index = first;
                index < first + node.count;
              ++index) {

            phys_aabb_grow(node.bnd_box, bvh->array_entity_aabbs[index]);
        }
    }

    inline void
    phys_bvh_subdivide(
        phys_bvh* bvh,
        const u32 node_index) {

        phys_bvh_node& node = bvh->array_nodes[node_index];
        if (node.count <= PHYS_BVH_LEAF_MAX) return;

        const u32 first = node.child_left_or_first;

        // find the bounds of the entity centroids
        phys_aabb centroid_bounds;
        phys_aabb_reset(centroid_bounds);
        for (
            u32 index = first;
                index < first + node.count;
              ++index) {

            phys_aabb_grow_pt(centroid_bounds, phys_aabb_centroid(bvh->array_entity_aabbs[index]));
        }

        // split along the longest axis of the centroid bounds
        const f32 extent_x = centroid_bounds.max.x - centroid_bounds.min.x;
        const f32 extent_y = centroid_bounds.max.y - centroid_bounds.min.y;
        const f32 extent_z = centroid_bounds.max.z - centroid_bounds.min.z;

        u32 axis       = 0;
        f32 extent_max = extent_x;
        if (extent_y > extent_max) { axis = 1; extent_max = extent_y; }
        if (extent_z > extent_max) { axis = 2; extent_max = extent_z; }

        // split at the median so the tree stays balanced
        // this keeps the depth at log2(n), so the query stack can't overflow
        const u32 count_left  = node.count / 2;
        const u32 count_right = node.count - count_left;
        phys_bvh_select(bvh, first, first + node.count - 1, first + count_left, axis);

        // create the children
        const u32 index_left  = bvh->count_nodes;
        const u32 index_right = index_left + 1;
        bvh->count_nodes += 2;
        assert(bvh->count_nodes <= bvh->capacity_nodes);

        phys_bvh_node& left  = bvh->array_nodes[index_left];
        phys_bvh_node& right = bvh->array_nodes[index_right];
        left.child_left_or_first  = first;
        left.count                = count_left;
        right.child_left_or_first = first + count_left;
        right.count               = count_right;

        // this node is now interior
        node.child_left_or_first = index_left;
        node.count               = 0;

        phys_bvh_node_update_bounds (bvh, index_left);
        phys_bvh_node_update_bounds (bvh, index_right);
        phys_bvh_subdivide          (bvh, index_left);
        phys_bvh_subdivide          (bvh, index_right);
    }

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL phys_bvh*
    phys_bvh_create(
        const u32       entity_count,
        const hnd_arena h_arena) {

        assert(entity_count != 0);
        assert(h_arena      != INVALID_HANDLE);

        // a binary tree with n leaves has 2n - 1 nodes,
        // and there's at most one leaf per entity
        const u32 node_capacity = ((2 * entity_count) - 1);

        const u32 save = arena_save(h_arena);

        auto bvh   =      (phys_bvh*)arena_push(h_arena, sizeof(phys_bvh));
        auto nodes = (phys_bvh_node*)arena_push(h_arena, node_capacity * sizeof(phys_bvh_node));
        auto ids   =     (entity_id*)arena_push(h_arena, entity_count  * sizeof(entity_id));
        auto aabbs =     (phys_aabb*)arena_push(h_arena, entity_count  * sizeof(phys_aabb));

        const bool is_valid = (
            bvh   != NULL &&
            nodes != NULL &&
            ids   != NULL &&
            aabbs != NULL
        );

        if (!is_valid) {
            arena_revert(h_arena, save);
            return(NULL);
        }

        bvh->array_nodes        = nodes;
        bvh->array_entity_ids   = ids;
        bvh->array_entity_aabbs = aabbs;
        bvh->count_nodes        = 0;
        bvh->count_entities     = 0;
        bvh->capacity_nodes     = node_capacity;
        bvh->capacity_entities  = entity_count;
        return(bvh);
    }

    IFB_INTERNAL void
    phys_bvh_build(
        phys_bvh*        bvh,
        const entity_id* ids,
        const phys_aabb* aabbs,
        const u32        count) {

        assert(bvh                     != NULL);
        assert(bvh->array_nodes        != NULL);
        assert(bvh->array_entity_ids   != NULL);
        assert(bvh->array_entity_aabbs != NULL);
        assert(count                   <= bvh->capacity_entities);

        bvh->count_nodes    = 0;
        bvh->count_entities = count;
        if (count == 0) return;

        assert(ids   != NULL);
        assert(aabbs != NULL);

        // copy the entities, the build reorders them
        for (
            u32 index = 0;
                index < count;
              ++index) {

            bvh->array_entity_ids   [index] = ids   [index];
            bvh->array_entity_aabbs [index] = aabbs [index];
        }

        // the root holds every entity, then gets split down
        phys_bvh_node& root = bvh->array_nodes[0];
        root.child_left_or_first = 0;
        root.count               = count;
        bvh->count_nodes         = 1;

        phys_bvh_node_update_bounds (bvh, 0);
        phys_bvh_subdivide          (bvh, 0);
    }

    IFB_INTERNAL u32
    phys_bvh_query_pairs(
        const phys_bvh*  bvh,
        phys_cllsn_list* list) {

        assert(bvh  != NULL);
        assert(list != NULL);

        u32 count_pairs = 0;
        if (bvh->count_nodes == 0) return(count_pairs);

        u32 stack[PHYS_BVH_STACK_SIZE];

        // test each entity against the tree
        for (
            u32 index_a = 0;
                index_a < bvh->count_entities;
              ++index_a) {

            const phys_aabb& aabb_a = bvh->array_entity_aabbs[index_a];

            u32 stack_count = 0;
            stack[stack_count++] = 0;

            while (stack_count > 0) {

                const phys_bvh_node& node = bvh->array_nodes[stack[--stack_count]];
                if (!phys_aabb_check(node.bnd_box, aabb_a)) continue;

                // interior node, check the children
                if (node.count == 0) {
                    assert(stack_count + 2 <= PHYS_BVH_STACK_SIZE);
                    stack[stack_count++] = node.child_left_or_first;
                    stack[stack_count++] = node.child_left_or_first + 1;
                    continue;
                }

                // leaf node, check the entities
                const u32 first = node.child_left_or_first;
                for (
                    u32 index_b = first;
                        index_b < first + node.count;
                      ++index_b) {

                    // every pair gets visited from both sides, only take it from the lower index
                    if (index_b <= index_a) continue;
                    if (!phys_aabb_check(aabb_a, bvh->array_entity_aabbs[index_b])) continue;

                    phys_cllsn_pair pair;
                    pair.a = bvh->array_entity_ids[index_a];
                    pair.b = bvh->array_entity_ids[index_b];
                    if (phys_cllsn_list_insert_pair(list, pair)) {
                        ++count_pairs;
                    }
                }
            }
        }

        return(count_pairs);
    }
};
