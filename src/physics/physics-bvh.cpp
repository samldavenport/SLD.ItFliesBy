#pragma once

#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "physics-internal.hpp"

namespace ifb {

    IFB_INTERNAL phys_bvh*
    phys_bvh_create(
        const u32       entity_count,
        const hnd_arena h_arena) {

        assert(entity_count != 0);
        assert(h_arena      != INVALID_HANDLE);
    
        const u32 node_capacity = ((2 * entity_count) - 1);
   
        const u32 save = arena_save(h_arena); 

        auto bvh   =      (phys_bvh*)arena_push(h_arena, sizeof(phys_bvh));
        auto nodes = (phys_bvh_node*)arena_push(h_arena, node_capacity * sizeof(phys_bvh_node)); 
         
        if (bvh == NULL || nodes == NULL) {
            arena_revert(h_arena, save);
            return(NULL);
        } 

        bvh->array    = nodes;
        bvh->capacity = node_capacity;
        bvh->count    = 0;
        return(bvh);
    }
};
