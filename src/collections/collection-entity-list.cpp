#pragma once

#include "entity.hpp"
#include "ifb-collections.hpp"
#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITION
    //--------------------------------------------------------------------
    
    struct entity_list {
        entity_id* ids;
        u32        capacity;
        u32        count;
    };

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL entity_list* 
    entity_list_arena_create(
        const eng_arena_handle a) {

        const u32 save = arena_save(a);

        const auto& cfg = config_instance();

        auto list = arena_push<entity_list>(a);
        auto ids  = arena_push<entity_id>  (a, cfg.entity_capacity);
        
        if (list == NULL || ids == NULL) {
            arena_revert(a, save);
            return(NULL);
        }
        arena_commit(a, save);

        list->ids      = ids;
        list->capacity = cfg.entity_capacity;
        list->count    = 0;
   
        return(list);
    }

    IFB_INTERNAL entity_list* 
    entity_list_stack_create(
        stack& s) {

        const auto& cfg  = config_instance();
        const u32   save = s.save();
       
        auto list = s.push_struct<entity_list>();
        auto ids  = s.push_struct<entity_id>(cfg.entity_capacity);
        
        if (list == NULL || ids == NULL) {
            s.revert(save);
            return(NULL);
        }

        list->ids      = ids;
        list->capacity = cfg.entity_capacity;
        list->count    = 0;
   
        return(list);
    }

    IFB_INTERNAL bool
    entity_list_add(
        entity_list*    el,
        const entity_id id) {

        entity_list_validate(el);
        assert(id != ENTITY_ID_INVALID);

        if (el->count == el->capacity) {
            return(false);
        }

        const bool can_add = !entity_list_contains(el, id);
        if (can_add) {
            el->ids[el->count++] = id;
        }
        return(can_add);
    }

    IFB_INTERNAL bool
    entity_list_remove(
        entity_list*    el,
        const entity_id id) {

        entity_list_validate(el);
        assert(id != ENTITY_ID_INVALID);

        // check if the last element is the id
        if (id == el->ids[el->count - 1]) {
            --el->count;
            return(true);
        }

        // search the list for the id
        for (
            u32 i = 0;
            i < el->count - 1;
            ++i
        ) {
            // if we find it,
            // swap it with the last element,
            // reduce the count and return
            if (id == el->ids[i]) {
                el->ids[i] = el->ids[el->count - 1];
                return(true);
            }
        }

        // we didn't find it
        return(false);
    }

    IFB_INTERNAL void
    entity_list_reset(
        entity_list* el) {

        entity_list_validate(el);
        el->count = 0;
    }

    IFB_INTERNAL void
    entity_list_validate(
        const entity_list* el) {

        assert(el->ids      != NULL);
        assert(el->capacity != 0);
        assert(el->count    <= el->capacity);
    }

    IFB_INTERNAL u32
    entity_list_capacity (
        const entity_list* el) {

        entity_list_validate(el);
        return(el->capacity);
    }

    IFB_INTERNAL u32
    entity_list_count(
        const entity_list* el) {

        entity_list_validate(el);
        return(el->count);
    }

    IFB_INTERNAL bool
    entity_list_contains(
        const entity_list* el,
        const entity_id    id) {

        entity_list_validate(el);
        assert(id != ENTITY_ID_INVALID);

        bool does_contain = false;
        for (
            u32 i = 0;
            i < el->count;
            ++i
        ) {
            if (id == el->ids[i]) {
                does_contain = true;
                break;
            }
        }

        return(does_contain);
    }
    
    IFB_INTERNAL entity_id
    entity_list_index(
        entity_list* el,
        const u32    index) {

        entity_list_validate(el);

        assert(index < el->count);
        return(el->ids[index]);
    }
};
