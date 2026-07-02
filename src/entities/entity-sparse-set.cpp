#pragma once

#include "entities.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCT DEFINITION
    //--------------------------------------------------------------------

    struct entity_sparse_set {
        
        struct {
            entity_id* id;
            u32*       sparse_index;
        } dense;

        struct {
            u32* dense_index;
        } sparse;

        struct {
            u32 sparse;
            u32 dense;
        } capacity;

        u32 count;
    };

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL u32
    entity_sparse_set_memory_requirement(
        const u32 capacity) {

        const ifb_config& cfg             = config_instance();
        const u32         capacity_sparse = (capacity / cfg.sparse_set_max_load_p100);

        const u32 size_struct                  = sizeof(entity_sparse_set);
        const u32 size_dense_data_id           = (capacity        * sizeof(entity_id));
        const u32 size_dense_data_sparse_index = (capacity        * sizeof(u32));
        const u32 size_sparse_data_dense_index = (capacity_sparse * sizeof(u32));

        const u32 size_total = (
            size_struct                  +
            size_dense_data_id           +
            size_dense_data_sparse_index +
            size_sparse_data_dense_index
        );

        return(size_total);
    }

    IFB_INTERNAL entity_sparse_set*
    entity_sparse_set_memory_init(
        const u32 capacity,
        memory&   mem) {

        const ifb_config& cfg             = config_instance();
        const u32         capacity_sparse = (capacity / cfg.sparse_set_max_load_p100);

        const u32 size_struct                  = sizeof(entity_sparse_set);
        const u32 size_dense_data_id           = (capacity        * sizeof(entity_id));
        const u32 size_dense_data_sparse_index = (capacity        * sizeof(u32));
        const u32 size_sparse_data_dense_index = (capacity_sparse * sizeof(u32));

        const u32 size_total = (
            size_struct                  +
            size_dense_data_id           +
            size_dense_data_sparse_index +
            size_sparse_data_dense_index
        );

        assert(
            capacity    != 0          &&
            mem.size    == size_total &&
            mem.address != 0
        );

        auto sparse_set                = (entity_sparse_set*)mem.ptr;
        sparse_set->dense.id           = (entity_id*)((addr)sparse_set                     + size_struct);
        sparse_set->dense.sparse_index =       (u32*)((addr)sparse_set->dense.id           + size_dense_data_id); 
        sparse_set->sparse.dense_index =       (u32*)((addr)sparse_set->dense.sparse_index + size_dense_data_sparse_index);
        sparse_set->capacity.dense     = capacity;
        sparse_set->capacity.sparse    = capacity_sparse;
        sparse_set->count              = 0;
        
        entity_sparse_set_reset(sparse_set);
        
        return(sparse_set);
    }

    IFB_INTERNAL void
    entity_sparse_set_validate(
        const entity_sparse_set* ess) {

        assert(
            ess                     != NULL &&
            ess->dense.id           != NULL &&
            ess->dense.sparse_index != NULL &&
            ess->sparse.dense_index != NULL &&
            ess->capacity.dense     != 0    &&
            ess->capacity.sparse    != 0    &&
            ess->count              <= ess->capacity.dense
        );

    }

    IFB_INTERNAL u32
    entity_sparse_set_capacity_sparse(
        const entity_sparse_set* ess) {

        entity_sparse_set_validate(ess);

        return(ess->capacity.sparse);
    }

    IFB_INTERNAL u32
    entity_sparse_set_capacity_dense(
        const entity_sparse_set* ess) {

        entity_sparse_set_validate(ess);

        return(ess->capacity.dense);
    }

    IFB_INTERNAL bool
    entity_sparse_set_index_of(
        const entity_sparse_set* ess,
        const cchar*             tag_cstr,
        u32&                     index) {


        entity_sparse_set_validate(ess);
        assert(tag_cstr != NULL);

        const entity_tag tag(tag_cstr);
        const entity_id  id = tag.hash();
        const u32        index_sparse_start = (ess->capacity.sparse - 1 & id);

        bool found = false;

        for(
            u32 probe = 0;
                probe < ess->capacity.sparse;
              ++probe
        ) {

            // calculate current idexes
            const u32 index_sparse = (index_sparse_start + probe) & ess->capacity.sparse;
            const u32 index_dense  = ess->sparse.dense_index[index_sparse];  
        
            // the value does not exist
            if (
                index_dense  == INVALID_INDEX ||
                index_sparse == INVALID_INDEX
            ) {
                break;
            }

            // this is a different value, go to the next one
            if (id != ess->dense.id[index_dense]) {
                continue;
            }

            index = index_sparse; 
            break;
        }

        return(found);
    }

    IFB_INTERNAL u32
    entity_sparse_set_insert(
        entity_sparse_set* ess,
        const cchar* tag_cstr) {


        entity_sparse_set_validate(ess);
        assert(tag_cstr != NULL);

        const entity_tag tag               = entity_tag(tag_cstr);
        const entity_id id                 = tag.hash(); 
        const u32       index_sparse_start = ((ess->capacity.sparse - 1) & id);
        const u32       index_dense_new    = ess->count;

        assert(
            id                 != ENTITY_ID_INVALID &&
            index_sparse_start != INVALID_INDEX     &&
            index_sparse_start <  ess->capacity.sparse
        );

        if (index_dense_new == ess->capacity.dense) {
            return(INVALID_INDEX);
        }

        u32 new_sparse_index = 0;
        
        bool did_insert = false;

        for (
            u32 probe = 0;
                probe < ess->capacity.sparse;
              ++probe
        ) {
        
            // calculate current indexes
            const u32 index_sparse = (index_sparse_start + probe) % ess->capacity.sparse;
            const u32 index_dense  = ess->sparse.dense_index[index_sparse]; 

            // if we already have a value here,
            // make sure its not a collision and continue
            if (index_dense != INVALID_INDEX) {
                const bool is_collision = (id == ess->dense.id[index_dense]);
                assert(is_collision);
                continue;
            }

            // we have a free space
            did_insert = true;

            // update the sparse data
            ess->sparse.dense_index [index_sparse] = index_dense_new;
            
            // update the dense_data
            ess->dense.sparse_index [index_dense_new] = index_sparse;
            ess->dense.id           [index_dense_new] = id;

            // update the count and break;
            ++ess->count;
            break;
        }

        return(did_insert);
    }

    IFB_INTERNAL void
    entity_sparse_set_reset(
        entity_sparse_set* ess) {
        
        entity_sparse_set_validate(ess);

        const u32 size_entity_id    = (ess->capacity.dense  * sizeof(entity_id));
        const u32 size_sparse_index = (ess->capacity.dense  * sizeof(u32));
        const u32 size_dense_index  = (ess->capacity.sparse * sizeof(u32));

        (void)memset(ess->dense.id,           0xFF, size_entity_id);
        (void)memset(ess->dense.sparse_index, 0xFF, size_sparse_index);
        (void)memset(ess->dense.id,           0xFF, size_dense_index);
    }


    IFB_INTERNAL void
    entity_sparse_set_remove(
        entity_sparse_set* ess,
        const cchar*       tag_cstr) {

        entity_sparse_set_validate(ess);
        assert(tag_cstr != NULL);

        const entity_tag tag                = entity_tag(tag_cstr);
        const entity_id  id                 = tag.hash();
        const u32        index_sparse_start = ((ess->capacity.sparse - 1) & id);
        
        for (
            u32 probe = 0;
                probe < ess->capacity.sparse;
              ++probe
        ) {
        
            // calculate current indexes
            const u32 index_sparse = (index_sparse_start + probe) % ess->capacity.sparse;
            const u32 index_dense  = ess->sparse.dense_index[index_sparse];
            
            // we should always be working with a valid value
            // that was verified with lookup
            assert(
                index_sparse != INVALID_INDEX &&
                index_dense  != INVALID_INDEX
            );

            // this is a different value, go to the next one
            if (id != ess->dense.id[index_dense]) {
                continue;
            }

            // calculate the last indexes
            const u32 index_sparse_last = ess->count - 1;
            const u32 index_dense_last  = ess->sparse.dense_index[index_sparse_last]; 

            // in order to remove something from the sparse set:
            // 1.) the last and current dense indexes need to be swapped,
            // 2.) the sparse data indexes stay the same, we just clear
            //      the one that's being removed, and point the last
            //     one to the sparse index that was removed 

            ess->dense.sparse_index [index_dense]       = index_dense_last;
            ess->sparse.dense_index [index_sparse_last] = index_dense;

            ess->dense.sparse_index [index_dense_last] = INVALID_INDEX;
            ess->dense.id           [index_dense_last] = ENTITY_ID_INVALID;

            --ess->count;
            break;
        } 
    }
};