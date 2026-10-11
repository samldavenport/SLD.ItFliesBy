#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "quad.hpp"
#include "map.hpp"
namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 QUAD_ELEMENT_COUNT     = 6;
    static constexpr u32 QUAD_ELEMENT_DATA_SIZE = sizeof(u32) * QUAD_ELEMENT_COUNT;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer;
    struct renderer_shader_source;
    struct renderer_quad_buffers;
    struct renderer_quad_vertices;
    struct renderer_quad_elements;
    struct renderer_tile_vertex;
    struct renderer_tile_shader;
    struct renderer_particle_shader;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static renderer* _renderer;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL renderer*   renderer_create                    (void);
    IFB_INTERNAL void        renderer_startup                   (reservation* res);
    IFB_INTERNAL void        renderer_shutdown                  (void);
    IFB_INTERNAL void        renderer_update_view_matrix        (void);
    IFB_INTERNAL void        renderer_draw_buffers              (void);
    IFB_INTERNAL void        renderer_view_projection_xform     (mat4& xform);
    IFB_INTERNAL bool        renderer_push_quad                 (const entity_id id);
    IFB_INTERNAL bool        renderer_set_map                   (const hnd_map h_map);
    IFB_INTERNAL const vec3& renderer_get_camera_origin         (void);
    IFB_INTERNAL const vec3& renderer_get_camera_target         (void);
    IFB_INTERNAL void        renderer_get_camera_orientation    (orientation& o);
    IFB_INTERNAL void        renderer_get_camera_xform          (mat4& xform);
    IFB_INTERNAL void        renderer_set_camera_origin         (const vec3& origin);
    IFB_INTERNAL void        renderer_set_camera_target         (const vec3& target);
    IFB_INTERNAL void        renderer_set_viewport_dimensions   (const u32 width, const u32 height);
    IFB_INTERNAL f32         renderer_get_viewport_aspect_ratio (void);
    IFB_INTERNAL void        renderer_get_viewport_xform        (mat4& xform);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------
    
    struct renderer_shader_source {
        const cchar* data;
        u32          size;
    };

    struct renderer_quad_elements {
        u32 elmnt_0_index_0;
        u32 elmnt_1_index_1;
        u32 elmnt_2_index_3;
        u32 elmnt_3_index_1;
        u32 elmnt_4_index_2;
        u32 elmnt_5_index_3;
    };

    struct renderer_quad_vertex {
        f32 pos_x;
        f32 pos_y;
        f32 pos_z;
        f32 color_r;
        f32 color_g;
        f32 color_b;
        f32 color_a;
    };

    struct renderer_quad_vertices {
        union {
            struct {
                renderer_quad_vertex top_right;
                renderer_quad_vertex bottom_right;
                renderer_quad_vertex bottom_left;
                renderer_quad_vertex top_left;
            };
            byte bytes  [112];
            f32  floats [28];
        };
    };

    struct renderer_quad_vertex_buffer {
        u32 size;
        union {
            renderer_quad_vertices* vertices;
            byte*                   bytes;
            void*                   vptr;
            addr                    addr;
            f32*                    floats;
        } data;
    };

    struct renderer_quad_element_buffer {
        u32 size;
        union {
            renderer_quad_elements* elements;
            byte*                   bytes;
            void*                   vptr;
            addr                    addr;
            u32*                    uints;
        } data;
    };

};

#endif //RENDERER_HPP
