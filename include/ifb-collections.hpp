#ifndef IFB_COLLECTIONS_HPP
#define IFB_COLLECTIONS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ARRAY LIST
    //--------------------------------------------------------------------

    template<typename t>
    class array_list {
    
    private:
        
        t*  _elmnt;
        u32 _capacity;
        u32 _count;

    public:

        void init      (t* elmnts, const u32 capacity);
        void validate  (void)                        const;
        u32  capacity  (void)                        const;
        u32  count     (void)                        const;
        bool is_full   (void)                        const;
        bool index_of  (const t& elmnt, u32& index)  const;
        bool contains  (const t& elmnt)              const;
        void reset     (void);
        t&   get       (const u32 index);
        bool add       (const t&  elmnt);
        void remove    (const t&  elmnt);
        void remove_at (const u32 index);

        inline t& operator[] (const u32 index) {
            assert(index < _count);
            return(_elmnt[index]);
        }
    };

    //--------------------------------------------------------------------
    // STACK
    //--------------------------------------------------------------------
      
    struct stack;

    u32     stack_memory_requirement (const u32 capacity);
    stack*  stack_memory_create      (const u32 capacity, memory& mem);
    stack*  stack_memory_create      (memory& mem);
    stack*  stack_arena_create       (const u32 capacity, const arena_handle arena_hnd);
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
    index_cache* index_cache_arena_create   (const u32 capacity, const arena_handle arena_hnd);
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
    entity_list* entity_list_arena_create  (const arena_handle arena_hnd);
    bool         entity_list_add           (entity_list* el, const entity_id id);
    bool         entity_list_remove        (entity_list* el, const entity_id id);
    void         entity_list_reset         (entity_list* el);
    void         entity_list_validate      (const entity_list* el);
    u32          entity_list_capacity      (const entity_list* el);
    u32          entity_list_count         (const entity_list* el);
    bool         entity_list_contains      (const entity_list* el, const entity_id);
    entity_id    entity_list_index         (entity_list* el, const u32 index);

    //--------------------------------------------------------------------
    // COMPONENT TABLE 
    //--------------------------------------------------------------------
   
    template<typename t>
    class component_table {
    
    private:
        t* _cmpnt;

    public:
       
        void stack_init (stack* s);
        void lookup     (const u32 sparse_index, t&       cmpnt);
        void update     (const u32 sparse_index, const t& cmpnt);
    };
};

#endif //IFB_COLLECTIONS_HPP
