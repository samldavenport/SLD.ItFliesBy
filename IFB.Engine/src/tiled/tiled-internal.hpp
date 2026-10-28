#ifndef TILED_INTERNAL_HPP
#define TILED_INTERNAL_HPP

#include "ifb-tiled.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const tiled_map_hashes&      tiled_map_get_hashes         (void); 
    IFB_INTERNAL tiled_map*                   tiled_map_create             (hnd_arena a, const u32 json_src_length, const cchar* json_src_ptr);
    IFB_INTERNAL tiled_map*                   tiled_map_parse              (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_layer*                 tiled_layer_parse_array      (hnd_arena a, const json_arr* arr, s32& count);
    IFB_INTERNAL const tiled_layer_hashes&    tiled_layer_get_hashes       (void);
    IFB_INTERNAL tiled_object_layer*          tiled_object_layer_parse     (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_object *               tiled_object_parse           (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_chunk*                 tiled_chunk_parse            (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_text*                  tiled_text_parse             (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_tileset*               tiled_tileset_parse_array    (hnd_arena a, const json_arr* arr, s32& count);
    IFB_INTERNAL tiled_grid*                  tiled_grid_parse             (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_tile_offset*           tiled_tile_offset_parse      (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_transformations*       tiled_transformations_parse  (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_tile*                  tiled_tile_parse             (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_frame*                 tiled_frame_parse            (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_terrain*               tiled_terrain_parse          (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_set*              tiled_wang_set_parse         (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_color*            tiled_wang_color_parse       (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_tile*             tiled_wang_tile_parse        (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL tiled_object_template*       tiled_object_template_parse  (hnd_arena a, const json_obj* obj);
    IFB_INTERNAL const tiled_property_hashes& tiled_property_get_hashes    (void);
    IFB_INTERNAL tiled_property*              tiled_property_parse_array   (hnd_arena a, const json_arr* arr, s32& count);
    IFB_INTERNAL tiled_point*                 tiled_point_parse            (hnd_arena a, const json_obj* obj);
};

#endif //TILED_INTERNAL_HPP
