#ifndef COLLECTIONS_INTERNAL_HPP
#define COLLECTIONS_INTERNAL_HPP

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    class entity_id_list {

    private:

        entity_id* _array;
        u32        _capacity;
        u32        _count;

    public:

        void validate    (void);
        bool arena_init  (arena* a);
        bool add         (const entity_id id);
        bool remove      (const entity_id id);
        void reset       (void);
        u32  capacity    (void);
        u32  count       (void);

        inline const entity_id&
        operator[] (const u32 index) {
           
            assert(index < _count);
            return(_array[index]);
        }

    };

};

#endif //COLLECTIONS_INTERNAL_HPP
