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
    };

    inline void
    map_calculate_position(
        const f32               tile_size, 
        const map_dimensions&   dims,
        const cmpnt_map_coords& coords,
        cmpnt_position&         pos) {
        
        const bool is_x_valid = (coords.row_x >= 0 && coords.row_x <= dims.count_rows); 
        const bool is_z_valid = (coords.col_z >= 0 && coords.col_z <= dims.count_cols); 

        // calculate width and height
        const f32 map_width  = dims.count_cols * tile_size;
        const f32 map_height = dims.count_rows * tile_size;

        pos.x = is_x_valid ? (coords.row_x * tile_size) : POS_INVALID;
        pos.z = is_z_valid ? (coords.col_z * tile_size) : POS_INVALID;
    }

    inline void
    map_calculate_coordinates(
        const hnd_map         h_map,
        const f32             tile_size, 
        const map_dimensions& dims,
        const cmpnt_position& pos,
        cmpnt_map_coords&     coords) {
        
        // calculate width and height
        const f32 map_width  = dims.count_cols * tile_size;
        const f32 map_height = dims.count_rows * tile_size;

        // calculate the map coordinates
        assert(tile_size != 0);
        coords.h_map = h_map;
        coords.row_x = pos.x <= map_width  ? pos.x / tile_size : MAP_COORD_INVALID; 
        coords.col_z = pos.z <= map_height ? pos.z / tile_size : MAP_COORD_INVALID; 
    }    
};

#endif //MAP_INTERNAL_HPP
