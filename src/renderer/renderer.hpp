#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr f32 QUAD_VERTEX_BASE_COORDS[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    static constexpr unsigned int QUAD_BASE_INDICES[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer_context;
    struct renderer_memory;
    struct shader_source;
    struct quad;
    struct quad_buffer;
    struct quad_shader;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // renderer context
    u32               renderer_context_memory_requirement (void);
    renderer_context* renderer_context_init_from_memory   (memory&   mem);
    void              renderer_context_startup            (renderer_context* ctx, memory& reserved_memory);
    void              renderer_context_shutdown           (renderer_context* ctx);
    
    // memory
    void*             renderer_memory_commit              (renderer_context* ctx);
    void              renderer_memory_decommit            (renderer_context* ctx, void* mem);
    
    // quads
    void              renderer_quad_shader_init           (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    quad_buffer*      renderer_quad_buffer_create         (renderer_context* ctx);
    void              renderer_quad_buffer_destroy        (renderer_context* ctx, quad_buffer* qb);
    u32               renderer_quad_push                  (renderer_context* ctx, const quad* q_ptr, const u32 q_count = 1);

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

    struct quad {
        vec3           pos;
        f32            width;
        f32            height;
        color_rgba_u32 color;
        f32            scale;
    };


    struct quad_buffer {
        quad*     ptr;
        u32       capacity;
        u32       count;
        gl_buffer gl_buf;
        gl_vertex gl_vtx;
    };


    struct quad_vertex {
        union {
            struct {
                vec3           position;
                color_rgba_f32 color;
                f32            scale;
            };
            byte data[32];
        };
    };

    struct quad_elements {
        union {
            struct {
                u32 a;
                u32 b;
                u32 c;
            } triangle_1;
            struct {
                u32 a;
                u32 b;
                u32 c;
            } triangle_2;
            byte data[24];
        };
    };

    struct quad_vertices {
        quad_vertex top_right;
        quad_vertex bottom_right;
        quad_vertex bottom_left;
        quad_vertex top_left;
    };

    struct quad_shader {
        quad_vertices* quad_buffer_vertices;
        quad_elements* quad_buffer_elements;
        u32            quad_count;
        u32            quad_capacity;
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_buffer  buf_element;
        } gl;
    };

    struct renderer_context {
        gl_context*     gl;
        renderer_memory mem;
        quad_shader     quad_shader;
    };

    struct shader_source {
        const cchar8* data;
        u32           size;
    };
};

#endif //RENDERER_HPP