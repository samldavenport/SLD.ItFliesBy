#ifndef MAP_HPP
#define MAP_HPP

#include "ifb-types.hpp"
#include "memory.hpp"
#include "sld.hpp"

using namespace sld;

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------
    
    struct map_mngr;
    struct map_color_table;
    struct map_memory;
    struct map_memory_block;
    struct map_memory_block_alctr;

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    static constexpr u32 MAP_NAME_LENGTH = 16;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL map_mngr*              map_mngr_create             (void);
    IFB_INTERNAL void                   map_mngr_startup            (memory& res);
    IFB_INTERNAL void                   map_mngr_shutdown           (void);
    IFB_INTERNAL const map_color_table& map_mngr_get_color_table    (void);
    IFB_INTERNAL f32                    map_mngr_get_tile_unit_size (void);

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

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

#endif //MAP_HPP
