#pragma once

#include "cartographer.hpp"
#include "ifb-collections.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct cartographer_block_allocator; 
    struct cartographer_memory_block;

    //--------------------------------------------------------------------
    // CARTOGRAPHER CONTEXT 
    //--------------------------------------------------------------------
    
    struct cartographer {
        cartographer_block_allocator* block_alctr;
        map*                          first_map; 
    };

    //--------------------------------------------------------------------
    // MAP 
    //--------------------------------------------------------------------

    struct map {
        map*  next;
        map*  prev;
        tile* root_tile;
        tile* tile_list;
    };

    //--------------------------------------------------------------------
    // WALL 
    //--------------------------------------------------------------------

    enum wall_direction_e {
        wall_direction_e_north_south = 0,
        wall_direction_e_east_west   = 1
    };

    using wall_direction = u32;

    struct wall {
        union {
            tile* tile_north;
            tile* tile_east;
        };
        union {
            tile* tile_south;
            tile* tile_west;
        };
        wall_direction dir;
        color_rgba_u32 color;
    };

    //--------------------------------------------------------------------
    // TILE 
    //--------------------------------------------------------------------

    struct tile{
        map*           map; 
        tile*          next;
        tile*          prev;
        tile*          tile_north;
        tile*          tile_south;
        tile*          tile_east;
        tile*          tile_west;
        wall*          wall_north;
        wall*          wall_south;
        wall*          wall_east;
        wall*          wall_west;
        color_rgba_u32 color;
        u32            unit_length;
        u32            unit_width;
        u32            unit_pos_x;
        u32            unit_pos_y;

    };

    //--------------------------------------------------------------------
    // ATLAS 
    //--------------------------------------------------------------------

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

    //--------------------------------------------------------------------
    // MEMORY 
    //--------------------------------------------------------------------
    
    enum cartographer_memory_block_type_e {
        cartographer_memory_block_type_e_unused    = 0,
        cartographer_memory_block_type_e_map       = 1,
        cartographer_memory_block_type_e_tile      = 2,
        cartographer_memory_block_type_e_wall      = 3,
        cartographer_memory_block_type_e_atlas     = 4,
        cartographer_memory_block_type_e_atlas_key = 5,
    };

    struct cartographer_memory_block {
        struct {
            u32 block_index; 
            u32 block_type;
        } header;
        union {
            map       map;
            tile      tile;
            wall      wall;
            atlas     atlas;
            atlas_key atlas_key;
        } data;
    };

    struct cartographer_block_allocator {
        cartographer_memory_block* block_array;
        index_cache                indexes;
        u32                        capacity;
        u32                        count_free;
        memory                     committed_memory;
    };

    IFB_INTERNAL void       cartographer_memory_init            (memory& reserved_memory);
    IFB_INTERNAL map*       cartographer_memory_alloc_map       (void);  
    IFB_INTERNAL tile*      cartographer_memory_alloc_tile      (void);  
    IFB_INTERNAL wall*      cartographer_memory_alloc_wall      (void);  
    IFB_INTERNAL atlas*     cartographer_memory_alloc_atlas     (void);  
    IFB_INTERNAL atlas_key* cartographer_memory_alloc_atlas_key (void);  

    IFB_INTERNAL void       cartographer_memory_free_map        (map*       m);
    IFB_INTERNAL void       cartographer_memory_free_tile       (tile*      t);
    IFB_INTERNAL void       cartographer_memory_free_wall       (wall*      w);
    IFB_INTERNAL void       cartographer_memory_free_atlas      (atlas*     a);
    IFB_INTERNAL void       cartographer_memory_free_atlas_key  (atlas_key* ak);
};
