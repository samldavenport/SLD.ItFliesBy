#ifndef COLLECTIONS_INTERNAL_HPP
#define COLLECTIONS_INTERNAL_HPP

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
        bool             add                  (const u32 dense_index);
        bool             remove               (const u32 dense_index);
        void             reset                (void);
        void             validate             (void)                  const;
        u32              capacity             (void)                  const;
        u32              count                (void)                  const;
        bool             contains             (const u32 dense_index) const;
        entity_id        get_entity_id        (const u32 index)       const;
        entity_archetype get_entity_archetype (const u32 index)       const;
        u32              get_dense_index      (const u32 index)       const;
        u32              get_sparse_index     (const u32 index)       const;
        const cchar*     get_tag              (const u32 index)       const;
    };
};

#endif //COLLECTIONS_INTERNAL_HPP
