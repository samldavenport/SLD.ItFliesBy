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
        reservation*       res;
        map_table*         tbl_map;
        map_render_buffer* render_buffer;
    } static * _map_mngr;

    //--------------------------------------------------------------------
    // MAP
    //--------------------------------------------------------------------

    IFB_INTERNAL u32             map_lookup_index   (const map_handle map_hnd);
    IFB_INTERNAL map_dimensions& map_get_dimensions (const u32 map_index);
    IFB_INTERNAL map_chunk&      map_get_chunk      (const u32 map_index, const u32 chunk_index);


    struct map {
        cstr_c16*      name;
        map_handle     hnd;
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
        map_handle*      hnd;
        map_dimensions*  dims;
        cstr_c16*        name;
        map_chunk_array* chunk_array;
    };
};

#endif //MAP_INTERNAL_HPP
