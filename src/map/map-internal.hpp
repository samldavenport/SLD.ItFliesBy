#ifndef MAP_INTERNAL_HPP
#define MAP_INTERNAL_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "map.hpp"
#include "memory-reservation.cpp"
#include "sld-strings.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL TYPES 
    //--------------------------------------------------------------------
   
    struct map;
    struct map_table;
    struct map_chunk;
    struct map_chunk_array;

    //--------------------------------------------------------------------
    // MAP MANAGER 
    //--------------------------------------------------------------------

    struct map_mngr {
        reservation*      res;
        map_table*        tbl_map;
        map_render_buffer render_buffer;
    } static * _map_mngr;

    //--------------------------------------------------------------------
    // MAP
    //--------------------------------------------------------------------

    IFB_INTERNAL u32             map_lookup_index   (const hnd_map map_hnd);
    IFB_INTERNAL map_dimensions& map_get_dimensions (const u32 map_index);
    IFB_INTERNAL map_chunk&      map_get_chunk      (const u32 map_index, const u32 chunk_index);

    struct map {
        cstr_c16*      name;
        hnd_map        hnd;
        map_dimensions dims;
        vec3           origin;
    };

    struct map_chunk {
        u32           origin_row;
        u32           origin_col;
        u32           count_rows;
        u32           count_cols;
        map_color_u32 base_color;
    };

    struct map_chunk_array {
        map_chunk chunks [IFB_CONFIG_MAP_CHUNK_CAPACITY];
    };

    struct map_table {
        hnd_map*         hnd;
        map_dimensions*  dims;
        cstr_c16*        name;
        map_chunk_array* chunk_array;
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
