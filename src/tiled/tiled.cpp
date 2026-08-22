#pragma once

#include "tiled.hpp"
#include "json.cpp"
#include "json.hpp"

namespace ifb {

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
        
    }

    IFB_INTERNAL tiled_map*              tiled_map_parse             (arena* a, const json_obj* obj);
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
