#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "quad.hpp"
namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 QUAD_ELEMENT_COUNT     = 6;
    static constexpr u32 QUAD_ELEMENT_DATA_SIZE = sizeof(u32) * QUAD_ELEMENT_COUNT;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer_context;
    struct renderer_memory;
    struct renderer_shader_source;
    struct renderer_camera;
    struct renderer_direction_gizmo_shader;
    struct renderer_camera;
    struct renderer_quad_shader;
    struct renderer_quad_buffers;
    struct renderer_quad_vertices;
    struct renderer_quad_elements;
    struct renderer_projection;
    struct renderer_grid_shader;
    struct renderer_tile_vertex;
    struct renderer_tile_shader;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static renderer_context* _renderer_ctx;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // renderer context
    IFB_INTERNAL renderer_context* renderer_context_create                   (void);
    IFB_INTERNAL void              renderer_context_startup                  (memory& reserved_memory);
    IFB_INTERNAL void              renderer_context_shutdown                 (void);
    IFB_INTERNAL void              renderer_context_update_view_matrix       (void);
    IFB_INTERNAL void*             renderer_context_memory_alloc             (const u32 size);
    IFB_INTERNAL void              renderer_context_draw_buffers             (void);
    IFB_INTERNAL mat4              renderer_context_view_projection_xform    (void);

    // camera
    IFB_INTERNAL void renderer_camera_init                   (void);
    IFB_INTERNAL void renderer_camera_get_origin             (vec3& origin);
    IFB_INTERNAL void renderer_camera_get_target             (vec3& target);
    IFB_INTERNAL void renderer_camera_get_forward            (vec3& forward);
    IFB_INTERNAL void renderer_camera_get_right              (vec3& right);
    IFB_INTERNAL void renderer_camera_get_up                 (vec3& up);
    IFB_INTERNAL void renderer_camera_get_view               (mat4& v);
    IFB_INTERNAL void renderer_camera_set_origin             (const vec3& origin);
    IFB_INTERNAL void renderer_camera_set_target             (const vec3& target);
    IFB_INTERNAL void renderer_camera_get_orientation        (orientation& o);
    IFB_INTERNAL mat4 renderer_camera_xform                  (void);

    // projection
    IFB_INTERNAL void renderer_projection_init               (void);
    IFB_INTERNAL void renderer_projection_set_viewport       (const u32 width, const u32 height);
    IFB_INTERNAL f32  renderer_projection_get_aspect_ratio   (void);
    IFB_INTERNAL mat4 renderer_projection_xform              (void);

    // hello quad
    IFB_INTERNAL void renderer_quad_shader_create            (void);
    IFB_INTERNAL void renderer_quad_shader_init              (const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL bool renderer_quad_push                     (const entity_id id);
    IFB_INTERNAL void renderer_quad_draw                     (const mat4& view_proj_xform);
    IFB_INTERNAL bool renderer_quad_get_vertices             (renderer_quad_vertices& vertices, const entity_id quad_id);

    // direction gizmo
    IFB_INTERNAL void renderer_direciton_gizmo_shader_create (void);
    IFB_INTERNAL void renderer_direciton_gizmo_shader_init   (const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void renderer_direction_gizmo_draw          (void);

    // grid
    IFB_INTERNAL void renderer_grid_shader_create            (void);
    IFB_INTERNAL void renderer_grid_shader_init              (const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void renderer_grid_draw                     (const mat4& view_proj_xform);

    // tile
    IFB_INTERNAL void renderer_tile_shader_create            (void);
    IFB_INTERNAL void renderer_tile_shader_init              (const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void renderer_tile_shader_set_tile_size     (const f32 width,      const f32 height); 
    IFB_INTERNAL void renderer_tile_shader_set_map_size      (const f32 count_rows, const f32 count_cols); 
    IFB_INTERNAL void renderer_tile_draw                     (const mat4& view_proj_xform);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------
    
    struct renderer_memory {
        stack stack;
    };

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

    struct renderer_context {
        gl_context*          gl;
        renderer_memory      memory;
        renderer_camera*     cam;
        renderer_projection* proj;
        vec3                 global_up;
        struct {
            renderer_quad_shader*            quad;
            renderer_direction_gizmo_shader* direction_gizmo;
            renderer_grid_shader*            grid; 
            renderer_tile_shader*            tile;
        } shader;
    };
};

#endif //RENDERER_HPP
