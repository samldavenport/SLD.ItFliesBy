#ifndef ENG_COLLECTIONS_HPP
#define ENG_COLLECTIONS_HPP

#include "ifb-collections.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STACK
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32     stack_memory_requirement (const u32 capacity);
    IFB_ENGINE_API stack*  stack_memory_create      (const u32 capacity, memory& mem);
    IFB_ENGINE_API stack*  stack_memory_create      (memory& mem);
    IFB_ENGINE_API stack*  stack_arena_create       (const u32 capacity, const hnd_arena arena_hnd);
    IFB_ENGINE_API u32     stack_get_capacity       (const stack* s);
    IFB_ENGINE_API u32     stack_get_position       (const stack* s);
    IFB_ENGINE_API u32     stack_get_size_free      (const stack* s);
    IFB_ENGINE_API void*   stack_get_head           (const stack* s);
    IFB_ENGINE_API void*   stack_get_tail           (const stack* s);
    IFB_ENGINE_API u32     stack_save               (stack* s);
    IFB_ENGINE_API void    stack_reset              (stack* s);
    IFB_ENGINE_API void*   stack_push               (stack* s, const u32 size);
    IFB_ENGINE_API void    stack_pull               (stack* s, const u32 size);
    IFB_ENGINE_API void    stack_revert             (stack* s, const u32 save);

    //--------------------------------------------------------------------
    // INDEX CACHE
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32          index_cache_memory_size    (const u32 capacity);
    IFB_ENGINE_API index_cache* index_cache_memory_create  (const u32 capacity, memory& mem); 
    IFB_ENGINE_API index_cache* index_cache_arena_create   (const u32 capacity, const hnd_arena arena_hnd);
    IFB_ENGINE_API void         index_cache_reset          (index_cache* cache);     
    IFB_ENGINE_API void         index_cache_set_index_free (index_cache* cache, const u32 index);
    IFB_ENGINE_API void         index_cache_set_index_used (index_cache* cache, const u32 index);
    IFB_ENGINE_API u32          index_cache_get_next_free  (index_cache* cache);
    IFB_ENGINE_API u32          index_cache_count_used     (const index_cache* cache);
    IFB_ENGINE_API u32          index_cache_count_free     (const index_cache* cache);
    IFB_ENGINE_API u32          index_cache_capacity       (const index_cache* cache);
    IFB_ENGINE_API bool         index_cache_is_index_free  (const index_cache* cache, const u32 index);

    //--------------------------------------------------------------------
    // ENTITY ID LIST
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32          entity_list_mem_req          (void);
    IFB_ENGINE_API entity_list* entity_list_memory_create    (const memory& mem);
    IFB_ENGINE_API entity_list* entity_list_arena_create     (const hnd_arena arena_hnd);
    IFB_ENGINE_API bool         entity_list_add              (entity_list* el, const entity_id id);
    IFB_ENGINE_API bool         entity_list_remove           (entity_list* el, const entity_id id);
    IFB_ENGINE_API void         entity_list_reset            (entity_list* el);
    IFB_ENGINE_API void         entity_list_validate         (const entity_list* el);
    IFB_ENGINE_API bool         entity_list_is_null_or_empty (const entity_list* el);
    IFB_ENGINE_API u32          entity_list_capacity         (const entity_list* el);
    IFB_ENGINE_API u32          entity_list_count            (const entity_list* el);
    IFB_ENGINE_API bool         entity_list_contains         (const entity_list* el, const entity_id);
    IFB_ENGINE_API entity_id    entity_list_index            (const entity_list* el, const u32 index);
    IFB_ENGINE_API void         entity_list_lookup           (const entity_list* el, const u32 index, entity& e);
};

#endif //ENG_COLLECTIONS_HPP
