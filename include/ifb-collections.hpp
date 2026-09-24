#ifndef IFB_COLLECTIONS_HPP
#define IFB_COLLECTIONS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STACK
    //--------------------------------------------------------------------
      
    struct stack;

    u32     stack_memory_requirement (const u32 capacity);
    stack*  stack_memory_create      (const u32 capacity, memory& mem);
    stack*  stack_memory_create      (memory& mem);
    stack*  stack_arena_create       (const u32 capacity, const hnd_arena arena_hnd);
    u32     stack_get_capacity       (const stack* s);
    u32     stack_get_position       (const stack* s);
    void*   stack_get_head           (const stack* s);
    void*   stack_get_tail           (const stack* s);
    u32     stack_save               (stack* s);
    void    stack_reset              (stack* s);
    void*   stack_push               (stack* s, const u32 size);
    void    stack_pull               (stack* s, const u32 size);
    void    stack_revert             (stack* s, const u32 save);
    
    //--------------------------------------------------------------------
    // INDEX CACHE 
    //--------------------------------------------------------------------

    struct index_cache;

    u32          index_cache_memory_size    (const u32 capacity);
    index_cache* index_cache_memory_create  (const u32 capacity, memory& mem); 
    index_cache* index_cache_arena_create   (const u32 capacity, const hnd_arena arena_hnd);
    void         index_cache_reset          (index_cache* cache);     
    void         index_cache_set_index_free (index_cache* cache, const u32 index);
    void         index_cache_set_index_used (index_cache* cache, const u32 index);
    u32          index_cache_get_next_free  (index_cache* cache);
    u32          index_cache_count_used     (const index_cache* cache);
    u32          index_cache_count_free     (const index_cache* cache);
    u32          index_cache_capacity       (const index_cache* cache);
    bool         index_cache_is_index_free  (const index_cache* cache, const u32 index);
    
    //--------------------------------------------------------------------
    // ENTITY ID LIST 
    //--------------------------------------------------------------------

    struct entity_list;

    u32          entity_list_mem_req       (void);
    entity_list* entity_list_memory_create (const memory& mem);
    entity_list* entity_list_arena_create  (const hnd_arena arena_hnd);
    bool         entity_list_add           (entity_list* el, const entity_id id);
    bool         entity_list_remove        (entity_list* el, const entity_id id);
    void         entity_list_reset         (entity_list* el);
    void         entity_list_validate      (const entity_list* el);
    u32          entity_list_capacity      (const entity_list* el);
    u32          entity_list_count         (const entity_list* el);
    bool         entity_list_contains      (const entity_list* el, const entity_id);
    entity_id    entity_list_index         (const entity_list* el, const u32 index);
    void         entity_list_lookup        (const entity_list* el, const u32 index, entity& e);

};

#endif //IFB_COLLECTIONS_HPP
