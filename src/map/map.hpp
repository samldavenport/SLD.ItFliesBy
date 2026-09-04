#ifndef TILE_HPP
#define TILE_HPP

#include "ifb-types.hpp"
#include "memory.hpp"
#include "sld.hpp"

using namespace sld;

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------
    
    struct map_mngr;
    struct map_tile;
    struct map;
    struct map_tile_table;
    struct map_chunk_table;
    struct map_table;
    struct tile_render_context;
    struct tile_render_buffer;
    struct map_name;
    struct map_chunk;

    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------
   
    static map_mngr* _map_mngr; 

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    static constexpr u32 TILE_MAP_NAME_LENGTH = 16;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL map_mngr* map_mngr_create             (void);
    IFB_INTERNAL void      map_mngr_startup            (memory& res);
    IFB_INTERNAL void      map_mngr_shutdown           (void);
    IFB_INTERNAL f32       map_mngr_get_tile_unit_size (void);

    IFB_INTERNAL map_handle
    map_create(
        const cchar*         name,
        const u32            count_rows,
        const u32            count_col,
        const s32            offset_row,
        const s32            offset_col,
        const color_rgba_u32 base_color
    );

    IFB_INTERNAL void map_destroy                     (const map_handle map_hnd);
    IFB_INTERNAL u32  map_tile_count                  (const map_handle map_hnd);
    IFB_INTERNAL void map_set_color                   (const map_handle map_hnd, const map_coords* coords, const color_rgba_u32* color, const u32 count);
    IFB_INTERNAL void map_set_flags                   (const map_handle map_hnd, const map_coords* coords, const tile_flags_u32* flags, const u32 count);
    IFB_INTERNAL u32  map_get_render_buffer_size      (const map_handle map_hnd);
    IFB_INTERNAL u32  map_get_render_buffer_data      (const map_handle map_hnd, const u32 buffer_size, byte* buffer_data);
    IFB_INTERNAL bool map_get_info                    (const map_handle map_hnd, map& map);
    IFB_INTERNAL bool map_get_world_position          (const map_handle map_hnd, const u32 row, const u32 col, position_3d& pos);
    IFB_INTERNAL bool map_get_tile_coordinates        (const map_handle map_hnd, const position_3d& pos, map_coords& coords);
    IFB_INTERNAL bool map_get_entity_tile_coordinates (const map_handle map_hnd, const entity_id e, map_coords& coords); 

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct map_tile {
        map_handle      map_hnd;
        u32             row;
        u32             col;
        color_rgba_u32  color;
        tile_flags_u32  flags;
    };

    struct map_mngr {
        stack           mem_stack;
        map_tile_table* tbl_tiles;
        map_table*      tbl_map;
        u32             map_capacity;
        u32             tiles_per_map;
        f32             tile_unit_size;
    };

    struct map_name {
        cchar cstr[TILE_MAP_NAME_LENGTH];
    };

    struct map {
        map_handle id;
        u32        count_rows;
        u32        count_cols;
        s32        offset_row;
        s32        offset_col;
        f32        origin_x;
        f32        origin_z;
        map_name*  name;
    };

    struct map_chunk {
        map_handle map;
        u32        index;
        u32        rows;
        u32        cols;
        u32        offset_row;
        u32        offset_col;
    };

    struct map_table {
        map_handle* map_hnd;
        u32*        count_rows;
        u32*        count_cols;
        s32*        offset_row;
        s32*        offset_col;
        map_name*   name;
    };

    struct map_tile_table { 
        color_rgba_u32* color;
        tile_flags_u32* flags;
    };

    struct tile_render_context {
        union {
            struct {
                u32  color;
            };
            byte data[4];
        };
    };

    struct tile_render_buffer {
        u32 data_size;
        u32 tile_count;
        union {
            tile_render_context* ctx_array;
            byte*                bytes;
            void*                vptr;
            addr                 address;
        } data;
    };
};

#endif //TILE_HPP
