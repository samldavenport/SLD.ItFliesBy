#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 QUAD_VERTEX_SIZE       = sizeof(f32) * 7;
    static constexpr u32 QUAD_VERTEX_COUNT      = 4;
    static constexpr u32 QUAD_DATA_SIZE         = QUAD_VERTEX_SIZE * 4; 
    static constexpr u32 QUAD_ELEMENT_COUNT     = 6;
    static constexpr u32 QUAD_ELEMENT_DATA_SIZE = sizeof(u32) * QUAD_ELEMENT_COUNT;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer_context;
    struct renderer_memory;
    struct shader_source;
    struct camera;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // renderer context
    IFB_INTERNAL u32               renderer_context_memory_requirement (void);
    IFB_INTERNAL renderer_context* renderer_context_init_from_memory   (memory&   mem);
    IFB_INTERNAL void              renderer_context_startup            (renderer_context* ctx, memory& reserved_memory);
    IFB_INTERNAL void              renderer_context_shutdown           (renderer_context* ctx);
    IFB_INTERNAL void              renderer_context_update_viewport    (renderer_context* ctx, const u32 width, const u32 height);
    IFB_INTERNAL void              renderer_context_gui                (renderer_context* ctx);
    
    // memory
    IFB_INTERNAL void* renderer_memory_commit               (renderer_context* ctx);
    IFB_INTERNAL void  renderer_memory_decommit             (renderer_context* ctx, void* mem);
    IFB_INTERNAL u32   renderer_memory_element_count        (renderer_context* ctx, const u32 element_size);

    // camera
    IFB_INTERNAL void renderer_camera_get_origin           (renderer_context* ctx, vec3&       origin);
    IFB_INTERNAL void renderer_camera_get_target           (renderer_context* ctx, vec3&       target);
    IFB_INTERNAL void renderer_camera_set_origin           (renderer_context* ctx, const vec3& origin);
    IFB_INTERNAL void renderer_camera_set_target           (renderer_context* ctx, const vec3& target);
    IFB_INTERNAL void renderer_camera_get_view             (renderer_context* ctx, view&       v);

    // hello quad
    IFB_INTERNAL void  renderer_hello_quad_shader_init      (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    IFB_INTERNAL void  renderer_hello_quad_draw             (renderer_context* ctx);

    // quad shader
    IFB_INTERNAL void  renderer_quad_shader_init            (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    IFB_INTERNAL u32   renderer_quad_push                   (renderer_context* ctx, const quad* q, const u32 count = 1);
    IFB_INTERNAL u32   renderer_quad_draw                   (renderer_context* ctx); 

    // direction gizmo
    IFB_INTERNAL void  renderer_direciton_gizmo_shader_init (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    IFB_INTERNAL void  renderer_direction_gizmo_draw        (renderer_context* ctx);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct renderer_memory : memory {
        u32 granularity;
        struct {
            u32* ids;
            u32  position;
            u32  capacity;
        } block_stack;
    };

    struct hello_quad_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_buffer  buf_element;
        } gl;
    };

    struct quad_vertex {
        union {
            struct {
                struct {
                    f32 x;  // 0
                    f32 y;  // 4
                    f32 z;  // 8
                } attrib_0_pos;
                struct {
                    f32 r; // 12
                    f32 g; // 16
                    f32 b; // 20
                    f32 a; // 24
                } attrib_1_color;
            };
            byte data[QUAD_VERTEX_SIZE];
        };
    };

    struct quad_vertices {
        union {
            struct {
                quad_vertex top_right;
                quad_vertex bottom_right;
                quad_vertex bottom_left;
                quad_vertex top_left;
            };
            quad_vertex array[QUAD_VERTEX_COUNT];
            byte        data [QUAD_DATA_SIZE];
        };
    };

    struct quad_elements {
        union {
            struct {
                struct {
                    u32 elmnt_0_index_0;
                    u32 elmnt_1_index_1;
                    u32 elmnt_2_index_3;
                } triangle_1;
                struct {
                    u32 elmnt_3_index_1;
                    u32 elmnt_4_index_2;
                    u32 elmnt_5_index_3;
                } triangle_2;
            };
            u32  array [QUAD_ELEMENT_COUNT];
            byte data  [QUAD_ELEMENT_DATA_SIZE];
        };
    };

    struct quad_buffers {
        u32 quad_capacity;
        u32 quad_count;
        struct {
            u32 size;
            union {
                quad_vertices* array;
                byte*          data;
                void*          vptr;
                addr           addr;
                f32*           floats;
            };
        } vertices;
        struct {
            u32 size;
            union {
                quad_elements* array;
                byte*          data;
                void*          vptr;
                addr           addr;
                u32*           uints;
            };
        } elements;
    };

    struct quad_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buffer_vtx;
            gl_buffer  buffer_elmnt;
            gl_shader  shdr_vtx;
            gl_shader  shdr_frg;
        } gl;
        quad_buffers buffers;
    };

    struct direction_gizmo_shader {
        gl_program program;
        gl_shader  vert_shdr;
        gl_shader  frag_shdr;
        gl_vertex  vertex;
        gl_uniform unif_mat4_view_proj;
        gl_uniform unif_mat4_model;
    };

    struct camera {
        vec3 origin;
        vec3 target;
    };

    struct renderer_context {
        gl_context*           gl;
        renderer_memory       mem;
        camera                cam;
        struct {
            hello_quad_shader      hello_quad;
            quad_shader            quad;
            direction_gizmo_shader direction_gizmo;
        } shader;
    };


    struct shader_source {
        const cchar8* data;
        u32           size;
    };
};

#endif //RENDERER_HPP