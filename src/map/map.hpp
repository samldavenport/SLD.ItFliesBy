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
    
    IFB_U32(map_id_u32);

    struct map_mngr;
    struct tile;
    struct map;
    struct tile_table;
    struct map_table;
    struct tile_render_context;
    struct tile_render_buffer;
    struct map_name;

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

    IFB_INTERNAL map_mngr* map_mngr_create   (void);
    IFB_INTERNAL void      map_mngr_startup  (memory& res);
    IFB_INTERNAL void      map_mngr_shutdown (void);

    IFB_INTERNAL map_id_u32
    map_create(
        const cchar*         name,
        const f32            tile_width,
        const f32            tile_height,
        const u32            count_rows,
        const u32            count_col,
        const color_rgba_u32 base_color
    );

    IFB_INTERNAL void map_destroy                (const map_id_u32 map_id);
    IFB_INTERNAL u32  map_tile_count             (const map_id_u32 map_id);
    IFB_INTERNAL void map_set_color              (const map_id_u32 map_id, const tile_coords* coords, const color_rgba_u32* color, const u32 count);
    IFB_INTERNAL void map_set_flags              (const map_id_u32 map_id, const tile_coords* coords, const tile_flags_u32* flags, const u32 count);
    IFB_INTERNAL u32  map_get_render_buffer_size (const map_id_u32 map_id);
    IFB_INTERNAL u32  map_get_render_buffer_data (const map_id_u32 map_id, const u32 buffer_size, byte* buffer_data);
    IFB_INTERNAL bool map_get_info               (const map_id_u32 map_id, map& map);

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct tile {
        u32             row;
        u32             col;
        color_rgba_u32  color;
        tile_flags_u32  flags;
    };

    struct map_mngr {
        stack           mem_stack;
        tile_table*     tbl_tiles;
        map_table* tbl_map;
        u32             map_capacity;
        u32             tiles_per_map;
    };

    struct map_name {
        cchar cstr[TILE_MAP_NAME_LENGTH];
    };

    struct map {
        map_id_u32 id;
        f32        tile_width;
        f32        tile_height;
        u32        count_rows;
        u32        count_cols;
        map_name*  name;
    };

    struct map_table {
        map_id_u32* map_id;
        f32*        tile_width;
        f32*        tile_height;
        u32*        count_rows;
        u32*        count_cols;
        map_name*   name;
    };

    struct tile_table {
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
