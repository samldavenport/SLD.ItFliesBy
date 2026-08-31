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
    
    IFB_U32(tile_id_u32);
    IFB_U32(tile_map_id_u32);
    IFB_U32(tile_flags_u32);

    struct tile_mngr;
    struct tile;
    struct tile_map;
    struct tile_table;
    struct tile_map_table;
    struct tile_render_context;
    struct tile_render_buffer;
    struct tile_map_name;

    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------
   
    static tile_mngr* _tile_mngr; 

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    static constexpr u32 TILE_MAP_NAME_LENGTH = 16;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL tile_mngr* tile_mngr_create   (void);
    IFB_INTERNAL void       tile_mngr_startup  (memory& res);
    IFB_INTERNAL void       tile_mngr_shutdown (void);

    IFB_INTERNAL tile_map_id_u32
    tile_map_create(
        const cchar* name,
        const f32    tile_width,
        const f32    tile_height,
        const u32    count_rows,
        const u32    count_col
    );

    IFB_INTERNAL void tile_map_destroy           (const tile_map_id_u32 map_id);
    IFB_INTERNAL u32  tile_map_tile_count        (const tile_map_id_u32 map_id);
    IFB_INTERNAL void tile_map_set_color         (const tile_map_id_u32 map_id, const u32* row, const u32* col, const color_rgba_u32* color, const u32 count);
    IFB_INTERNAL void tile_map_set_flags         (const tile_map_id_u32 map_id, const u32* row, const u32* col, const tile_flags_u32* flags, const u32 count);
    IFB_INTERNAL bool tile_map_get_render_buffer (const tile_map_id_u32 map_id, tile_render_buffer* render_buffer, arena* a);

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct tile {
        union {
            struct {
                u16 row;
                u16 col;
            };
            tile_id_u32 id;
        };
        u32             map_id;
        color_rgba_u32  color;
        tile_flags_u32  flags;
    };

    struct tile_mngr {
        stack           mem_stack;
        tile_table*     tbl_tiles;
        tile_map_table* tbl_map;
        u32             map_capacity;
        u32             tiles_per_map;
    };

    struct tile_map_name {
        cchar cstr[TILE_MAP_NAME_LENGTH];
    };

    struct tile_map {
        tile_map_id_u32 id;
        f32             tile_width;
        f32             tile_height;
        u32             count_rows;
        u32             count_cols;
        tile_map_name   name;
    };

    struct tile_map_table {
        tile_map_id_u32* map_id;
        f32*             tile_width;
        f32*             tile_height;
        u32*             count_rows;
        u32*             count_cols;
        tile_map_name*   name;
    };

    struct tile_table {
        color_rgba_u32* color;
        tile_flags_u32* flags;
    };

    struct tile_render_context {
        union {
            struct {
                vec2 corner;
                u32  id;
                u32  color;
            };
            byte data[12];
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
