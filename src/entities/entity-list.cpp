#pragma once

#include "entity.hpp"

namespace ifb {

    IFB_INTERNAL entity_list*
    entity_list_create(
        arena* a) {

        entity_mngr_validate();
        assert(a != NULL);
        
        const u32 save     = arena_save(a);
        const u32 capacity = _entity_mngr->capacity.dense; 

        auto* list              = arena_push<entity_list> (a);
        auto  data_id           = arena_push<entity_id>   (a, capacity);
        auto  data_sparse_index = arena_push<u32>         (a, capacity);
        auto  data_dense_index  = arena_push<u32>         (a, capacity);

        const bool did_alloc = (
            list              != NULL &&
            data_id           != NULL &&
            data_sparse_index != NULL &&
            data_dense_index  != NULL
        );

        if (!did_alloc) {
            arena_revert(a,save);
            return(NULL);
        }

        arena_commit(a, save);

        list->data.id           = data_id;
        list->data.sparse_index = data_sparse_index;
        list->data.dense_index  = data_dense_index;
        list->count             = 0;
        return(list);
    }

    IFB_INTERNAL void
    entity_list_validate(
        const entity_list* list) {

        entity_mngr_validate();

        assert(
            list                    != NULL &&
            list->data.id           != NULL &&
            list->data.sparse_index != NULL &&
            list->count             <= _entity_mngr->capacity.dense
        );
    }
};