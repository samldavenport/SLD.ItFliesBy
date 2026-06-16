#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer_context;
    struct renderer_memory;
    struct shader_source;
    struct quad_buffer;
    struct quad_data;
    struct quad_shader;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // renderer context
    IFB_INTERNAL u32               renderer_context_memory_requirement (void);
    IFB_INTERNAL renderer_context* renderer_context_init_from_memory   (memory&   mem);
    IFB_INTERNAL void              renderer_context_startup            (renderer_context* ctx, memory& reserved_memory);
    IFB_INTERNAL void              renderer_context_shutdown           (renderer_context* ctx);
    
    // memory
    IFB_INTERNAL void* renderer_memory_commit        (renderer_context* ctx);
    IFB_INTERNAL void  renderer_memory_decommit      (renderer_context* ctx, void* mem);
    IFB_INTERNAL u32   renderer_memory_element_count (renderer_context* ctx, const u32 element_size);

    // hello quad
    IFB_INTERNAL void renderer_hello_quad_shader_init (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    IFB_INTERNAL void renderer_hello_quad_draw        (renderer_context* ctx);

    // quad shader
    IFB_INTERNAL void renderer_quad_shader_init (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    IFB_INTERNAL u32  renderer_quad_push        (renderer_context* ctx, const quad* q, const u32 count = 1);
    IFB_INTERNAL u32  renderer_quad_draw        (renderer_context* ctx); 

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

    #pragma pack(push, 1)
    struct quad_vertex {
        union {
            struct {
                f32 pos_x;
                f32 pos_y;
                f32 pos_z;
                f32 color_r;
                f32 color_g;
                f32 color_b;
                f32 color_a;
            };
            byte data[sizeof(f32) * 7];
        };
    };
    #pragma pack(pop)

    struct quad_vertex_buffer {
        u32          capacity;
        u32          count;
        quad_vertex* elements;
    };

    struct quad_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_buffer  buf_element;
        } gl;
        quad_vertex_buffer vtx_buffer;
    };

    struct renderer_context {
        gl_context*           gl;
        renderer_memory       mem;
        hello_quad_shader     hello_quad_shader;
        quad_shader           quad_shader;
    };

    struct shader_source {
        const cchar8* data;
        u32           size;
    };
};

#endif //RENDERER_HPP