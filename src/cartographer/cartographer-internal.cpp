#pragma once

#include "cartographer.hpp"

namespace ifb {

    struct cartographer {
        map* first_map; 
    };

    struct map {
        map*  next;
        map*  prev;
        tile* root_tile;
    };

    struct tile{
        map*  map; 
        tile* tile_north;
        tile* tile_south;
        tile* tile_east;
        tile* tile_west;
        wall* wall_north;
        wall* wall_south;
        wall* wall_east;
        wall* wall_west;
    };

    struct atlas {
        atlas_key* keys;
        byte*      image_data_ptr;
        u32        image_data_size;
        u32        image_count;
    };

    struct atlas_key {
        atlas* atlas;
        u32    index;
        u32    unit_x;
        u32    unit_y;
        u32    unit_length;
        u32    unit_width;
    };

    enum cartographer_memory_block_type_e {
        cartographer_memory_block_type_e_map       = 0,
        cartographer_memory_block_type_e_tile      = 1,
        cartographer_memory_block_type_e_wall      = 2,
        cartographer_memory_block_type_e_atlas     = 3,
        cartographer_memory_block_type_e_atlas_key = 4,
    };

    struct cartographer_memory_block {
        u32 block_index; 
        u32 block_type;
        union {
            map*       map;
            tile*      tile;
            wall*      wall;
            atlas*     atlas;
            atlas_key* atlas_key;
        } data;
    };
};
