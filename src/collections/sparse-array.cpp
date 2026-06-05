#pragma once

#include "ifb.hpp"

#define sa_mask(capacity, key) ((capacity-1) & key)

namespace ifb {

    struct sparse_array {
        struct {
            u32*  dense_hash;
            u32*  dense_index;
            void* sparse_val;
        } array;
        struct {
            u32 key;
            u32 element;
        } size;
        u32 count;
        u32 capacity_sparse;
        u32 capacity_dense;
        f32 max_load_p100;
    };
    

    IFB_API u32
    sparse_array_memory_requirement(
        const u32 capacity,
        const u32 size_key,
        const u32 size_val,
        const f32 max_load_p100) {

        assert(
            capacity      != 0    &&
            size_key      != 0    &&
            size_val      != 0    &&
            max_load_p100 >  0.0f &&            
            max_load_p100 <= 1.0f            
        );

        const u32 count_max = (capacity * max_load);

        const u32 size_struct            = sizeof(sparse_array);
        const u32 size_array_dense_hash  = count_max * sizeof(u32);
        const u32 size_array_dense_index = count_max * sizeof(u32);
        const u32 size_array_sparse_val  = capacity  * size_val;
        
        const u32 size_total = (
            size_struct            +
            size_array_dense_hash  +
            size_array_dense_index +
            size_array_sparse_val
        );

        return(size_total); 
    }

    IFB_API sparse_array*
    sparse_array_memory_init(
        const u32 capacity,
        const u32 size_key,
        const u32 size_val,
        const f32 max_load_p100,
        const u32 mem_size,
        void*     mem_ptr) {

        const u32 mem_req = sparse_array_memory_requirement(
            capacity,
            size_key,
            size_val,
            max_load_p100            
        );


        assert(
            mem_size != 0       &&
            mem_size == mem_req &&
            mem_ptr  != NULL
        );

        zero_memory(mem_ptr, mem_size);

        const u32 dense_count = (capacity    * max_load_p100);
        const u32 dense_size  = (count_dense * sizeof(u32));  

        auto sa = (sparse_array*)mem_ptr;
        sa->array.dense_hash  =  (u32*)((addr)sa) + sizeof(sparse_array);
        sa->array.dense_index =  (u32*)((addr)sa) + dense_size);
        sa->array.sparse_val  = (void*)((addr)sa) + dense_size);
        sa->capacity_sparse   = capacity;
        sa->capacity_dense    = dense_count;
        sa->count             = 0;
        sa->size.element      = size_val;
        sa->size.key          = size_key;
        sa->max_load_p100     = max_load_p100;

        return(sa);
    }

    IFB_API bool
    sparse_array_is_valid(
        const sparse_array* sa) {

        const bool is_valid = (
            sa                    != NULL &&
            sa->array.dense_hash  != NULL &&
            sa->array.dense_index != NULL &&
            sa->array.sparse_val  != NULL &&
            sa->capacity_sparse   != 0    &&
            sa->capacity_dense    != 0    &&
            sa->size.element      != 0    &&
            sa->size.key          != 0    &&
            sa->max_load_p100     >  0.0f &&
            sa->max_load_p100     <= 1.0f &&
            sa->count             <= sa->capacity_dense
        );
        return(is_valid);
    }

    IFB_API void
    sparse_array_assert_valid(
        const sparse_array* sa) {

        assert(
            sparse_array_is_valid(sa);
        )
    }
    
    IFB_API void*
    sparse_array_lookup(
        const sparse_array* sa,
        const cchar8*       key) {

        sparse_array_assert_valid(sa);
        assert(key != NULL);

        const u32 hash  = hash_u32(key, sa->size.key);
        const u32 index = sa_mask(sa->capacity_sparse, hash);
    }

    IFB_API void
    sparse_array_insert(
        const sparse_array* sa,
        const cchar8*       key,
        const void*         val,
        const u32           count) {

        
    }

    IFB_API void
    sparse_array_remove(
        const sparse_array* sa,
        const cchar8*       key,
        const u32           count) {

    }
};