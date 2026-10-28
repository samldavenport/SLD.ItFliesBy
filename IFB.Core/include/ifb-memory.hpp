#ifndef IFB_MEMORY_HPP
#define IFB_MEMORY_HPP

#include "ifb.hpp"
#include "ifb-types.hpp"
#include "ifb-platform.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct memory_mngr;
    struct reservation;
    struct arena_allocator;
    struct stack_allocator;
    struct block_allocator;
    struct global_allocator;
    struct arena;
    struct stack;

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct block_allocator {
        addr start;
        u32  block_size;
        u32  block_count_total;
        u32  block_mask_hi;
        u32  block_mask_lo;
    };

    struct block_memory {
        block_allocator* alctr;
        u32              id;
    };
};

#endif //IFB_MEMORY_HPP
