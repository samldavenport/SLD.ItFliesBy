#pragma once

#include "ifb-collections.hpp"
#include "map.hpp"

namespace ifb {

    struct map_memory {

    };

    struct map_memory_block {
        map_memory_block* next;
        map_memory_block* prev;
        stack             stack;
    };

    struct map_memory_block_alctr {
        map_memory_block* free;
        map_memory_block* used;
        u32               block_count; 
        u32               block_size; 
    };

};
