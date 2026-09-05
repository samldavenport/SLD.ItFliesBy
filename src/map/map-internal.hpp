#ifndef MAP_INTERNAL_HPP
#define MAP_INTERNAL_HPP

#include "map.hpp"

namespace ifb {

    struct map_mngr {
        stack           mem_stack;
        map_tile_table* tbl_tiles;
        map_table*      tbl_map;
        u32             map_capacity;
        u32             tiles_per_map;
        f32             tile_unit_size;
    };
};

#endif //MAP_INTERNAL_HPP
