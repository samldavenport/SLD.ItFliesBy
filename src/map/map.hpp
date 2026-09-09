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

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    static constexpr u32 MAP_NAME_LENGTH = 16;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // map manager
    IFB_INTERNAL map_mngr*              map_mngr_create             (void);
    IFB_INTERNAL void                   map_mngr_startup            (memory& res);
    IFB_INTERNAL void                   map_mngr_shutdown           (void);
    IFB_INTERNAL const map_color_table& map_mngr_get_color_table    (void);
    IFB_INTERNAL f32                    map_mngr_get_tile_unit_size (void);

    // map
    IFB_INTERNAL map_handle map_create  (const cchar* map_name);
    IFB_INTERNAL void       map_destroy (const map_handle map_hnd);

    // map chunk
    IFB_INTERNAL u32
    map_chunk_create(
        const map_handle     map_hnd,
        const u32            count_rows,
        const u32            count_cols,
        const u32            offset_rows,
        const u32            offset_cols,
        const color_rgba_u32 base_color
    );

    IFB_INTERNAL void
    map_chunk_destroy(
        const map_handle map_hnd,
        const u32        chunk_index
    );

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
