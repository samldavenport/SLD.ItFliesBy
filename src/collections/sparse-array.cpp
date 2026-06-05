#pragma once

#include "ifb.hpp"

#define sa_mask(sparse_capacity, hash) ((sparse_capacity-1) & hash)

namespace ifb {

    struct sparse_element {
        void* val;
        u32   hash;
        u32   dense_index;
        u32   sparse_index;
        u32   val_size;
    };

    struct sparse_array {
        struct {
            struct {
                u32* sparse_index;
                u32* hash;
            } dense;
            struct {
                u32*  dense_index;
                void* val;
            } sparse;
        } data;
        u32 size_val;
        u32 size_key;
        u32 capacity_sparse;
        u32 capacity_dense;
        u32 count;
        f32 max_load_p100;
    };

    IFB_API u32
    sparse_array_memory_requirement (
        const u32 capacity,
        const u32 val_size,
        const f32 max_load_p100) {

        assert(
            capacity      != 0    &&
            val_size      != 0    &&
            max_load_p100 >  0.0f &&            
            max_load_p100 <= 1.0f
        );

        const u32 size_struct = sizeof(sparse_array);
        const u32 count_dense = (u32)((f32)capacity * max_load_p100);
        const u32 size_dense  = count_dense * sizeof(u32) * 2;
        const u32 size_sparse = capacity    * (val_size + sizeof(u32));
        const u32 size_total  = size_struct + size_dense + size_sparse;
    
        return(size_total);
    }

    IFB_API sparse_array*
    sparse_array_memory_init(
        const u32 capacity,
        const u32 val_size,
        const f32 max_load_p100,
        const u32 key_size,
        const u32 mem_size, 
        void*     mem_ptr) {

        const u32 mem_req = sparse_array_memory_requirement(
            capacity,
            val_size,
            max_load_p100
        );

        assert(
            mem_req  != 0       &&
            mem_size != 0       &&
            mem_size == mem_req &&
            mem_ptr  != NULL
        );

        // clear memory
        zero_memory(mem_ptr, mem_size);

        // calculate sizes
        const u32 dense_capactiy    = (f32)capacity  * max_load_p100;
        const u32 dense_size        = dense_capactiy * sizeof(u32);
        const u32 sparse_index_size = capacity       * sizeof(u32);

        // cast pointers and initialize
        auto sa = (sparse_array*)mem_ptr;
        sa->data.dense.sparse_index =  (u32*)((addr)sa + sizeof(sparse_array));
        sa->data.dense.hash         =  (u32*)((addr)sa->data.dense.sparse_index + dense_size);
        sa->data.sparse.dense_index =  (u32*)((addr)sa->data.dense.hash         + dense_size);
        sa->data.sparse.val         = (void*)((addr)sa->data.sparse.dense_index + sparse_index_size);
        sa->size_val                = val_size;
        sa->size_key                = key_size;
        sa->capacity_sparse         = capacity;
        sa->capacity_dense          = dense_capactiy;
        sa->count                   = 0;
        sa->max_load_p100           = max_load_p100;

        // set indexes and hashes to invalid
        memset((void*)sa->data.dense.sparse_index, 0xFF, dense_size);
        memset((void*)sa->data.dense.hash,         0xFF, dense_size);
        memset((void*)sa->data.sparse.dense_index, 0xFF, sparse_index_size);
    
        return(sa);
    }

    IFB_API bool
    sparse_array_is_valid(
        const sparse_array* sa) {

        const bool is_valid = (
            sa != NULL &&
            sa->data.dense.sparse_index != NULL               &&
            sa->data.dense.hash         != NULL               &&
            sa->data.sparse.dense_index != NULL               &&
            sa->data.sparse.val         != NULL               &&
            sa->size_val                != 0                  &&
            sa->size_key                != 0                  &&
            sa->capacity_sparse         != 0                  &&
            sa->capacity_dense          != 0                  &&
            sa->count                   <  sa->capacity_dense &&
            sa->max_load_p100           >  0.0f               &&
            sa->max_load_p100           <= 1.0f
        );
        return(is_valid);
    }

    IFB_API void
    sparse_array_assert_valid(
        const sparse_array* sa) {

        const bool is_valid = sparse_array_is_valid(sa);
        assert(is_valid);
    }

    IFB_API void*
    sparse_array_lookup(
        const sparse_array* sa,
        const cchar8*       key) {

        assert(
            sparse_array_is_valid(sa) &&
            key != NULL
        );

        const u32 hash               = hash_u32((void*)key, sa->size_key);
        const u32 sparse_index_start = sa_mask(sa->capacity_sparse, hash);
         
        assert(
            hash               != INVALID_INDEX &&
            sparse_index_start != INVALID_INDEX &&
            sparse_index_start <  sa->capacity_sparse
        );

        void* val = NULL;

        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe) {

            // calculate indexes and offset
            const u32 index_sparse = (sparse_index_start + probe) % sa->capacity_sparse;
            const u32 index_dense  = sa->data.sparse.dense_index[index_sparse]; 
            const u32 val_offset   = (sa->size_val * index_sparse);

            // the first invalid index we hit means it doesn't exist
            if (index_dense == INVALID_INDEX) {
                break;
            }

            // get the current hash
            assert(index_dense < sa->capacity_dense);
            const u32 curr_hash = sa->data.dense.hash[index_dense];
            
            // if the hashes match, we have found a value
            if (curr_hash == hash) {
                val = (void*)((addr)sa->data.sparse.val + val_offset);
                break;
            }
        }

        return(val);
    }

    IFB_API bool
    sparse_array_insert(
        const sparse_array* sa,
        const cchar8*       key,
        const void*         val) {

        const bool is_valid = (
            sparse_array_is_valid(sa) &&
            key   != NULL             &&
            val   != NULL
        );
        assert(is_valid);

        const u32 hash               = hash_u32((void*)key, sa->size_key);
        const u32 sparse_index_start = sa_mask(sa->capacity_sparse, hash);
        const u32 dense_index_new    = sa->count;

        assert(
            hash               != INVALID_INDEX &&
            sparse_index_start != INVALID_INDEX &&
            sparse_index_start <  sa->capacity_sparse
        );

        if (dense_index_new == sa->capacity_dense) {
            return(false);
        } 

        bool did_insert = false;

        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe
        ) {
        
            const u32 index_sparse     = (sparse_index_start + probe) % sa->capacity_sparse;
            const u32 index_dense_curr = sa->data.sparse.dense_index[index_sparse]; 

            // if we already have a value here,
            // make sure its not a collision and continue
            if (index_dense_curr != INVALID_INDEX) {
                const bool is_collision = (hash == sa->data.dense.hash[index_dense_curr]);
                assert(!is_collision);
                continue;
            }

            // we have a free space
            did_insert = true;

            // copy the value
            const u32 val_offset = (index_sparse * sa->size_val);
            void*     val_dst    = (void*)((addr)sa->data.sparse.val + val_offset); 
            memmove(val_dst, val, sa->size_val);

            // set the indexes and hash and break
            sa->data.sparse.dense_index [index_sparse]    = dense_index_new;
            sa->data.dense.hash         [dense_index_new] = hash;
            sa->data.dense.sparse_index [dense_index_new] = index_sparse;
            break;
        }

        return(did_insert);
    }

    IFB_API bool
    sparse_array_remove(
        sparse_array* sa,
        const cchar8*       key) {

        const bool is_valid = (
            sparse_array_is_valid(sa) &&
            key != NULL
        );
        assert(is_valid);

        if (sa->count == 0) {
            return(false);
        }

        const u32 hash               = hash_u32((void*)key, sa->size_key);
        const u32 sparse_index_start = sa_mask(sa->capacity_sparse, hash);
        const u32 dense_index_last   = (sa->count - 1); 

        assert(
            hash               != INVALID_INDEX &&
            sparse_index_start != INVALID_INDEX &&
            sparse_index_start <  sa->capacity_sparse
        );


        bool did_remove = false;

        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe
        ) {

            // get the indexes
            const u32 index_sparse = (sparse_index_start + probe) % sa->capacity_sparse;
            const u32 index_dense  = sa->data.sparse.dense_index[index_sparse]; 

            // the first invalid index we hit means it doesn't exist
            if (index_dense == INVALID_INDEX) {
                break;
            }

            // get the current hash
            assert(index_dense < sa->capacity_dense);
            const u32 curr_hash = sa->data.dense.hash[index_dense];
            
            // continue if its not a match 
            if (curr_hash != hash) {
                continue;
            }

            // we have a match
            did_remove = true;

            // if we have only one element or its the last element,
            // clear the dense element, reduce the count, and we're done
            if (sa->count == 1 || index_dense == dense_index_last) {
                sa->count = 0;
                sa->data.dense.hash         [index_dense] = INVALID_INDEX;
                sa->data.dense.sparse_index [index_dense] = INVALID_INDEX;
                break;
            }            
            
            // get the index of the last sparse element
            const u32 index_sparse_last = sa->data.dense.sparse_index[dense_index_last];
            
            // move the last sparse value to the current one
            void* val_dst = (void*)((addr)sa->data.sparse.val + (sa->size_val * index_sparse_last)); 
            void* val_src = (void*)((addr)sa->data.sparse.val + (sa->size_val * index_sparse)); 
            memmove(val_dst, val_src, sa->size_val);

            // update the dense index of the last sparse value
            sa->data.sparse.dense_index[index_sparse_last] = index_dense;

            // update the current dense data then clear the last dense data
            sa->data.dense.hash         [index_dense]      = sa->data.dense.hash         [dense_index_last];
            sa->data.dense.sparse_index [index_dense]      = sa->data.dense.sparse_index [dense_index_last];
            sa->data.dense.hash         [dense_index_last] = INVALID_INDEX;
            sa->data.dense.sparse_index [dense_index_last] = INVALID_INDEX;

            // update the count and break;
            --sa->count;
            break;
        }

        return(did_remove);
    }

};