#ifndef IFB_MAP_HPP
#define IFB_MAP_HPP

#include "ifb-types.hpp"
#include "ifb-entities.hpp"
#include "ifb-components.hpp"
#include "ifb-memory.hpp"
#include "sld.hpp"

using namespace sld;

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------
    
    struct map_mngr;
    struct map_color_table;
    struct map_dimensions;

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    static constexpr u32 MAP_NAME_LENGTH = 16;

    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum map_tile_flag_e {
        map_tile_flag_e_textured    = bit_value(1), 
        map_tile_flag_e_navigable   = bit_value(0),
        map_tile_flag_e_wall_row    = bit_value(2), // render only, the tile is a row wall
        map_tile_flag_e_wall_column = bit_value(3), // render only, the tile is a column wall
    };

    enum map_wall_flag_e {
        map_wall_flag_e_placed   = bit_value(0),
        map_wall_flag_e_textured = bit_value(1),
    };

    // row walls lie on the line z = row and run along x, on the min-z edge of tile (row, col)
    // column walls lie on the line x = col and run along z, on the min-x edge of tile (row, col)
    enum map_wall_orientation_e {
        map_wall_orientation_e_row    = 0,
        map_wall_orientation_e_column = 1,
    };

    enum map_color_e {
        map_color_e_red_light    = 0,
        map_color_e_red_dark     = 1,
        map_color_e_orange_light = 2,
        map_color_e_orange_dark  = 3,
        map_color_e_yellow_light = 4,
        map_color_e_yellow_dark  = 5,
        map_color_e_green_light  = 6,
        map_color_e_green_dark   = 7,
        map_color_e_aqua_light   = 8,
        map_color_e_aqua_dark    = 9,
        map_color_e_blue_light   = 10,
        map_color_e_blue_dark    = 11,
        map_color_e_purple_light = 12,
        map_color_e_purple_dark  = 13,
        map_color_e_gray_light   = 14,
        map_color_e_gray_dark    = 15,
        map_color_e_none         = 16,
    };

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct map_dimensions {
        u32 count_rows;
        u32 count_cols;
        u32 count_chunks;
    };

    struct map_color_table {
        color_rgba_u32 red_light;
        color_rgba_u32 red_dark;
        color_rgba_u32 orange_light;
        color_rgba_u32 orange_dark;
        color_rgba_u32 yellow_light;
        color_rgba_u32 yellow_dark;
        color_rgba_u32 green_light;
        color_rgba_u32 green_dark;
        color_rgba_u32 aqua_light;
        color_rgba_u32 aqua_dark;
        color_rgba_u32 blue_light;
        color_rgba_u32 blue_dark;
        color_rgba_u32 purple_light;
        color_rgba_u32 purple_dark;
        color_rgba_u32 gray_light;
        color_rgba_u32 gray_dark;
    };
};

#endif //IFB_MAP_HPP
