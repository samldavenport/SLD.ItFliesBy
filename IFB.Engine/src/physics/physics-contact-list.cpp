#pragma once

#include "eng-physics.hpp"
#include "ifb-config.hpp"

namespace ifb {

    inline void
    phys_cntct_list_validate(
        const phys_cntct_list* list) {

        assert(list               != NULL);
        assert(list->array_a      != NULL);
        assert(list->array_b      != NULL);
        assert(list->array_normal != NULL);
        assert(list->array_depth  != NULL);
        assert(list->count        <= config::entity_capacity);
    }

    IFB_INTERNAL phys_cntct_list*
    phys_cntct_list_create(
        void) {

        // the same capacity as the collision list, every contact starts as a collision pair
        const u32 capacity = config::entity_capacity;

        auto list         = (phys_cntct_list*)phys_mngr_res_alloc(sizeof(phys_cntct_list));
        auto array_a      =       (entity_id*)phys_mngr_res_alloc(capacity * sizeof(entity_id));
        auto array_b      =       (entity_id*)phys_mngr_res_alloc(capacity * sizeof(entity_id));
        auto array_normal =            (vec3*)phys_mngr_res_alloc(capacity * sizeof(vec3));
        auto array_depth  =             (f32*)phys_mngr_res_alloc(capacity * sizeof(f32));

        assert(list         != NULL);
        assert(array_a      != NULL);
        assert(array_b      != NULL);
        assert(array_normal != NULL);
        assert(array_depth  != NULL);

        list->array_a      = array_a;
        list->array_b      = array_b;
        list->array_normal = array_normal;
        list->array_depth  = array_depth;
        list->count        = 0;
        return(list);
    }

    IFB_INTERNAL u32
    phys_cntct_list_get_count(
        const phys_cntct_list* list) {

        phys_cntct_list_validate(list);
        return(list->count);
    }

    IFB_INTERNAL phys_cntct
    phys_cntct_list_get(
        const phys_cntct_list* list,
        const u32              index) {

        phys_cntct_list_validate(list);
        assert(list->count > index);

        phys_cntct cntct;
        cntct.a      = list->array_a      [index];
        cntct.b      = list->array_b      [index];
        cntct.normal = list->array_normal [index];
        cntct.depth  = list->array_depth  [index];
        return(cntct);
    }

    // returns the index of the contact between a and b, in either order
    // or INVALID_INDEX if they aren't touching
    IFB_INTERNAL u32
    phys_cntct_list_find(
        const phys_cntct_list* list,
        const entity_id        a,
        const entity_id        b) {

        phys_cntct_list_validate(list);

        const bool      a_is_lo = (a.val < b.val);
        const entity_id lo      = a_is_lo ? a : b;
        const entity_id hi      = a_is_lo ? b : a;

        for (
            u32 index = 0;
                index < list->count;
              ++index) {

            if (list->array_a[index] == lo && list->array_b[index] == hi) {
                return(index);
            }
        }

        return(INVALID_INDEX);
    }

    // false if the list is full, or the pair is already in it
    IFB_INTERNAL bool
    phys_cntct_list_insert(
        phys_cntct_list*  list,
        const phys_cntct& cntct) {

        phys_cntct_list_validate(list);
        assert(cntct.a     != cntct.b);
        assert(cntct.depth >= 0.0f);

        if (list->count == config::entity_capacity) {
            return(false);
        }

        if (phys_cntct_list_find(list, cntct.a, cntct.b) != INVALID_INDEX) {
            return(false);
        }

        // order the pair so a is the lower id, like the collision list
        // the normal points from a to b, so it flips with the pair
        const bool a_is_lo = (cntct.a.val < cntct.b.val);
        vec3 normal = cntct.normal;
        if (!a_is_lo) {
            normal.x = -normal.x;
            normal.y = -normal.y;
            normal.z = -normal.z;
        }

        const u32 index = list->count;
        list->array_a      [index] = a_is_lo ? cntct.a : cntct.b;
        list->array_b      [index] = a_is_lo ? cntct.b : cntct.a;
        list->array_normal [index] = normal;
        list->array_depth  [index] = cntct.depth;
        ++list->count;
        return(true);
    }

    IFB_INTERNAL void
    phys_cntct_list_reset(
        phys_cntct_list* list) {

        phys_cntct_list_validate(list);
        list->count = 0;
    }
};
