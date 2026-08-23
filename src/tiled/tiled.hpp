#ifndef TILED_HPP
#define TILED_HPP

#include "ifb.hpp"
#include "memory.hpp"
#include "sld.hpp"
#include "json.hpp"

namespace ifb {

    //NOTE(SAM):https://doc.mapeditor.org/en/stable/reference/json-map-format/#json-map-format 

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct tiled_map;
    struct tiled_layer;
    struct tiled_tile_layer;
    struct tiled_object_layer;
    struct tiled_object; 
    struct tiled_chunk;
    struct tiled_text;
    struct tiled_tileset;
    struct tiled_grid;
    struct tiled_tile_offset;
    struct tiled_transformations;
    struct tiled_tile;
    struct tiled_frame;
    struct tiled_terrain;
    struct tiled_wang_set;
    struct tiled_wang_color;
    struct tiled_wang_tile;
    struct tiled_object_template;
    struct tiled_property;
    struct tiled_point;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL tiled_map*              tiled_map_create            (arena* a, const u32 json_src_length, const cchar* json_src_ptr);
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

    //--------------------------------------------------------------------
    // MAP 
    //--------------------------------------------------------------------

    enum tiled_map_orientation_e {
        tiled_map_orientation_e_orthogonal = 0,
        tiled_map_orientation_e_isometric  = 1,
        tiled_map_orientation_e_oblique    = 2,
        tiled_map_orientation_e_staggered  = 3,
        tiled_map_orientation_e_hexagonal  = 4
    };

    enum tiled_map_render_order_e {
        tiled_map_render_order_e_right_down = 0, 
        tiled_map_render_order_e_right_up   = 1, 
        tiled_map_render_order_e_left_down  = 2, 
        tiled_map_render_order_e_left_up    = 3
    };

    enum tiled_map_stagger_axis_e {
        tiled_map_stagger_axis_e_x = 0,
        tiled_map_stagger_axis_e_y = 1,
    };

    enum tiled_map_stagger_index_e {
        tiled_map_stagger_index_e_odd  = 0,
        tiled_map_stagger_index_e_even = 1
    };

    using tiled_map_orientation_u32   = u32;
    using tiled_map_render_order_u32  = u32;
    using tiled_map_stagger_axis_u32  = u32;
    using tiled_map_stagger_index_u32 = u32;

    struct tiled_map {
        const cchar*                map_class;
        const cchar*                tiled_version;
        const cchar*                format_version;
        tiled_layer*                layer_array;
        tiled_property*             property_array; 
        tiled_tileset*              tileset_array; 
        tiled_map_orientation_u32   orientation;
        tiled_map_render_order_u32  render_order;
        tiled_map_stagger_axis_u32  stagger_axis;
        tiled_map_stagger_index_u32 stagger_index;
        color_rgba_u32              background_color;
        u32                         property_count;
        u32                         layer_count;
        u32                         compression_level;
        u32                         height;
        u32                         hex_side_length;
        u32                         next_layer_id;
        u32                         next_object_id;
        u32                         parallax_origin_x;
        u32                         parallax_origin_y;
        u32                         skew_x;
        u32                         skew_y;
        u32                         tile_height;
        u32                         tileset_count;
        u32                         tile_width;
        u32                         column_width;
        bool                        infinite;
    };
   

    //--------------------------------------------------------------------
    // LAYER 
    //--------------------------------------------------------------------

    enum tiled_layer_compression_e {
        tiled_layer_compression_e_zlib = 0,
        tiled_layer_compression_e_gzip = 1,
        tiled_layer_compression_e_zstd = 2
    };

    enum tiled_layer_mode_e {
        tiled_layer_mode_e_normal      = 0,  
        tiled_layer_mode_e_add         = 1,  
        tiled_layer_mode_e_multiply    = 2,  
        tiled_layer_mode_e_screen      = 3,  
        tiled_layer_mode_e_overlay     = 4,  
        tiled_layer_mode_e_darken      = 5,  
        tiled_layer_mode_e_lighten     = 6,  
        tiled_layer_mode_e_color_dodge = 7,  
        tiled_layer_mode_e_color_burn  = 8,  
        tiled_layer_mode_e_hard_light  = 9,  
        tiled_layer_mode_e_soft_light  = 10,  
        tiled_layer_mode_e_difference  = 11,  
        tiled_layer_mode_e_exclusion   = 12
    };

    enum tiled_layer_draw_order_e {
        tiled_layer_draw_order_e_topdown = 0,
        tiled_layer_draw_order_e_index   = 1,
    };

    enum tiled_layer_encoding_e {
        tiled_layer_encoding_e_csv   = 0,
        tiled_layer_encoding_e_index = 1
    };

    enum tiled_layer_type_e {
        tiled_layer_type_e_tile_layer = 0,
        tiled_layer_type_e_object_group = 1,
        eiled_layer_type_e_image_layer = 2,
        tiled_layer_type_e_group = 3
    };

    using tiled_layer_compression_u32 = u32;
    using tiled_layer_mode_u32        = u32;
    using tiled_layer_draw_order_u32  = u32;
    using tiled_layer_encoding_u32    = u32;
    using tiled_layer_type_u32        = u32;

    struct tiled_layer {
        union {
            u32*   gid_array;
            cchar* base_64_encoded_cstr;
        } data;
        cchar*                      name;
        cchar*                      layer_class;
        tiled_chunk*                chunk_array;
        cchar*                      image;
        tiled_layer*                layer_array;
        tiled_object*               object_array;
        tiled_property*             property_array;
        f64                         offset_x;
        f64                         offset_y;
        f64                         opacity;
        f64                         parallax_x;
        f64                         parallax_y;
        u32                         id;
        u32                         chunk_count;
        u32                         layer_count;
        u32                         object_count;
        u32                         property_count;
        u32                         height;
        u32                         width;
        u32                         image_height;
        u32                         image_width;
        u32                         start_x;
        u32                         start_y;
        u32                         x;
        u32                         y;
        color_rgba_u32              tint_color;
        color_rgba_u32              transparent_color;
        tiled_layer_compression_u32 compression;
        tiled_layer_mode_u32        mode;
        tiled_layer_draw_order_u32  draw_order;
        tiled_layer_encoding_u32    encoding;
        tiled_layer_type_u32        type;
        bool                        locked;
        bool                        repeat_x;
        bool                        repeat_y;
        bool                        visible;
    };
    
    //--------------------------------------------------------------------
    // CHUNK 
    //--------------------------------------------------------------------

    struct tiled_chunk {
        union {
            u32*   gid_array;
            cchar* base_64_encoded_cstr;
        } data;
        u32 width;
        u32 height;
        u32 x;
        u32 y;
    };
    
    //--------------------------------------------------------------------
    // OBJECT 
    //--------------------------------------------------------------------
    
    struct tiled_object {
        union {
            tiled_point* polygon;
            tiled_point* polyline;
        };
        cchar*          name;
        cchar*          type;
        tiled_property* property_array;
        tiled_text*     text;
        f64             width;
        f64             height;
        f64             x;
        f64             y;
        f64             opacity;
        f64             rotation;
        u32             gid;
        u32             id;
        u32             property_count;
        bool            capsule;
        bool            ellipse;
        bool            point;
        bool            visible;
    };
    
    //--------------------------------------------------------------------
    // TEXT 
    //--------------------------------------------------------------------

    enum tiled_text_horizontal_alignment_e {
        tiled_text_horizontal_alignment_e_center  = 0,
        tiled_text_horizontal_alignment_e_right   = 1,
        tiled_text_horizontal_alignment_e_justify = 2,
        tiled_text_horizontal_alignment_e_left    = 3
    };

    enum tiled_text_vertical_alignment_e {
        tiled_text_vertical_alignment_e_center = 0,
        tiled_text_vertical_alignment_e_bottom = 1,
        tiled_text_vertical_alignment_e_top    = 2,
    };

    using tiled_text_horizontal_alignment_u32 = u32;
    using tiled_text_vertical_alignment_u32   = u32;

    struct tiled_text {
        cchar*                              font_family;
        cchar*                              text;
        u32                                 pixel_size;
        color_rgba_u32                      color;
        tiled_text_horizontal_alignment_u32 h_align;
        tiled_text_vertical_alignment_u32   v_align;
        bool                                bold;
        bool                                italic;
        bool                                kerning;
        bool                                strikeout;
        bool                                underline;
        bool                                wrap;
    };
    
    //--------------------------------------------------------------------
    // TILESET 
    //--------------------------------------------------------------------

    enum tiled_tileset_fill_mode_e {
        tiled_tileset_fill_mode_e_stretch                = 0,
        tiled_tileset_fill_mode_e_perspectice_aspect_fit = 1,
    };
    
    enum tiled_tileset_object_alignment_e {
        tiled_tileset_objec_alignment_e_unspecified   = 0,    
        tiled_tileset_object_alignment_e_top_left     = 1,    
        tiled_tileset_object_alignment_e_top          = 2,    
        tiled_tileset_object_alignment_e_top_right    = 3,    
        tiled_tileset_object_alignment_e_left         = 4,    
        tiled_tileset_object_alignment_e_center       = 5,    
        tiled_tileset_object_alignment_e_right        = 6,    
        tiled_tileset_object_alignment_e_bottom_left  = 7,    
        tiled_tileset_object_alignment_e_bottom       = 8,    
        tiled_tileset_object_alignment_e_bottom_right = 9,    
    };
    
    enum tiled_tileet_render_size_e {
        tiled_tileset_render_size_e_tile = 0,
        tiled_tileset_render_size_e_grid = 1,
    };

    using tiled_tileset_fill_mode_u32         = u32;
    using tiled_tileset_object_alignment_u32 = u32;
    using tiled_tileet_render_size_u32       = u32;

    struct tiled_tileset {
        cchar*                             tileset_class;
        cchar*                             image;
        cchar*                             name;
        cchar*                             source;
        cchar*                             tiled_version;
        cchar*                             type;
        cchar*                             version;
        tiled_grid*                        grid;
        tiled_property*                    property_array;
        tiled_terrain*                     terrain_array;
        tiled_tile_offset*                 tile_offset;
        tiled_tile*                        tile_array;
        tiled_transformations*             transformation_array;
        tiled_wang_set*                    wang_set_array;
        color_rgba_u32                     background_color;
        color_rgba_u32                     transparent_color;
        u32                                columns;
        u32                                first_grid;
        u32                                image_height;
        u32                                image_width;
        u32                                margin;
        u32                                property_count;
        u32                                spacing;
        u32                                terrain_count;
        u32                                tile_count;
        u32                                tile_height;
        u32                                tile_width;
        u32                                transformation_count;
        u32                                wang_set_count;
        tiled_tileset_fill_mode_u32        fill_mode;        
        tiled_tileset_object_alignment_u32 object_alignment;
        tiled_tileet_render_size_u32       render_size;
    };
    
    //--------------------------------------------------------------------
    // GRID 
    //--------------------------------------------------------------------
    
    enum tiled_grid_orientation_e {
        tiled_grid_orientation_e_orthogonal = 0,
        tiled_grid_orientation_e_isometric  = 1
    };

    using tiled_grid_orientation_u32 = u32;

    struct tiled_grid {
        u32                        height;
        u32                        width;
        tiled_grid_orientation_u32 orientation;
    };
    
    //--------------------------------------------------------------------
    // TILE OFFSET 
    //--------------------------------------------------------------------

    struct tiled_offset {
        u32 x;
        u32 y;
    };
    
    //--------------------------------------------------------------------
    // TRANSFORMATION 
    //--------------------------------------------------------------------
    
    struct tiled_transformation {
        bool h_flip;
        bool v_flip;
        bool rotate;
        bool prefer_untransformed;
    }; 
    
    //--------------------------------------------------------------------
    // TILE
    //--------------------------------------------------------------------

    struct tiled_tile {
        cchar*          image;
        cchar*          type;
        tiled_frame*    frame_array;
        tiled_layer*    object_group;
        tiled_property* property_array;
        tiled_terrain*  terrain_array;
        f64             probability;
        u32             id;
        u32             image_height;
        u32             image_width;
        u32             x;
        u32             y;
        u32             width;
        u32             height;
        u32             frame_count;
        u32             property_count;
        u32             terrain_count;
    };

    //--------------------------------------------------------------------
    // FRAME 
    //--------------------------------------------------------------------
    
    struct tiled_frame {
        u32 duration;
        u32 tile_id;
    };

    //--------------------------------------------------------------------
    // TERRAIN 
    //--------------------------------------------------------------------
    
    struct tiled_terrain {
        cchar*          name;
        tiled_property* property_array;
        u32             property_count;
        u32             tile;
    };

    //--------------------------------------------------------------------
    // WANG SET 
    //--------------------------------------------------------------------
    
    enum tiled_wang_set_type_e {
        tiled_wang_set_type_e_corner = 0,
        tiled_wang_set_type_e_edge   = 1,
        tiled_wang_set_type_e_mixed  = 2
    };

    using tiled_wang_set_type_u32 = u32;

    struct tiled_wang_set {
        cchar*                  wang_set_class;
        tiled_wang_color*       wang_color_array;
        tiled_property*         property_array;
        tiled_wang_tile*        wang_tile_array;
        tiled_wang_set_type_u32 type;
        u32                     wang_color_count;
        u32                     property_count;
        u32                     tile;
    };

    //--------------------------------------------------------------------
    // WANG COLOR 
    //--------------------------------------------------------------------
    
    struct tiled_wang_color {
        cchar*          name;
        cchar*          tiled_wang_color_class;
        tiled_property* property_array;
        f64             probability;
        color_rgba_u32  color;
        u32             property_count;
        u32             tile;
    };

    //--------------------------------------------------------------------
    // WANG TILE 
    //--------------------------------------------------------------------
    
    struct tiled_wang_tile {
        u32  tile_id;
        byte wang_id[8];
    };

    //--------------------------------------------------------------------
    // OBJECT TEMPLATE
    //--------------------------------------------------------------------
    
    struct tiled_object_template {
        tiled_tileset* tile_set;
        tiled_object*  object;
    };
   
    //--------------------------------------------------------------------
    // PROPERTY 
    //--------------------------------------------------------------------
    
    enum tiled_property_type_e {
        tiled_property_type_e_string = 0, 
        tiled_property_type_e_int    = 1, 
        tiled_property_type_e_float  = 2, 
        tiled_property_type_e_bool   = 3, 
        tiled_property_type_e_color  = 4, 
        tiled_property_type_e_file   = 5, 
        tiled_property_type_e_object = 6, 
        tiled_property_type_e_class  = 7 
    };

    using tiled_property_type_u32 = u32;

    struct tiled_property {
        cchar*                  name;
        tiled_property_type_u32 type;
        cchar*                  property_type;
        union {
            u32    as_u32;
            s32    as_s32;
            u64    as_u64;
            s64    as_s64;
            f32    as_f32;
            f64    as_f64;
            cchar* as_cstr;
            void*  as_vptr;
        } value;
    };

    //--------------------------------------------------------------------
    // POINT 
    //--------------------------------------------------------------------
    
    struct tiled_point {
        f32 x;
        f32 y;
    };
};

#endif //TILED_HPP
