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
    struct hello_triangle_shader;

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
    
    // hello quad
    void              renderer_hello_quad_shader_init     (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    void              renderer_hello_quad_draw            (renderer_context* ctx);

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

    struct renderer_context {
        gl_context*           gl;
        renderer_memory       mem;
        hello_quad_shader     hello_quad_shader;
    };

    struct shader_source {
        const cchar8* data;
        u32           size;
    };
};

#endif //RENDERER_HPP