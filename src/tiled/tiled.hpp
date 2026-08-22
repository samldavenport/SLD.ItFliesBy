#ifndef TILED_HPP
#define TILED_HPP

#include "ifb.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct tiled_map;
    struct tiled_layer;
    struct tiled_tile_layer;
    struct tiled_object_layer;
    struct tiled_object; 
    struct tiled_object_ellipse;
    struct tiled_object_rectangle;
    struct tiled_object_point;
    struct tiled_object_polygon;
    struct tiled_object_polyline;
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
    // MAP 
    //--------------------------------------------------------------------

    enum tiled_map_orientation_e {
        tiled_map_orientation_e_orthogonal = 0,
        tiled_map_orientation_e_isometric  = 1,
        tiled_map_orientation_e_oblique    = 2,
        tiled_map_orientation_e_staggered  = 3,
        tiled_map_orientation_e_hexagonal  = 4
    };
    using tiled_map_orientation_u32 = u32;

    enum tiled_map_render_order_e {
        tiled_map_render_order_e_right_down = 0, 
        tiled_map_render_order_e_right_up   = 1, 
        tiled_map_render_order_e_left_down  = 2, 
        tiled_map_render_order_e_left_up    = 3
    };
    using tiled_map_render_order_u32 = u32;

    struct tiled_map {
        cchar*                     map_class;
        tiled_layer*               layer_array;
        tiled_property*            property_array; 
        u32                        property_count;
        color_rgba_u32             background_color;
        u32                        layer_count;
        u32                        compression_level;
        u32                        height;
        u32                        hex_side_length;
        bool                       infinite;
        u32                        next_layer_id;
        u32                        next_object_id;
        u32                        parallax_origin_x;
        u32                        parallax_origin_y;
        tiled_map_orientation_u32  orientation;
        tiled_map_render_order_u32 render_order;
    };
};

#endif //TILED_HPP
