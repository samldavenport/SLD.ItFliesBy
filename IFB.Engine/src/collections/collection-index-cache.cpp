#include "ifb-engine.hpp"
#include "ifb.hpp"
#include "sld.hpp"
#include <cassert>

namespace ifb {

    struct index_cache {
        bool* index;
        u32   capacity;
    };
   
    inline void
    index_cache_validate(
        const index_cache* cache) {

        assert(cache);
        assert(cache->index    != NULL);
        assert(cache->capacity != 0);
    }

    IFB_ENGINE_API u32
    eng_index_cache_memory_size(
        const u32 capacity) {

        const u32 size_struct = sizeof(index_cache);
        const u32 size_data   = sizeof(bool) * capacity;
        const u32 size_total  = size_struct + size_data;

        return(size_total);
    }

    IFB_ENGINE_API index_cache* 
    eng_index_cache_memory_create(
        const u32     capacity,
        memory& mem) {

        assert(capacity    != 0); 
        assert(mem.size    != 0); 
        assert(mem.address != 0); 

        zero_memory(mem);

        auto cache      = (index_cache*)mem.address;
        cache->capacity = capacity;
        cache->index    = (bool*)(mem.address + sizeof(index_cache));

        return(cache);
    }

    IFB_ENGINE_API index_cache*
    eng_index_cache_arena_create(
        const u32          capacity,
        const hnd_arena arena_hnd) {

        assert(capacity  != 0);
        assert(arena_hnd != INVALID_HANDLE);

        const u32 save = eng_arena_save(arena_hnd);

        auto cache = ifb_eng_arena_push_struct(arena_hnd, index_cache, 1);
        auto data  = ifb_eng_arena_push_struct(arena_hnd, bool, capacity);
   
        if (cache == NULL || data == NULL) {
            eng_arena_revert(arena_hnd, save);
            return(NULL);
        }

        cache->index    = data;
        cache->capacity = capacity;
    
        return(cache);
    }
    
    IFB_ENGINE_API void
    eng_index_cache_reset(
        index_cache* cache) {

        index_cache_validate(cache); 

        for (
            u32 i = 0;
            i < cache->capacity;
            ++i) {

            cache->index[i] = false;
        }
    }

    IFB_ENGINE_API void 
    eng_index_cache_set_index_free(
        index_cache* cache,
        const u32    index) {
    
        index_cache_validate(cache);
        assert(index < cache->capacity);

        cache->index[index] = false;
    }

    IFB_ENGINE_API void 
    eng_index_cache_set_index_used(
        index_cache* cache,
        const u32    index) {

        index_cache_validate(cache);
        
        cache->index[index] = true;
    }

    IFB_ENGINE_API u32
    eng_index_cache_get_next_free(
        index_cache* cache) {

        index_cache_validate(cache);

        u32 index = INVALID_INDEX;
        for (
            u32 i = 0;
            i < cache->capacity;
            ++i) {

            if (!cache->index[i]) {
                cache->index[i] = true;
                index           = i;
                break;
            }
        }

        return(index);
    }
    
    IFB_ENGINE_API u32
    eng_index_cache_count_used(
        const index_cache* cache) {
    
        index_cache_validate(cache);

        u32 count = 0;
        for (
            u32 i = 0;
            i < cache->capacity;
            ++i) {

           if (cache->index[i]) ++count; 
        }

        return(count);
    }

    IFB_ENGINE_API u32
    eng_index_cache_count_free(
        const index_cache* cache) {
    
        index_cache_validate(cache);

        u32 count = 0;
        for (
            u32 i = 0;
            i < cache->capacity;
            ++i) {

           if (!cache->index[i]) ++count; 
        }

        return(count);
    }

    IFB_ENGINE_API u32
    eng_index_cache_capacity(
        const index_cache* cache) {

        index_cache_validate(cache);
        return(cache->capacity);
    } 

    IFB_ENGINE_API bool
    eng_index_cache_is_index_free(
        const index_cache* cache,
        const u32 index) {
    
        index_cache_validate(cache);
        assert(index < cache->capacity);
        return(!cache->index[index]);
    }
};
