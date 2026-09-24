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
    struct map_render_buffer;
    struct map_dimensions;

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    static constexpr u32 MAP_NAME_LENGTH = 16;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // map manager
    IFB_INTERNAL map_mngr*                map_mngr_create               (void);
    IFB_INTERNAL void                     map_mngr_startup              (reservation* res);
    IFB_INTERNAL void                     map_mngr_shutdown             (void);
    IFB_INTERNAL const map_color_table&   map_mngr_get_color_table      (void);
    IFB_INTERNAL const map_render_buffer& map_mngr_get_render_buffer    (void);
    IFB_INTERNAL void                     map_mngr_calc_world_positions (void);

    // map
    IFB_INTERNAL hnd_map                  map_create               (const cchar* map_name, const u32 count_rows, const u32 count_cols);
    IFB_INTERNAL bool                     map_destroy              (const hnd_map h_map);
    IFB_INTERNAL bool                     map_render               (const hnd_map h_map);
    IFB_INTERNAL bool                     map_get_dimensions       (const hnd_map h_map, map_dimensions& dims);
    IFB_INTERNAL bool                     map_get_pos_from_coords  (const hnd_map h_map, const cmpnt_map_coords& coords, cmpnt_position&   pos);
    IFB_INTERNAL bool                     map_get_coords_from_pos  (const hnd_map h_map, const cmpnt_position&   pos,    cmpnt_map_coords& coords);
    IFB_INTERNAL bool                     map_get_entities         (const hnd_map h_map, entity_list* e_list);

    // map chunk
    IFB_INTERNAL u32
    map_chunk_create(
        const hnd_map       map_hnd,
        const u32           count_rows,
        const u32           count_cols,
        const u32           origin_row,
        const u32           origin_col,
        const map_color_u32 base_color
    );
    IFB_INTERNAL bool
    map_chunk_destroy(
        const hnd_map map_hnd,
        const u32        chunk_index
    );

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct map_render_buffer {
        u32 data_size;
        union {
            map_tile* tile_array;
            byte*     bytes;
        } data;
    };

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


#endif //MAP_HPP
