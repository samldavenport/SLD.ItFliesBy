#ifndef MAP_INTERNAL_HPP
#define MAP_INTERNAL_HPP

#include "map.hpp"

namespace ifb {

    struct map_mngr {
        map_memory*     memory;
        u32             tiles_per_map;
        f32             tile_unit_size;
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

    struct map_memory : memory {
        map_memory_block_alctr block_alctr;
    };
};

#endif //MAP_INTERNAL_HPP
