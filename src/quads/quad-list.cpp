#pragma once

#include "quad.hpp"

namespace ifb {

    IFB_INTERNAL quad_list*
    quad_list_create(
        arena* a) {

        assert(a);

        auto list  = arena_push<quad_list>(a);
        auto array = arena_push<entity_id>(a, _quad_mngr->capacity);
        assert(list != NULL && array != NULL);
    
        list->array = array;
        list->count = 0;

        return(list);
    }

    IFB_INTERNAL void
    quad_list_validate(
        const quad_list* ql) {

        assert(
            ql        != NULL &&
            ql->array != NULL &&
            ql->count <= _quad_mngr->capacity
        );
    }

    IFB_INTERNAL bool
    quad_list_add(
        quad_list*       ql,
        const entity_id* quad_id,
        const u32        count) {

        quad_list_validate(ql);
        assert(quad_id != NULL && count != 0);


        const u32 count_free = (_quad_mngr->capacity - ql->count); 
        const u32 count_to_add = (count >= count_free)
            ? count_free
            : count;

        if (count_to_add == 0) {
            return(false);
        }

        for (
            u32 index = 0;
                index < count_to_add;
              ++index){

            u32& list_index = ql->count;  
            ql->array[list_index] = quad_id[index];
            ++list_index;
        }

        ql->count += count_to_add;

        return(true);
    }

    IFB_INTERNAL void
    quad_list_remove(
        quad_list*       ql,
        const entity_id* quad_id,
        const u32        count) {

        u32 count_removed = 0;
        
        for (
            u32 remove_index = 0;
                remove_index < count;
              ++remove_index
        ) {
            for (
                u32 list_index = 0;
                    list_index < ql->count;
                  ++list_index
            ) {

                if (quad_id[remove_index] != ql->array[list_index]) {
                    continue;
                }

                const u32 list_index_last = (ql->count - 1);

                if (list_index != list_index_last) {
                    ql->array[list_index] = ql->array[list_index_last];
                }
                ++count_removed;
                --ql->count;
            }
        }
    
        assert(count_removed == count);
    }

};