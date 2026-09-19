#pragma once

#include "eng-internal.hpp"
#include "files.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "physics-internal.hpp"

namespace ifb {
    
    struct physics_force_list {
        entity_id* array_ids;
        vec3*      array_forces;
        u32        count;
    };

    inline void
    physics_force_list_validate(
        const physics_force_list* list,
        const u32                 capacity) {

        assert(list);
        assert(list->array_ids    != NULL);
        assert(list->array_forces != NULL);
        assert(list->count        <= capacity);
    }

    inline u32
    physics_force_list_get_index(
        const physics_force_list* list,
        const entity_id           eid,   
        const u32                 capacity) {

        const entity_id* array_ids = list->array_ids;
        
        u32 index = INVALID_INDEX;
        for (
            u32 index_curr = 0;
                index_curr < capacity;
              ++index_curr) {

            if (eid == array_ids[index_curr]) {
                index = index_curr;
                break;
            }         
        }
        return(index);
    }

    IFB_INTERNAL physics_force_list*
    physics_force_list_create(
        void) {

        const auto& cfg = config_instance();
  
        // calculate sizes
        const u32 size_list         = sizeof(physics_force_list);
        const u32 size_array_ids    = cfg.entity_capacity * sizeof(entity_id);
        const u32 size_array_forces = cfg.entity_capacity * sizeof(vec3);
        const u32 size_min          = size_list + size_array_ids + size_array_forces;

        // allocate memory
        addr mem_addr = (addr)physics_mngr_res_alloc(size_min);
        assert(mem_addr != 0);

        // cast pointers
        auto list         = (physics_force_list*)(mem_addr);
        auto array_ids    =          (entity_id*)(mem_addr += size_list); 
        auto array_forces =               (vec3*)(mem_addr += size_array_ids); 
  
        // initialize list
        list->array_ids    = array_ids;
        list->array_forces = array_forces;
        list->count        = 0;
        return(list);
    } 

    IFB_INTERNAL bool
    physics_force_list_add(
        physics_force_list* list,
        const entity_id     eid,
        const vec3&         v3_force) {

        const auto& cfg = config_instance();

        // validate
        physics_force_list_validate(list, cfg.entity_capacity);
        assert(eid != ENTITY_ID_INVALID);

        // check if we can add
        const bool can_add = (list->count < cfg.entity_capacity);
        if (!can_add) return(false); 

        // get the index of the entity, if it exists 
        u32 force_index = physics_force_list_get_index(list, eid, cfg.entity_capacity); 
        if (force_index == INVALID_INDEX) {

            // entity does not exist, add it
            const u32 index = list->count++;
            list->array_ids   [index] = eid;
            list->array_forces[index] = v3_force;
        }
        else {
       
            // entity does exist, update it
            auto& force_curr  = list->array_forces[force_index];
            force_curr.x += v3_force.x;
            force_curr.y += v3_force.y;
            force_curr.z += v3_force.z;
        }

        return(true);
    }

    IFB_INTERNAL bool  
    physics_force_list_lookup(
        physics_force_list* list,
        const entity_id     eid,
        vec3&               v3_force) {

        const auto& config = config_instance();

        // validate
        assert(eid != ENTITY_ID_INVALID); 
        physics_force_list_validate(list, config.entity_capacity);

        // get the force if it exists
        const u32 index       = physics_force_list_get_index(list, eid, config.entity_capacity);
        const bool does_exist = (index != INVALID_INDEX);
        if (does_exist) {
            v3_force = list->array_forces[index];
        } 
        return(does_exist);
    }

    IFB_INTERNAL bool
    physics_force_list_remove(
        physics_force_list* list,
        const entity_id     eid) {

        const auto& cfg = config_instance();
        assert(eid != ENTITY_ID_INVALID);

        const u32  index      = physics_force_list_get_index(list, eid, cfg.entity_capacity);
        const bool can_remove = index != INVALID_INDEX; 
        if (can_remove) {
            const u32 index_last = list->count-1;
            list->array_ids    [index] = list->array_ids    [index_last];
            list->array_forces [index] = list->array_forces [index_last];
            --list->count;
        }   
        return(can_remove);
    }

    IFB_INTERNAL void
    physics_force_list_reset(
        physics_force_list* list) {

        const auto& cfg = config_instance();
        list->count = 0;     
    }

    IFB_INTERNAL u32
    physics_force_list_index_of(
        const physics_force_list* list,
        const u32                 entity_id) {

        const auto& cfg = config_instance();

    }

    IFB_INTERNAL entity_id
    physics_force_list_get_id(
        const physics_force_list* list,
        const u32 index) {

    }

    IFB_INTERNAL vec3&
    physics_force_list_get_force(
        const physics_force_list* list,
        const u32 index) {

    }

    IFB_INTERNAL u32
    physics_force_list_get_count(
        const physics_force_list* list,
        const u32 index) {

    }
};

