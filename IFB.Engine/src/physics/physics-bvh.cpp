#pragma once

#include "ifb.hpp"
#include "memory-arena.cpp"
#include "eng-physics.hpp"
#include "eng-memory.hpp"
#include "eng-jobs.hpp"
#include "ifb-profile.hpp"
#include <cfloat>

namespace ifb {

    // up to 4 entities per leaf, about half the nodes of 2 per leaf
    // for a few more box tests in each leaf the query visits
    constexpr u32 PHYS_BVH_LEAF_MAX   = 4;
    constexpr u32 PHYS_BVH_STACK_SIZE = 64;

    //--------------------------------------------------------------------
    // HELPERS
    //--------------------------------------------------------------------

    // the build only moves these refs around, not the ids, boxes, and masks
    // once the tree is built, the entities are gathered into leaf order in one pass
    inline void
    phys_bvh_ref_swap(
        phys_bvh_ref* refs,
        const u32     index_a,
        const u32     index_b) {

        const phys_bvh_ref tmp = refs[index_a];
        refs[index_a] = refs[index_b];
        refs[index_b] = tmp;
    }

    // quickselect on the ref centroids along the axis, refs[lo..hi] inclusive
    // afterwards, the ref at index k is in its sorted position,
    // everything in [lo, k) is <= it and everything in (k, hi] is >= it
    // the partition is three way, so centroids that share a value
    // (quads in a row, or resting on the same ground) don't make it quadratic
    inline void
    phys_bvh_select(
        phys_bvh_ref* refs,
        u32           lo,
        u32           hi,
        const u32     k,
        const u32     axis) {

        assert(lo <= k && k <= hi);

        // the pivot is one of the values in the range, so the equal run is never empty
        // and the range shrinks every pass
        while (lo < hi) {

            // median of three for the pivot
            const u32 mid   = lo + ((hi - lo) / 2);
            const f32 val_a = refs[lo].centroid[axis];
            const f32 val_b = refs[mid].centroid[axis];
            const f32 val_c = refs[hi].centroid[axis];
            const f32 pivot = fmaxf(fminf(val_a, val_b), fminf(fmaxf(val_a, val_b), val_c));

            // [lo, lt) < pivot, [lt, gt) == pivot, [gt, hi] > pivot
            u32 lt    = lo;
            u32 index = lo;
            u32 gt    = hi + 1;
            while (index < gt) {
                const f32 val = refs[index].centroid[axis];
                if (val < pivot) {
                    phys_bvh_ref_swap(refs, lt, index);
                    ++lt;
                    ++index;
                }
                else if (val > pivot) {
                    --gt;
                    phys_bvh_ref_swap(refs, index, gt);
                }
                else {
                    ++index;
                }
            }

            if      (k <  lt) hi = lt - 1;
            else if (k >= gt) lo = gt;
            else              return;
        }
    }

    inline void
    phys_bvh_node_update_bounds(
        phys_bvh*        bvh,
        const u32        node_index,
        const phys_aabb* aabbs) {

        phys_bvh_node& node = bvh->array_nodes[node_index];
        phys_aabb_reset(node.bnd_box);

        const phys_bvh_ref* refs  = bvh->array_build_refs;
        const u32           first = node.child_left_or_first;
        for (
            u32 index = first;
                index < first + node.count;
              ++index) {

            phys_aabb_grow(node.bnd_box, aabbs[refs[index].index]);
        }
    }

    // splits a single node into two children, the children are
    // appended to the end of the node array to be split later
    inline void
    phys_bvh_subdivide(
        phys_bvh*        bvh,
        const u32        node_index,
        const phys_aabb* aabbs) {

        phys_bvh_node& node = bvh->array_nodes[node_index];
        if (node.count <= PHYS_BVH_LEAF_MAX) return;

        const phys_bvh_ref* refs  = bvh->array_build_refs;
        const u32           first = node.child_left_or_first;

        // find the bounds of the entity centroids
        f32 centroid_min[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
        f32 centroid_max[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (
            u32 index = first;
                index < first + node.count;
              ++index) {

            for (
                u32 axis = 0;
                    axis < 3;
                  ++axis) {

                centroid_min[axis] = fminf(centroid_min[axis], refs[index].centroid[axis]);
                centroid_max[axis] = fmaxf(centroid_max[axis], refs[index].centroid[axis]);
            }
        }

        // split along the longest axis of the centroid bounds
        u32 axis       = 0;
        f32 extent_max = centroid_max[0] - centroid_min[0];
        for (
            u32 axis_check = 1;
                axis_check < 3;
              ++axis_check) {

            const f32 extent = centroid_max[axis_check] - centroid_min[axis_check];
            if (extent > extent_max) {
                axis       = axis_check;
                extent_max = extent;
            }
        }

        // split at the median so the tree stays balanced
        // this keeps the depth at log2(n), so the query stack can't overflow
        const u32 count_left  = node.count / 2;
        const u32 count_right = node.count - count_left;
        phys_bvh_select(bvh->array_build_refs, first, first + node.count - 1, first + count_left, axis);

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

        phys_bvh_node_update_bounds (bvh, index_left,  aabbs);
        phys_bvh_node_update_bounds (bvh, index_right, aabbs);
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

        auto bvh   =        (phys_bvh*)arena_push(h_arena, sizeof(phys_bvh));
        auto nodes =   (phys_bvh_node*)arena_push(h_arena, node_capacity * sizeof(phys_bvh_node));
        auto ids   =       (entity_id*)arena_push(h_arena, entity_count  * sizeof(entity_id));
        auto aabbs =       (phys_aabb*)arena_push(h_arena, entity_count  * sizeof(phys_aabb));
        auto masks = (phys_cllsn_mask*)arena_push(h_arena, entity_count  * sizeof(phys_cllsn_mask));
        auto refs  =    (phys_bvh_ref*)arena_push(h_arena, entity_count  * sizeof(phys_bvh_ref));

        const bool is_valid = (
            bvh   != NULL &&
            nodes != NULL &&
            ids   != NULL &&
            aabbs != NULL &&
            masks != NULL &&
            refs  != NULL
        );

        if (!is_valid) {
            arena_revert(h_arena, save);
            return(NULL);
        }

        bvh->array_nodes        = nodes;
        bvh->array_entity_ids   = ids;
        bvh->array_entity_aabbs = aabbs;
        bvh->array_cllsn_masks  = masks;
        bvh->array_build_refs   = refs;
        bvh->count_nodes        = 0;
        bvh->count_entities     = 0;
        bvh->capacity_nodes     = node_capacity;
        bvh->capacity_entities  = entity_count;
        return(bvh);
    }

    IFB_INTERNAL void
    phys_bvh_build(
        phys_bvh*              bvh,
        const entity_id*       ids,
        const phys_aabb*       aabbs,
        const phys_cllsn_mask* masks,
        const u32              count) {

        assert(bvh                     != NULL);
        assert(bvh->array_nodes        != NULL);
        assert(bvh->array_entity_ids   != NULL);
        assert(bvh->array_entity_aabbs != NULL);
        assert(bvh->array_cllsn_masks  != NULL);
        assert(bvh->array_build_refs   != NULL);
        assert(count                   <= bvh->capacity_entities);

        bvh->count_nodes    = 0;
        bvh->count_entities = count;
        if (count == 0) return;

        assert(ids   != NULL);
        assert(aabbs != NULL);
        assert(masks != NULL);

        // the entities are gathered from these at the end, so they can't be the bvh's own arrays
        assert(ids   != bvh->array_entity_ids);
        assert(aabbs != bvh->array_entity_aabbs);
        assert(masks != bvh->array_cllsn_masks);

        // a ref per entity, the centroid is worked out once here instead of on every comparison
        phys_bvh_ref* refs = bvh->array_build_refs;
        for (
            u32 index = 0;
                index < count;
              ++index) {

            const vec3 centroid = phys_aabb_centroid(aabbs[index]);
            refs[index].centroid[0] = centroid.x;
            refs[index].centroid[1] = centroid.y;
            refs[index].centroid[2] = centroid.z;
            refs[index].index       = index;
        }

        // the root holds every entity, then gets split down
        phys_bvh_node& root = bvh->array_nodes[0];
        root.child_left_or_first = 0;
        root.count               = count;
        bvh->count_nodes         = 1;

        phys_bvh_node_update_bounds(bvh, 0, aabbs);

        // split the nodes in the order they're created
        // children always land after their parent, so a single pass visits every node
        // and the node count can never pass the capacity
        for (
            u32 node_index = 0;
                node_index < bvh->capacity_nodes;
              ++node_index) {

            if (node_index >= bvh->count_nodes) break;
            phys_bvh_subdivide(bvh, node_index, aabbs);
        }

        // gather the entities into leaf order, the query walks these
        for (
            u32 index = 0;
                index < count;
              ++index) {

            const u32 from = refs[index].index;
            bvh->array_entity_ids   [index] = ids   [from];
            bvh->array_entity_aabbs [index] = aabbs [from];
            bvh->array_cllsn_masks  [index] = masks [from];
        }
    }

    // the pairs one range of entities found, in the order the serial query would have found them
    struct phys_bvh_query_result {
        phys_cllsn_pair* pairs;
        u32              capacity;
        u32              count;
        u32              count_dropped; // didn't fit in the buffer
    };

    struct phys_bvh_query_job_ctx {
        const phys_bvh*        bvh;
        phys_bvh_query_result* result;
        job_counter*           counter;
        u32                    index_first;
        u32                    index_end;
    };
    static_assert(sizeof(phys_bvh_query_job_ctx) <= sizeof(job_ctx), "phys_bvh_query_job_ctx doesn't fit in a job");

    // below this many entities the query runs on the calling thread, the jobs cost more than they save
    constexpr u32 PHYS_BVH_QUERY_JOB_MIN_ENTITIES = 128;

    // the smallest range a job gets, and how many ranges each thread gets
    // more ranges than threads, so a thread that gets a crowded range doesn't hold everyone up
    constexpr u32 PHYS_BVH_QUERY_JOB_MIN_RANGE      = 64;
    constexpr u32 PHYS_BVH_QUERY_JOB_RANGES_THREAD  = 4;

    // tests the entities in [index_first, index_end) against the tree
    // the tree is only read, so ranges can run on different threads at the same time
    inline void
    phys_bvh_query_range(
        const phys_bvh*        bvh,
        const u32              index_first,
        const u32              index_end,
        phys_bvh_query_result& result) {

        assert(bvh         != NULL);
        assert(result.pairs != NULL);
        assert(index_end   <= bvh->count_entities);

        u32 stack[PHYS_BVH_STACK_SIZE];

        for (
            u32 index_a = index_first;
                index_a < index_end;
              ++index_a) {

            const phys_aabb&       aabb_a = bvh->array_entity_aabbs[index_a];
            const phys_cllsn_mask& mask_a = bvh->array_cllsn_masks[index_a];

            u32 stack_count = 0;
            stack[stack_count++] = 0;

            // every node gets pushed at most once per traversal,
            // so the node count bounds the iterations
            for (
                u32 iteration = 0;
                    iteration < bvh->count_nodes;
                  ++iteration) {

                if (stack_count == 0) break;

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

                    // check that we care about what we are colliding with
                    // before the box test, it's the cheaper of the two
                    const phys_cllsn_mask& mask_b = bvh->array_cllsn_masks[index_b];
                    if ((mask_a.collides_with & mask_b.category) == 0) continue;
                    if ((mask_b.collides_with & mask_a.category) == 0) continue;

                    if (!phys_aabb_check(aabb_a, bvh->array_entity_aabbs[index_b])) continue;

                    if (result.count == result.capacity) {
                        ++result.count_dropped;
                        continue;
                    }

                    phys_cllsn_pair& pair = result.pairs[result.count++];
                    pair.a = bvh->array_entity_ids[index_a];
                    pair.b = bvh->array_entity_ids[index_b];
                }
            }

            assert(stack_count == 0);
        }
    }

    // where a range starts when the entities are split evenly, the first ranges get the remainder
    inline u32
    phys_bvh_query_range_first(
        const u32 i_range,
        const u32 range_size,
        const u32 range_remainder) {

        const u32 extra = (i_range < range_remainder) ? i_range : range_remainder;
        return((i_range * range_size) + extra);
    }

    // runs on a worker, only writes its own result, then marks itself done
    IFB_INTERNAL void
    phys_bvh_query_job(
        const hnd_arena h_arena,
        void*           ctx) {

        (void)h_arena;

        IFB_PROFILE_SCOPE("phys_bvh_query_job");

        const auto* query = (const phys_bvh_query_job_ctx*)ctx;
        phys_bvh_query_range(query->bvh, query->index_first, query->index_end, *query->result);
        job_counter_done(*query->counter);
    }

    // finds every candidate pair in the tree and adds it to the list
    // the entities are split into ranges that run as jobs, the calling thread runs the first range itself
    // each range writes to its own buffer, and the buffers are added to the list in range order,
    // so the list comes out the same as if it ran on one thread
    // the buffers come from the arena, and are gone when it returns
    IFB_INTERNAL u32
    phys_bvh_query_pairs(
        const phys_bvh*  bvh,
        phys_cllsn_list* list,
        const hnd_arena  h_arena) {

        assert(bvh     != NULL);
        assert(list    != NULL);
        assert(h_arena != INVALID_HANDLE);

        u32 count_pairs = 0;
        const u32 count_entities = bvh->count_entities;
        if (bvh->count_nodes == 0 || count_entities < 2) return(count_pairs);

        // how many ranges to split the entities into
        u32 range_count = 1;
        const u32 worker_count = job_dspchr_get_worker_count();
        if (worker_count > 0 && count_entities >= PHYS_BVH_QUERY_JOB_MIN_ENTITIES) {

            const u32 range_count_max = (worker_count + 1) * PHYS_BVH_QUERY_JOB_RANGES_THREAD;
            const u32 range_count_fit = count_entities / PHYS_BVH_QUERY_JOB_MIN_RANGE;
            range_count = (range_count_fit < range_count_max) ? range_count_fit : range_count_max;
            if (range_count == 0) range_count = 1;
        }

        // a buffer per range, each can hold everything the list can
        // a range can't find more pairs than the list has room for without the list dropping them too
        const u32 save            = arena_save(h_arena);
        const u32 buffer_capacity = config::entity_capacity;
        auto*     results         = (phys_bvh_query_result*)arena_push(h_arena, range_count * sizeof(phys_bvh_query_result));
        bool      is_valid        = (results != NULL);
        for (
            u32 i_range = 0;
                is_valid && i_range < range_count;
              ++i_range) {

            results[i_range].pairs         = (phys_cllsn_pair*)arena_push(h_arena, buffer_capacity * sizeof(phys_cllsn_pair));
            results[i_range].capacity      = buffer_capacity;
            results[i_range].count         = 0;
            results[i_range].count_dropped = 0;
            is_valid &= (results[i_range].pairs != NULL);
        }

        assert(is_valid);
        if (!is_valid) {
            arena_revert(h_arena, save);
            return(count_pairs);
        }

        // split the entities evenly, the first ranges get the remainder
        const u32 range_size      = count_entities / range_count;
        const u32 range_remainder = count_entities % range_count;

        // hand out every range but the first
        job_counter counter = {};
        if (range_count > 1) {

            IFB_PROFILE_SCOPE("phys_bvh_query_dispatch");

            job_counter_add(counter, range_count - 1);
            for (
                u32 i_range = 1;
                    i_range < range_count;
                  ++i_range) {

                job j;
                j.proc = phys_bvh_query_job;

                auto* query = (phys_bvh_query_job_ctx*)j.ctx.data;
                query->bvh         = bvh;
                query->result      = &results[i_range];
                query->counter     = &counter;
                query->index_first = phys_bvh_query_range_first(i_range, range_size, range_remainder);
                query->index_end   = phys_bvh_query_range_first(i_range + 1, range_size, range_remainder);

                // the queue is full, so do it here
                if (!job_dspchr_add_job(j)) {
                    phys_bvh_query_range(bvh, query->index_first, query->index_end, results[i_range]);
                    job_counter_done(counter);
                }
            }
        }

        // the first range runs here while the workers do the rest
        phys_bvh_query_range(bvh, 0, phys_bvh_query_range_first(1, range_size, range_remainder), results[0]);

        if (range_count > 1) {
            job_counter_wait(counter);
        }

        // add the pairs to the list in range order
        {
            IFB_PROFILE_SCOPE("phys_bvh_query_merge");

            for (
                u32 i_range = 0;
                    i_range < range_count;
                  ++i_range) {

                const phys_bvh_query_result& result = results[i_range];
                for (
                    u32 i_pair = 0;
                        i_pair < result.count;
                      ++i_pair) {

                    if (phys_cllsn_list_insert_pair(list, result.pairs[i_pair])) {
                        ++count_pairs;
                    }
                }
                phys_cllsn_list_add_dropped(list, result.count_dropped);
            }
        }

        arena_revert(h_arena, save);
        return(count_pairs);
    }
};
