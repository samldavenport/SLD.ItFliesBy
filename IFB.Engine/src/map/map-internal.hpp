#ifndef MAP_INTERNAL_HPP
#define MAP_INTERNAL_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "ifb-map.hpp"
#include "memory-internal.hpp"
#include "sld-strings.hpp"
#include "sld.hpp"
#include <cmath>
#include "eng-types.hpp"
#include "types-internal.hpp"

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
    // INTERNAL TYPES 
    //--------------------------------------------------------------------
   
    struct map;
    struct map_table;
    struct map_chunk;
    struct map_chunk_array;
    struct map_floor_tile;
    struct map_floor_array;
    struct map_wall_tile;
    struct map_wall_array;

    //--------------------------------------------------------------------
    // MAP MANAGER 
    //--------------------------------------------------------------------

    struct map_mngr {
        reservation* res;
        map_table*   tbl_map;
    } static * _map_mngr;

    //--------------------------------------------------------------------
    // MAP
    //--------------------------------------------------------------------

    IFB_INTERNAL u32              map_lookup_index      (const hnd_map map_hnd);
    IFB_INTERNAL map_dimensions&  map_get_dimensions    (const u32 map_index);
    IFB_INTERNAL map_chunk&       map_get_chunk         (const u32 map_index, const u32 chunk_index);
    IFB_INTERNAL map_floor_array& map_get_floor_array   (const u32 map_index);
    IFB_INTERNAL map_floor_tile*  map_find_floor_tile   (const u32 map_index, const s32 row, const s32 col, map_chunk** chunk_out);
    IFB_INTERNAL bool             map_is_tile_navigable (const u32 map_index, const s32 row, const s32 col);
    IFB_INTERNAL bool             map_get_sprite_rect   (const hnd_sprite_atlas h_atlas, const u32 i_sprite, f32& u_left, f32& v_top, f32& u_right, f32& v_bottom);
    IFB_INTERNAL map_wall_array&  map_get_wall_array    (const u32 map_index, const map_wall_orientation_e orientation);
    IFB_INTERNAL map_wall_tile*   map_chunk_get_wall    (map_chunk& chunk, const s32 row, const s32 col, const map_wall_orientation_e orientation);
    IFB_INTERNAL map_wall_tile*   map_find_wall         (const u32 map_index, const s32 row, const s32 col, const map_wall_orientation_e orientation, map_chunk** chunk_out);
    IFB_INTERNAL bool             map_has_wall          (const u32 map_index, const s32 row, const s32 col, const map_wall_orientation_e orientation);

    // limits for pushing an entity back onto the chunks, in tile units
    static constexpr u32 MAP_CONSTRAIN_ITERATION_COUNT = 8;
    static constexpr f32 MAP_CONSTRAIN_EPSILON         = 0.0001f;

    // walls collide as slabs centered on their edge, in tile units
    // they also extend past their ends by this much so corners are closed
    static constexpr f32 MAP_WALL_HALF_THICKNESS       = 0.05f;

    struct map {
        cstr_c16*      name;
        hnd_map        hnd;
        map_dimensions dims;
        vec3           origin;
    };

    struct map_wall_tile {
        map_color_u32     base_color;
        u32               i_sprite; // INVALID_INDEX if the wall is a solid color
        map_wall_flags_u8 flags;    // map_wall_flag_e, the wall only exists if it's placed
    };

    struct map_floor_tile {
        u32               i_sprite; // INVALID_INDEX if the tile is a solid color
        u8                color;    // map_color_e
        map_tile_flags_u8 flags;    // map_tile_flag_e
    };

    struct map_floor_array {
        u32            count;
        map_floor_tile tiles [IFB_CONFIG_MAP_TILE_CAPACITY];
    };

    struct map_wall_array {
        u32            count;
        map_wall_tile* tiles; // map_wall_capacity
    };

    struct map_chunk {
        u32              origin_row;
        u32              origin_col;
        u32              count_rows;
        u32              count_cols;
        map_color_u32    base_color;
        hnd_sprite_atlas h_atlas;      // INVALID_HANDLE until a floor tile or wall in the chunk has a sprite
        map_floor_tile*  floor;        // count_rows * count_cols, row-major, in the map's floor array
        map_wall_tile*   row_walls;    // (count_rows + 1) * count_cols, row-major, in the map's row wall array
        map_wall_tile*   column_walls; // count_rows * (count_cols + 1), row-major, in the map's column wall array
    };

    struct map_chunk_array {
        map_chunk chunks [IFB_CONFIG_MAP_CHUNK_CAPACITY];
    };

    struct map_table {
        hnd_map*         hnd;
        map_dimensions*  dims;
        cstr_c16*        name;
        map_chunk_array* chunk_array;
        map_floor_array* floor_array;
        map_wall_array*  column_wall_array;
        map_wall_array*  row_wall_array;
        vec3*            origin;
    };

    // these two are exact inverses and don't check the map bounds,
    // use map_calculate_is_in_bounds for that
    inline void
    map_calculate_position(
        const f32               tile_size,
        const vec3&             origin,
        const cmpnt_map_coords& coords,
        cmpnt_position&         pos) {

        pos.x = origin.x + (coords.col_x * tile_size);
        pos.y = origin.y + (coords.lvl_y * tile_size);
        pos.z = origin.z + (coords.row_z * tile_size);
    }

    inline void
    map_calculate_coordinates(
        const f32             tile_size,
        const vec3&           origin,
        const cmpnt_position& pos,
        cmpnt_map_coords&     coords) {

        assert(tile_size != 0);
        coords.col_x = (pos.x - origin.x) / tile_size;
        coords.lvl_y = (pos.y - origin.y) / tile_size;
        coords.row_z = (pos.z - origin.z) / tile_size;
    }

    // if the box overlaps the solid more than the current max overlap,
    // updates the max and sets the push to the shortest way out of the solid
    inline void
    map_calculate_box_push(
        const f32 box_x_min,
        const f32 box_z_min,
        const f32 box_x_max,
        const f32 box_z_max,
        const f32 solid_x_min,
        const f32 solid_z_min,
        const f32 solid_x_max,
        const f32 solid_z_max,
        f32&      overlap_area_max,
        f32&      push_x,
        f32&      push_z) {

        const f32 overlap_x = ((box_x_max < solid_x_max) ? box_x_max : solid_x_max) - ((box_x_min > solid_x_min) ? box_x_min : solid_x_min);
        const f32 overlap_z = ((box_z_max < solid_z_max) ? box_z_max : solid_z_max) - ((box_z_min > solid_z_min) ? box_z_min : solid_z_min);
        if (overlap_x <= MAP_CONSTRAIN_EPSILON || overlap_z <= MAP_CONSTRAIN_EPSILON) return;

        const f32 overlap_area = overlap_x * overlap_z;
        if (overlap_area <= overlap_area_max) return;
        overlap_area_max = overlap_area;

        // the shortest way out of the solid
        const f32 exit_x = ((box_x_max - solid_x_min) < (solid_x_max - box_x_min)) ? (solid_x_min - box_x_max) : (solid_x_max - box_x_min);
        const f32 exit_z = ((box_z_max - solid_z_min) < (solid_z_max - box_z_min)) ? (solid_z_min - box_z_max) : (solid_z_max - box_z_min);
        const bool is_exit_x = (fabsf(exit_x) < fabsf(exit_z));
        push_x = is_exit_x ? exit_x : 0.0f;
        push_z = is_exit_x ? 0.0f   : exit_z;
    }

    inline bool
    map_calculate_is_in_bounds(
        const map_dimensions&   dims,
        const cmpnt_map_coords& coords) {

        const bool is_col_valid = (coords.col_x >= 0.0f && coords.col_x < (f32)dims.count_cols);
        const bool is_row_valid = (coords.row_z >= 0.0f && coords.row_z < (f32)dims.count_rows);
        return(is_col_valid && is_row_valid);
    }
};

#endif //MAP_INTERNAL_HPP
