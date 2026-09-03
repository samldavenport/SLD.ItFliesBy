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

    class stack {
    
    private:

        memory _mem;
        u32    _pos;
        u32    _save;

    public:

        // instance methods
        void  init       (memory& mem);
        void  validate   (void) const;
        u32   size_total (void) const;
        u32   size_used  (void) const;
        u32   size_free  (void) const;
        void* head       (void) const;
        void* tail       (void) const;
        u32   save       (void);
        void  reset      (void);
        void* push       (const u32 size);
        void  pull       (const u32 size);
        void  revert     (const u32 save);

        template<typename t>
        t* push_struct(const u32 count = 1);
    };

    //--------------------------------------------------------------------
    // SPARSE SET
    //--------------------------------------------------------------------

    template<typename t>
    class sparse_set {

    private:

        struct {
            struct {
                u32* sparse_index;
                u32* hash;
            } dense;
            struct {
                u32* dense_index;
                t*   val;
            } sparse;
        } _data;

        struct {
            u32 sparse;
            u32 dense;
        } _capacity;

        u32 _key_size;
        u32 _count;

    public:

        // static methods
        static u32  memory_requirement (const u32 capacity, const u32 key_size, const f32 max_load_p100);

        // instance methods
        void init            (const u32 capacity, const u32 key_size, const f32 max_load_p100, memory& mem);
        u32  capacity_sparse (void) const;
        u32  capacity_dense  (void) const;
        u32  key_size        (void) const;
        u32  count           (void) const;
        void validate        (void) const;
        void reset           (void);
        bool insert          (const cchar* key, const t& val);
        void update          (const cchar* key, const t& val);
        bool lookup          (const cchar* key, t&       val) const;
        void remove          (const cchar* key);
    };
    
    //--------------------------------------------------------------------
    // INDEX CACHE 
    //--------------------------------------------------------------------

    class index_cache {

    private:

        bool* _index;
        u32   _capacity;

    public:

        void memory_init    (const u32 capacity, bool* index_array); 
        void stack_init     (const u32 capacity, stack& s);
        void reset          (void);     
        void set_index_free (const u32 index);
        void set_index_used (const u32 index);
        void validate       (void)            const;
        u32  get_next_free  (void)            const;
        u32  count_used     (void)            const;
        u32  count_free     (void)            const;
        u32  capacity       (void)            const;
        bool is_index_free  (const u32 index) const;
    };
    
    //--------------------------------------------------------------------
    // ENTITY ID LIST 
    //--------------------------------------------------------------------

    struct entity_list;

    entity_list* entity_list_arena_create (const arena_handle arena_hnd);
    entity_list* entity_list_stack_create (stack& s);
    bool         entity_list_add          (entity_list* el, const entity_id id);
    bool         entity_list_remove       (entity_list* el, const entity_id id);
    void         entity_list_reset        (entity_list* el);
    void         entity_list_validate     (const entity_list* el);
    u32          entity_list_capacity     (const entity_list* el);
    u32          entity_list_count        (const entity_list* el);
    bool         entity_list_contains     (const entity_list* el, const entity_id);
    entity_id    entity_list_index        (entity_list* el, const u32 index);

    //--------------------------------------------------------------------
    // COMPONENT TABLE 
    //--------------------------------------------------------------------
   
    template<typename t>
    class component_table {
    
    private:
        t* _cmpnt;

    public:
       
        void stack_init (stack& s);
        void lookup     (const u32 sparse_index, t&       cmpnt);
        void update     (const u32 sparse_index, const t& cmpnt);
    };
};

#endif //IFB_COLLECTIONS_HPP
