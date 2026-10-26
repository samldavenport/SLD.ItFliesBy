#pragma once

#include "ifb-types.hpp"
#include "physics-internal.hpp"

namespace ifb {

    IFB_INTERNAL phys_bvh_node*
    phys_bvh_alloc(
        const hnd_arena h_arena,
        const u32       count) {

        assert(h_arena    != INVALID_HANDLE);

        auto nodes = (phys_bvh_node*)arena_push(h_arena, sizeof(phys_bvh_node) * count);
        for (
             u32 i_node = 0;
                 i_node < count;
               ++i_node) {

            auto& node = nodes[i_node];
            node.child_a = NULL;
            node.child_b = NULL;
            node.entity  = ENTITY_ID_INVALID;
        }

        return(nodes);
    }

    IFB_INTERNAL bool
    phys_bvh_node_is_leaf(
        const phys_bvh_node* node) {

        assert(node != NULL);

        return(node->entity != ENTITY_ID_INVALID);
    } 

    IFB_INTERNAL phys_potential_contact_array*
    phys_bvh_node_get_potential_contacts(
        const phys_bvh_node* node,
        const f32            limit,
        const hnd_arena      h_arena) {

        assert(node    != NULL);
        assert(h_arena != INVALID_HANDLE);

        // check if we have room for contacts or a valid limit
        const bool is_leaf      = phys_bvh_node_is_leaf(node);
        const bool cant_proceed = (is_leaf || limit == 0);            
        if (cant_proceed) {
            return(NULL);
        }

        
    }

    IFB_INTERNAL bool
    phys_bvh_node_aabb_check(
        const phys_bvh_node* node_a,
        const phys_bvh_node* node_b) {

        assert(node_a != NULL);
        assert(node_b != NULL);

        const f32 node_a_x_min = node_a->bounding_box.center.x - node_a->bounding_box.half_extents.x;
        const f32 node_a_x_max = node_a->bounding_box.center.x + node_a->bounding_box.half_extents.x;
        const f32 node_a_y_min = node_a->bounding_box.center.y - node_a->bounding_box.half_extents.y;
        const f32 node_a_y_max = node_a->bounding_box.center.y + node_a->bounding_box.half_extents.y;
        const f32 node_a_z_min = node_a->bounding_box.center.z - node_a->bounding_box.half_extents.z;
        const f32 node_a_z_max = node_a->bounding_box.center.z + node_a->bounding_box.half_extents.z;

        const f32 node_b_x_min = node_b->bounding_box.center.x - node_b->bounding_box.half_extents.x;
        const f32 node_b_x_max = node_b->bounding_box.center.x + node_b->bounding_box.half_extents.x;
        const f32 node_b_y_min = node_b->bounding_box.center.y - node_b->bounding_box.half_extents.y;
        const f32 node_b_y_max = node_b->bounding_box.center.y + node_b->bounding_box.half_extents.y;
        const f32 node_b_z_min = node_b->bounding_box.center.z - node_b->bounding_box.half_extents.z;
        const f32 node_b_z_max = node_b->bounding_box.center.z + node_b->bounding_box.half_extents.z;

        bool does_overlap = true;
        does_overlap &= (node_a_x_min < node_b_x_max);
        does_overlap &= (node_a_x_max > node_b_x_min);
        does_overlap &= (node_a_y_min < node_b_y_max);
        does_overlap &= (node_a_y_max > node_b_y_min);
        does_overlap &= (node_a_z_min < node_b_z_max);
        does_overlap &= (node_a_z_max > node_b_z_min);
        return(does_overlap);
    }

    IFB_INTERNAL phys_potential_contact_array*
    phys_bvh_node_get_potential_contacts_with(
        const phys_bvh_node* node_a,
        const phys_bvh_node* node_b,
        const f32            limit,
        const hnd_arena      h_arena) {

        assert(node_a  != NULL);
        assert(node_b  != NULL);
        assert(h_arena != INVALID_HANDLE);
   
        const bool collision  = phys_bvh_node_aabb_check(node_a, node_b); 
        const bool no_contact = (!collision || limit == 0);
        if (no_contact) {
            return(NULL);
        }
    }
};
