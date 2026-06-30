#pragma once

#include "ifb-collections.hpp"

#define SPARSE_SET_FUNC template<typename t> auto sparse_set<t>::

namespace ifb {

    SPARSE_SET_FUNC
    memory_requirement (
        const u32 capacity,
        const u32 key_size,
        const f32 max_load_p100) -> u32 {

        return(0);
    }

    SPARSE_SET_FUNC
    memory_init(
        const u32 capacity,
        const u32 key_size,
        const f32 max_load_p100,
        memory&   mem) -> void {

    }


    SPARSE_SET_FUNC
    capacity_sparse(
        void) const -> u32 {

        return(_capacity.sparse);
    }

    SPARSE_SET_FUNC
    capacity_dense(
        void) const -> u32 {

        return(_capacity.dense);
    }

    SPARSE_SET_FUNC
    key_size(
        void) const -> u32 {

        return(_key_size);
    }

    SPARSE_SET_FUNC
    count(
        void) const -> u32 {

        return(_count);
    }

    SPARSE_SET_FUNC 
    validate(
        void) const -> void {

        assert(
            _data.dense.sparse_index != NULL             &&
            _data.dense.hash         != NULL             &&
            _data.sparse.dense_index != NULL             &&
            _data.sparse.val         != NULL             &&
            _capacity.sparse         != 0                &&
            _capacity.dense          != 0                &&
            _capacity.dense          <= _capacity.sparse &&
            _key_size                != 0                &&
            _count                   <= _capacity.dense
        );
    }

    SPARSE_SET_FUNC 
    reset(
        void) -> void {

        const u32 sparse_size = sizeof(u32) * _count;

        memset(_data.dense.sparse_index, 0xFF, sparse_size);
        memset(_data.dense.hash,         0xFF, sparse_size);
        memset(_data.sparse.dense_index, 0xFF, sparse_size);

        _count = 0;
    }

    SPARSE_SET_FUNC
    insert(
        const cchar* key,
        const t&     val) -> bool {

        validate();
        assert(key != NULL);

        const u32 hash               = hash_u32((void*)key, _key_size);
        const u32 index_sparse_start = ((_capacity.sparse - 1) & hash);
        const u32 index_dense_new    = _count;

        assert(
            hash               != INVALID_INDEX &&
            index_sparse_start != INVALID_INDEX &&
            index_sparse_start <  _capacity.sparse;
        );

        if (index_dense_new == _capacity.sparse) {
            return(INVALID_INDEX);
        }

        bool did_insert = false;
        
        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe
        ) {
        
            // calculate current indexes
            const u32 index_sparse = (index_sparse_start + probe) % _capacity.sparse;
            const u32 index_dense  = _data.sparse.dense_index[index_sparse];

            // if we already have a value here,
            // make sure its not a collision and continue
            if (index_dense != INVALID_INDEX) {
                const bool is_collision = (hash == _data.dense.hash[index_dense]);
                assert(is_collision);
                continue;
            }

            // we have a free space
            did_insert = true;

            // update the sparse data
            _data.sparse.dense_index [index_sparse] = index_dense_new;
            _data.sparse.val         [index_sparse] = val;
            
            // update the dense_data
            _data.dense.sparse_index [index_dense_new] = index_sparse;
            _data.dense.hash         [index_dense_new] = hash;

            // update the count and break;
            ++_count;
            break;
        }

        return(did_insert); 
    }

    SPARSE_SET_FUNC
    update(
        const cchar* key,
        const t&     val) -> void {

        validate();
        assert(key != NULL);

        const u32 hash               = hash_u32((void*)key, _key_size);
        const u32 index_sparse_start = ((_capacity.sparse - 1) & hash);

        assert(
            hash               != INVALID_INDEX &&
            index_sparse_start != INVALID_INDEX &&
            index_sparse_start <  _capacity.sparse;
        );

        
        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe
        ) {
        
            // calculate current indexes
            const u32 index_sparse = (index_sparse_start + probe) % _capacity.sparse;
            const u32 index_dense  = _data.sparse.dense_index[index_sparse];

            // we should always be working with a valid value
            // that was verified with lookup
            assert(
                index_sparse != INVALID_INDEX &&
                index_dense  != INVALID_INDEX &&
            );

            // this is a different value, go to the next one
            if (hash != _data.dense.hash[index_dense]) {
                continue;
            }

            // update the value we found and break
            _data.sparse.val[index_sparse] = val;
            break;
        }
    }

    SPARSE_SET_FUNC
    lookup(
        const cchar* key,
        t&           val) const -> bool {

        validate();
        assert(key != NULL);

        const u32 hash               = hash_u32((void*)key, _key_size);
        const u32 index_sparse_start = ((_capacity.sparse - 1) & hash);

        assert(
            hash               != INVALID_INDEX &&
            index_sparse_start != INVALID_INDEX &&
        );

        bool found = false;

        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe
        ) {
        
            // calculate current indexes
            const u32 index_sparse = (index_sparse_start + probe) % _capacity.sparse;
            const u32 index_dense  = _data.sparse.dense_index[index_sparse];

            // the value does not exist
            if (
                index_dense  == INVALID_INDEX ||
                index_sparse == INVALID_INDEX
            ) {
                break;
            }

            // this is a different value, go to the next one
            if (hash != _data.dense.hash[index_dense]) {
                continue;
            }

            // update the value we found and break
            _data.sparse.val[index_sparse] = val;
            break;
        }
    }

    SPARSE_SET_FUNC
    remove(
        const cchar* key) -> void  {

        validate();
        assert(key != NULL);

        const u32 hash               = hash_u32((void*)key, _key_size);
        const u32 index_sparse_start = ((_capacity.sparse - 1) & hash);
        
        for (
            u32 probe = 0;
                probe < sa->capacity_sparse;
              ++probe
        ) {
        
            // calculate current indexes
            const u32 index_sparse = (index_sparse_start + probe) % _capacity.sparse;
            const u32 index_dense  = _data.sparse.dense_index[index_sparse];

            // we should always be working with a valid value
            // that was verified with lookup
            assert(
                index_sparse != INVALID_INDEX &&
                index_dense  != INVALID_INDEX &&
            );

            // this is a different value, go to the next one
            if (hash != _data.dense.hash[index_dense]) {
                continue;
            }

            // update the indexes
            _data.sparse.dense_index [index_sparse] = INVALID_INDEX;
            _data.dense.sparse_index [index_dense] = INVALID_INDEX;
            _data.dense.hash         [index_dense] = INVALID_INDEX;

            // update the count and break;
            --_count;
            break;
        }
    }

};