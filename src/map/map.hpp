#ifndef MAP_HPP
#define MAP_HPP

#include "ifb-types.hpp"
#include "ifb-entities.hpp"
#include "memory.hpp"
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
    // METHODS 
    //--------------------------------------------------------------------

    // map manager
    IFB_INTERNAL map_mngr*                map_mngr_create          (void);
    IFB_INTERNAL void                     map_mngr_startup         (reservation* res);
    IFB_INTERNAL void                     map_mngr_shutdown        (void);
    IFB_INTERNAL const map_color_table&   map_mngr_get_color_table (void);
    IFB_INTERNAL u32                      map_mngr_get_count       (void);
    IFB_INTERNAL hnd_map                  map_mngr_get_map         (const u32 index);
    
    // map
    IFB_INTERNAL hnd_map                  map_create                (const cchar* map_name, const u32 count_rows, const u32 count_cols);
    IFB_INTERNAL bool                     map_destroy               (const hnd_map h_map);
    IFB_INTERNAL u32                      map_render_buffer         (byte* buffer_data, const u32 buffer_size, const hnd_map h_map);
    IFB_INTERNAL bool                     map_get_dimensions        (const hnd_map h_map, map_dimensions& dims);
    IFB_INTERNAL bool                     map_get_navigable_bounds  (const hnd_map h_map, u32& row_min, u32& col_min, u32& row_max, u32& col_max);
    IFB_INTERNAL bool                     map_get_origin            (const hnd_map h_map, vec3& origin);
    IFB_INTERNAL bool                     map_set_origin            (const hnd_map h_map, const vec3& origin);
    IFB_INTERNAL bool                     map_get_pos_from_coords   (const cmpnt_map_coords& coords, cmpnt_position& pos);
    IFB_INTERNAL bool                     map_get_coords_from_pos   (const cmpnt_position&   pos,    cmpnt_map_coords& coords);
    IFB_INTERNAL bool                     map_is_in_bounds          (const cmpnt_map_coords& coords);
    IFB_INTERNAL bool                     map_constrain_movement    (const hnd_map h_map, const f32 inset, cmpnt_position& pos, cmpnt_velocity& vel);
    IFB_INTERNAL entity_list*             map_get_entities          (const hnd_map h_map, const hnd_arena h_arena);
    IFB_INTERNAL entity_list*             map_get_entities_in_chunk (const hnd_map h_map, const u32 chunk_index, const hnd_arena h_arena);
    IFB_INTERNAL bool                     map_tile_set_sprite       (const hnd_map h_map, const u32 row, const u32 col, const cchar* atlas_name_cstr, const cchar* sprite_name_cstr);                      
    IFB_INTERNAL bool                     map_chunk_set_sprite      (const hnd_map h_map, const u32 chunk_index, const cchar* atlas_name_cstr, const cchar* sprite_name_cstr);

    // map walls
    // row walls are on the min-z edge of tile (row, col), row can be count_rows for the far edge
    // column walls are on the min-x edge of tile (row, col), col can be count_cols for the far edge
    IFB_INTERNAL bool                     map_wall_set             (const hnd_map h_map, const u32 row, const u32 col, const map_wall_orientation_e orientation, const map_color_u32 color);
    IFB_INTERNAL bool                     map_wall_set_sprite      (const hnd_map h_map, const u32 row, const u32 col, const map_wall_orientation_e orientation, const cchar* atlas_name_cstr, const cchar* sprite_name_cstr);
    IFB_INTERNAL bool                     map_wall_clear           (const hnd_map h_map, const u32 row, const u32 col, const map_wall_orientation_e orientation);
    IFB_INTERNAL bool                     map_wall_is_placed       (const hnd_map h_map, const u32 row, const u32 col, const map_wall_orientation_e orientation);

    // map entities
    IFB_INTERNAL void                     map_entity_lookup_coords (const entity& e, cmpnt_map_coords& coords);
    IFB_INTERNAL void                     map_entity_update_coords (const entity& e, const cmpnt_map_coords& coords);

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
    map_chunk_get_dimensions(
        const hnd_map map_hnd,
        const u32     chunk_index,
        u32&          count_rows,
        u32&          count_cols,
        u32&          origin_row,
        u32&          origin_col
    );
    IFB_INTERNAL bool
    map_chunk_destroy(
        const hnd_map map_hnd,
        const u32        chunk_index
    );

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


#endif //MAP_HPP
