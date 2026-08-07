#ifndef COLLECTIONS_INTERNAL_HPP
#define COLLECTIONS_INTERNAL_HPP

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENTITY ID LIST 
    //--------------------------------------------------------------------
    
    class entity_list {
        
    private:
        
        entity_id* _ids;
        u32        _capacity;
        u32        _count;

    public:

        bool             arena_init           (arena* a);
        bool             stack_init           (stack& s);
        bool             add                  (const entity_id id);
        bool             remove               (const entity_id id);
        void             reset                (void);
        void             validate             (void)                  const;
        u32              capacity             (void)                  const;
        u32              count                (void)                  const;
        bool             contains             (const entity_id id) const;

        inline entity_id
        operator[](const u32 index) {
            return(_ids[index]);
        }
    };
    
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

#endif //COLLECTIONS_INTERNAL_HPP
