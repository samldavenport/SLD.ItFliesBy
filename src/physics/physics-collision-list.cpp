#pragma once

#include "physics-internal.hpp"
#include "ifb-config.hpp"

namespace ifb {

    struct phys_cllsn_list {
        entity_id* a;
        entity_id* b;
        u32        count;
    };

    IFB_INTERNAL phys_cllsn_list*
    phys_cllsn_list_create(
        void) {

        const u32 capacity = config::entity_capacity;
        
        auto list    = (phys_cllsn_list*)phys_mngr_res_alloc(sizeof(phys_cllsn_list));
        auto array_a =       (entity_id*)phys_mngr_res_alloc(capacity * sizeof(entity_id)); 
        auto array_b =       (entity_id*)phys_mngr_res_alloc(capacity * sizeof(entity_id)); 
   
        assert(list    != NULL); 
        assert(array_a != NULL); 
        assert(array_b != NULL); 
    
        list->a     = array_a;
        list->b     = array_b;
        list->count = 0;
        return(list);
    }

    IFB_INTERNAL u32
    phys_cllsn_list_get_count(
        const phys_cllsn_list* list) {

        assert(list);
        assert(list->count <= config::entity_capacity);
        return(list->count);
    }

    IFB_INTERNAL const phys_cllsn_pair
    phys_cllsn_list_get_pair(
        const phys_cllsn_list* list,
        const u32              index) {

        assert(list        != NULL);
        assert(list->a     != NULL);
        assert(list->b     != NULL);
        assert(list->count <= config::entity_capacity);
        assert(list->count > index);
    
        phys_cllsn_pair pair;
        pair.a = list->a[index];
        pair.b = list->b[index];
    
        return(pair);
    }

    IFB_INTERNAL bool
    phys_cllsn_list_insert_pair(
        phys_cllsn_list*       list,
        const phys_cllsn_pair& pair) {

        assert(list        != NULL);
        assert(list->a     != NULL);
        assert(list->b     != NULL);
        assert(list->count <= config::entity_capacity);

        if (list->count == config::entity_capacity) {
            return(false);
        }

        // order the pair so (a,b) and (b,a) are the same entry
        const bool      a_is_lo = (pair.a.val < pair.b.val);
        const entity_id lo      = a_is_lo ? pair.a : pair.b;
        const entity_id hi      = a_is_lo ? pair.b : pair.a;

        // check for the existing pair
        for (
            u32 index = 0;
                index < list->count;
              ++index) {

            if (list->a[index] == lo && list->b[index] == hi) {
                return(false);
            }
        }

        // it doesnt exist in the list, so insert it
        const u32 index = list->count;
        list->a[index] = lo;
        list->b[index] = hi;
        ++list->count;
        return(true);
    }

    IFB_INTERNAL void
    phys_cllsn_list_remove_pair(
        phys_cllsn_list* list,
        const u32        index) {

        assert(list        != NULL);
        assert(list->a     != NULL);
        assert(list->b     != NULL);
        assert(list->count <= config::entity_capacity);
        assert(list->count > index);

        const u32 last_index = list->count-1;
        
        list->a[index] = list->a[last_index]; 
        list->b[index] = list->b[last_index]; 
        
        --list->count;
    }
    
    IFB_INTERNAL void
    phys_cllsn_list_reset(
        phys_cllsn_list* list) {

        assert(list        != NULL);
        assert(list->a     != NULL);
        assert(list->b     != NULL);
        assert(list->count <= config::entity_capacity);

        list->count = 0;
    }
};
