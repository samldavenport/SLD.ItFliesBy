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
    
    IFB_HANDLE(hnd_map);
    
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
    // COMPONENTS
    //--------------------------------------------------------------------

    // the position component is the source of truth for where an entity is
    // h_map is the map the entity belongs to, the rest is the position
    // measured in tiles from the map origin: columns along x, levels along y, rows along z
    struct cmpnt_map_coords {
        hnd_map h_map;
        f32     col_x;
        f32     lvl_y;
        f32     row_z;
    };

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


#endif //IFB_MAP_HPP
