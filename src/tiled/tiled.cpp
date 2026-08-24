#pragma once

#include "tiled.hpp"
#include "json.cpp"
#include "json.hpp"
#include "memory-arena.cpp"
#include "sld.hpp"

using namespace sld;
namespace ifb {


    //--------------------------------------------------------------------
    // MAP METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const tiled_map_hashes&
    tiled_map_get_hashes(
        void) {

        static tiled_map_hashes hashes = {0};
    
        static const cchar cstr_orthogonal [16] = "orthogonal";
        static const cchar cstr_isometric  [16] = "isometric";
        static const cchar cstr_oblique    [16] = "oblique";
        static const cchar cstr_staggered  [16] = "staggered";
        static const cchar cstr_hexagonal  [16] = "hexagonal";
        static const cchar cstr_right_down [16] = "right_down";
        static const cchar cstr_right_up   [16] = "right_up";
        static const cchar cstr_left_down  [16] = "left_down";
        static const cchar cstr_left_up    [16] = "left_up";
        static const cchar cstr_x          [16] = "x";
        static const cchar cstr_y          [16] = "y";
        static const cchar cstr_odd        [16] = "odd";
        static const cchar cstr_even       [16] = "even";
        static const cchar cstr_map        [16] = "map";

        hashes.orthogonal = hash_u32((void*)cstr_orthogonal, 16);
        hashes.isometric  = hash_u32((void*)cstr_isometric,  16);
        hashes.oblique    = hash_u32((void*)cstr_oblique,    16);
        hashes.staggered  = hash_u32((void*)cstr_staggered,  16);
        hashes.hexagonal  = hash_u32((void*)cstr_hexagonal,  16);
        hashes.right_down = hash_u32((void*)cstr_right_down, 16);
        hashes.right_up   = hash_u32((void*)cstr_right_up,   16);
        hashes.left_down  = hash_u32((void*)cstr_left_down,  16);
        hashes.left_up    = hash_u32((void*)cstr_left_up,    16);
        hashes.x          = hash_u32((void*)cstr_x,          16);
        hashes.y          = hash_u32((void*)cstr_y,          16);
        hashes.odd        = hash_u32((void*)cstr_odd,        16);
        hashes.even       = hash_u32((void*)cstr_even,       16);
        hashes.map        = hash_u32((void*)cstr_map,        16);

        return(hashes);
    } 
    
    IFB_INTERNAL tiled_map*
    tiled_map_create(
        arena*       a,
        const u32    json_src_length,
        const cchar* json_src_ptr) {

        assert(a               != NULL);
        assert(json_src_length != 0);
        assert(json_src_ptr    != NULL);

        // create the document
        const json_doc* doc = json_doc_create(a, json_src_length, json_src_ptr);
        assert(doc);

        // get the map object
        const json_obj* map_obj = json_doc_get_obj(doc, "map");

        // create the map
        tiled_map* map = tiled_map_parse(a, map_obj);

        return(map);
    }

    IFB_INTERNAL tiled_map*
    tiled_map_parse(
        arena* a,
        const json_obj* obj) {

        assert(a);
        assert(obj);

        auto map = arena_push<tiled_map>(a);
        if (!map) return(NULL);


        bool result = true;

        const cchar*    background_color = json_obj_get_cstr (obj, "backgroundcolor");
        const json_arr* arr_layer        = json_obj_get_arr  (obj, "layers"); 
        const json_arr* arr_properties   = json_obj_get_arr  (obj, "properties"); 
        const json_obj* obj_tileset      = json_obj_get_obj  (obj, "tileset");

        map->map_class         = json_obj_get_cstr (obj, "class");
        map->compression_level = json_obj_get_s32  (obj, "compressionlevel");
        map->height            = json_obj_get_s32  (obj, "height");
        map->hex_side_length   = json_obj_get_s32  (obj, "hexsidelength");
        map->infinite          = json_obj_get_bool (obj, "infinite");
        map->next_layer_id     = json_obj_get_s32  (obj, "nextlayerid");
        map->next_object_id    = json_obj_get_s32  (obj, "nextobjectid");

        return(NULL);

    }

    IFB_INTERNAL tiled_layer*            tiled_layer_parse           (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_tile_layer*       tiled_tile_layer_parse      (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_object_layer*     tiled_object_layer_parse    (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_object *          tiled_object_parse          (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_chunk*            tiled_chunk_parse           (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_text*             tiled_text_parse            (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_tileset*          tiled_tileset_parse         (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_grid*             tiled_grid_parse            (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_tile_offset*      tiled_tile_offset_parse     (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_transformations*  tiled_transformations_parse (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_tile*             tiled_tile_parse            (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_frame*            tiled_frame_parse           (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_terrain*          tiled_terrain_parse         (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_set*         tiled_wang_set_parse        (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_color*       tiled_wang_color_parse      (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_tile*        tiled_wang_tile_parse       (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_object_template*  tiled_object_template_parse (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_property*         tiled_property_parse        (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_point*            tiled_point_parse           (arena* a, const json_obj* obj);


};
