#pragma once

#include "tiled.hpp"
#include "ifb-types.hpp"
#include "json.cpp"
#include "json.hpp"
#include "memory-arena.cpp"
#include "sld.hpp"

using namespace sld;
namespace ifb {

    inline u32
    tiled_hash_string_member(
        const cchar* cstr) {

        if (cstr == NULL) return(INVALID_HASH_32);

        static cchar cstr_tmp[16];
        (void) memset  (cstr_tmp, 0,    16);
        (void) memmove (cstr_tmp, cstr, strnlen(cstr, 16));

        return(hash_u32((void*)cstr_tmp, 16));
    }

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
        static const cchar cstr_right_down [16] = "right-down";
        static const cchar cstr_right_up   [16] = "right-up";
        static const cchar cstr_left_down  [16] = "left-down";
        static const cchar cstr_left_up    [16] = "left-up";
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
        const json_arr* arr_tileset      = json_obj_get_arr  (obj, "tilesets");
        const cchar*    orientation      = json_obj_get_cstr (obj, "orientation");
        const cchar*    render_order     = json_obj_get_cstr (obj, "renderorder");
        const cchar*    stagger_axis     = json_obj_get_cstr (obj, "staggeraxis");
        const cchar*    stagger_index    = json_obj_get_cstr (obj, "staggerindex");

        map->map_class         = json_obj_get_cstr  (obj, "class");
        map->compression_level = json_obj_get_s32   (obj, "compressionlevel");
        map->height            = json_obj_get_s32   (obj, "height");
        map->hex_side_length   = json_obj_get_s32   (obj, "hexsidelength");
        map->infinite          = json_obj_get_bool  (obj, "infinite");
        map->next_layer_id     = json_obj_get_s32   (obj, "nextlayerid");
        map->next_object_id    = json_obj_get_s32   (obj, "nextobjectid");
        map->parallax_origin_x = json_obj_get_f64   (obj, "parallaxoriginx");
        map->parallax_origin_y = json_obj_get_f64   (obj, "parallaxoriginy");
        map->skew_x            = json_obj_get_s32   (obj, "skewx");
        map->skew_y            = json_obj_get_s32   (obj, "skewy");
        map->tiled_version     = json_obj_get_cstr  (obj, "tiledversion");
        map->tile_height       = json_obj_get_s32   (obj, "tileheight");
        map->tile_width        = json_obj_get_s32   (obj, "tilewidth");
        map->format_version    = json_obj_get_cstr  (obj, "version");
        map->column_width      = json_obj_get_s32   (obj, "width");
        map->property_count    = json_arr_get_count (arr_properties);
        map->layer_count       = json_arr_get_count (arr_layer);
        map->orientation       = tiled_hash_string_member (orientation);
        map->render_order      = tiled_hash_string_member (render_order);
        map->stagger_axis      = tiled_hash_string_member (stagger_axis);
        map->stagger_index     = tiled_hash_string_member (stagger_index);
        map->layer_array       = tiled_layer_parse_array      (a, arr_layer,      map->layer_count);
        map->property_array    = tiled_property_parse_array   (a, arr_properties, map->property_count); 
        map->tileset_array     = tiled_tileset_parse_array    (a, arr_tileset,    map->tileset_count);

        return(map);
    }

    IFB_INTERNAL const tiled_layer_hashes&
    tiled_layer_get_hashes(
        void) {

        static tiled_layer_hashes hashes = {0};

        const cchar cstr_zlib         [16] = "zlib";
        const cchar cstr_gzip         [16] = "gzip";
        const cchar cstr_zstd         [16] = "zstd";
        const cchar cstr_normal       [16] = "normal";
        const cchar cstr_add          [16] = "add";
        const cchar cstr_multiply     [16] = "multiply";
        const cchar cstr_screen       [16] = "screen";
        const cchar cstr_overlay      [16] = "overlay";
        const cchar cstr_darken       [16] = "darken";
        const cchar cstr_lighten      [16] = "lighten";
        const cchar cstr_color_dodge  [16] = "colordodge";
        const cchar cstr_color_burn   [16] = "colorburn";
        const cchar cstr_hard_light   [16] = "hardlight";
        const cchar cstr_soft_light   [16] = "softlight";
        const cchar cstr_difference   [16] = "difference";
        const cchar cstr_exclusion    [16] = "exclusion";
        const cchar cstr_topdown      [16] = "topdown"; 
        const cchar cstr_index        [16] = "index"; 
        const cchar cstr_csv          [16] = "csv"; 
        const cchar cstr_base64       [16] = "base64"; 
        const cchar cstr_tile_layer   [16] = "tilelayer"; 
        const cchar cstr_object_group [16] = "objectgroup"; 
        const cchar cstr_image_layer  [16] = "imagelayer"; 
        const cchar cstr_group        [16] = "group"; 

        hashes.zlib         = hash_u32((void*)cstr_zlib,         16);
        hashes.gzip         = hash_u32((void*)cstr_gzip,         16);
        hashes.zstd         = hash_u32((void*)cstr_zstd,         16);
        hashes.normal       = hash_u32((void*)cstr_normal,       16);
        hashes.add          = hash_u32((void*)cstr_add,          16);
        hashes.multiply     = hash_u32((void*)cstr_multiply,     16);
        hashes.screen       = hash_u32((void*)cstr_screen,       16);
        hashes.overlay      = hash_u32((void*)cstr_overlay,      16);
        hashes.darken       = hash_u32((void*)cstr_darken,       16);
        hashes.lighten      = hash_u32((void*)cstr_lighten,      16);
        hashes.color_dodge  = hash_u32((void*)cstr_color_dodge,  16);
        hashes.color_burn   = hash_u32((void*)cstr_color_burn,   16);
        hashes.hard_light   = hash_u32((void*)cstr_hard_light,   16);
        hashes.soft_light   = hash_u32((void*)cstr_soft_light,   16);
        hashes.difference   = hash_u32((void*)cstr_difference,   16);
        hashes.exclusion    = hash_u32((void*)cstr_exclusion,    16);
        hashes.topdown      = hash_u32((void*)cstr_topdown,      16); 
        hashes.index        = hash_u32((void*)cstr_index,        16); 
        hashes.csv          = hash_u32((void*)cstr_csv,          16); 
        hashes.base64       = hash_u32((void*)cstr_base64,       16); 
        hashes.tile_layer   = hash_u32((void*)cstr_tile_layer,   16); 
        hashes.object_group = hash_u32((void*)cstr_object_group, 16); 
        hashes.image_layer  = hash_u32((void*)cstr_image_layer,  16); 
        hashes.group        = hash_u32((void*)cstr_group,        16); 

        return(hashes);
    }

    IFB_INTERNAL tiled_layer*
    tiled_layer_parse_array(
        arena*          a,
        const json_arr* arr,
        s32&            count) {

        assert(a);
        if (!arr) return(NULL);
    
        count = json_arr_get_count(arr); 
        if (count == 0) return(NULL);

        auto layer_array = arena_push<tiled_layer>(a, count);
        assert(layer_array);

        for (
            u32 i = 0;
            i < count;
            ++i) {
        
             
        }

    }

    IFB_INTERNAL tiled_object_layer*     tiled_object_layer_parse    (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_object *          tiled_object_parse          (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_chunk*            tiled_chunk_parse           (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_text*             tiled_text_parse            (arena* a, const json_obj* obj);


    IFB_INTERNAL tiled_tileset*
    tiled_tileset_parse_array(
        arena*          a,
        const json_arr* arr,
        s32&            count) {

        count = json_arr_get_count (arr); 
   
        if (count == 0) return(NULL);

        auto tileset_array = arena_push<tiled_tileset>(a, count);

        for (
            u32 i = 0;
            i < count;
            ++i) {

            const json_obj* obj = json_arr_get_obj(arr, i);
            assert(obj);


                    

        }

        return(NULL);
    }

    IFB_INTERNAL tiled_grid*
    tiled_grid_parse(
        arena* a,
        const json_obj* obj) {

        //TODO(SLD)
        return(NULL);
    }

    IFB_INTERNAL tiled_tile_offset*
    tiled_tile_offset_parse(
        arena* a,
        const json_obj* obj) {

        assert(a);

        if (obj == NULL) return(NULL);
       
        auto offset = arena_push<tiled_tile_offset>(a);
        assert(offset != NULL);

        offset->x = json_obj_get_s32(obj, "x");
        offset->y = json_obj_get_s32(obj, "y");
       
        return(offset);
    }   

    IFB_INTERNAL tiled_transformations*
    tiled_transformations_parse(
        arena* a,
        const json_obj* obj) {

        assert(a);
        if (obj == NULL) return(NULL);

        auto xform = arena_push<tiled_transformations>(a);
        assert(xform);

        xform->h_flip               = json_obj_get_bool(obj, "hflip"); 
        xform->v_flip               = json_obj_get_bool(obj, "vflip"); 
        xform->rotate               = json_obj_get_bool(obj, "rotate"); 
        xform->prefer_untransformed = json_obj_get_bool(obj, "preferuntransformed"); 

        return(xform);
    }

    IFB_INTERNAL tiled_tile*             tiled_tile_parse            (arena* a, const json_obj* obj);

    IFB_INTERNAL tiled_frame*
    tiled_frame_parse(
        arena* a,
        const json_obj* obj) {

        assert(a);
        if (obj == NULL) return(NULL);

        auto frame = arena_push<tiled_frame>(a);
        assert(frame);

        frame->duration = json_obj_get_s32(obj, "duration");
        frame->tile_id  = json_obj_get_s32(obj, "tileid");
    }
    
    IFB_INTERNAL tiled_terrain*
    tiled_terrain_parse(
        arena* a,
        const json_obj* obj) {

        assert(a);
        if (obj == NULL) return(NULL);

        auto terrain = arena_push<tiled_terrain>(a);
        assert(terrain != NULL);

        const auto prop_array = json_obj_get_arr(obj, "properties");

        terrain->name           = json_obj_get_cstr          (obj, "name");
        terrain->tile           = json_obj_get_s32           (obj, "tile");
        terrain->property_array = tiled_property_parse_array (a, prop_array, terrain->property_count);

        return(terrain);
    }

    IFB_INTERNAL tiled_wang_set*         tiled_wang_set_parse        (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_color*       tiled_wang_color_parse      (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_wang_tile*        tiled_wang_tile_parse       (arena* a, const json_obj* obj);
    IFB_INTERNAL tiled_object_template*  tiled_object_template_parse (arena* a, const json_obj* obj);

    IFB_INTERNAL const tiled_property_hashes&
    tiled_property_get_hashes(
        void) {

        static tiled_property_hashes hashes;

        static const cchar cstr_string [16] = "string"; 
        static const cchar cstr_int    [16] = "int"; 
        static const cchar cstr_float  [16] = "float"; 
        static const cchar cstr_bool   [16] = "bool"; 
        static const cchar cstr_color  [16] = "color"; 
        static const cchar cstr_file   [16] = "file"; 
        static const cchar cstr_object [16] = "object"; 
        static const cchar cstr_class  [16] = "class";

        hashes.type_string = hash_u32 ((void*)cstr_string, 16); 
        hashes.type_int    = hash_u32 ((void*)cstr_int,    16); 
        hashes.type_float  = hash_u32 ((void*)cstr_float,  16); 
        hashes.type_bool   = hash_u32 ((void*)cstr_bool,   16); 
        hashes.type_color  = hash_u32 ((void*)cstr_color,  16); 
        hashes.type_file   = hash_u32 ((void*)cstr_file,   16); 
        hashes.type_object = hash_u32 ((void*)cstr_object, 16); 
        hashes.type_class  = hash_u32 ((void*)cstr_class,  16); 

        return(hashes);
    }

    IFB_INTERNAL tiled_property*
    tiled_property_parse_array(
        arena*          a,
        const json_arr* arr,
        s32&            count) {

        assert(a != NULL);

        if (arr == NULL) return(NULL);

        count = json_arr_get_count(arr);
        if (count == 0) return(NULL);

        auto property_array = arena_push<tiled_property>(a, count);
        if (!property_array) return(NULL);

        for (
            u32 i = 0;
            i < count;
            ++i) {

            auto& prop             = property_array[i];
            const json_obj* obj    = json_arr_get_obj(arr, i);
            const cchar* type_cstr = json_obj_get_cstr(obj, "type");
            prop.name              = json_obj_get_cstr(obj, "name"); 
            prop.custom_type       = json_obj_get_cstr(obj, "propertytype");
            prop.type              = tiled_hash_string_member(type_cstr);

            switch(prop.type) {
                case ((u32)tiled_property_type_e_string): prop.value.as_cstr = json_obj_get_cstr (obj, "value"); break;
                case ((u32)tiled_property_type_e_int):    prop.value.as_s32  = json_obj_get_s32  (obj, "value"); break;
                case ((u32)tiled_property_type_e_float):  prop.value.as_f32  = json_obj_get_f32  (obj, "value"); break;
                case ((u32)tiled_property_type_e_bool):   prop.value.as_bool = json_obj_get_bool (obj, "value"); break;
                case ((u32)tiled_property_type_e_color):     
                case ((u32)tiled_property_type_e_file):       
                case ((u32)tiled_property_type_e_object):   
                case ((u32)tiled_property_type_e_class): break;    
                default: assert(false && "Invalid tiled property type");
            }
        }

        return(property_array);
    }

    IFB_INTERNAL tiled_point*
    tiled_point_parse(
        arena*          a,
        const json_obj* obj) {

        assert(a);
        if (obj == NULL) return(NULL);

        auto point = arena_push<tiled_point>(a);
        assert(point);

        point->x = json_obj_get_f32 (obj, "x");
        point->y = json_obj_get_f32 (obj, "y");

        return(point);
    }
};
