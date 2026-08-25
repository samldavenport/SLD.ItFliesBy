#ifndef TILE_HPP
#define TILE_HPP

#include "ifb.hpp"

using namespace sld;

namespace ifb {

    struct tile_id {
        union {
            struct {
                byte grid_id;
                byte index;
            };
            u32 val;
        };
    };

    struct tile {
        tile_id id;
        u32     color_rgba;
        u32     texture_coord_x;
        u32     texture_coord_y;
    }; 

    struct tile_grid_id {
        u32 id;
    };

    struct tile_grid {
        tile_grid_id id;
        u32          count_rows;
        u32          count_columns;
        tile_id*     tile_id_array;
    };

    struct tile_map_id {
        u32 val;  
    };

    struct tile_map {
        u32           grid_count;
        tile_grid_id* grid_id_array;
    };




    IFB_INTERNAL tile_map_id  tile_map_create         (const cchar* name,     const u32 grid_count);
    IFB_INTERNAL tile_grid_id tile_grid_create        (const cchar* name,     const u32 map_id, const u32 row_count, const u32 col_count);
    IFB_INTERNAL void         tile_grid_set_navigable (const u32* tile_index, const u32 tile_count = 1);


};

#endif //TILE_HPP
