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
    struct quad;
    struct quad_buffer;
    struct quad_shader;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // renderer context
    u32               renderer_context_memory_requirement  (void);
    renderer_context* renderer_context_init_from_memory    (memory&   mem);
    void              renderer_context_startup             (renderer_context* ctx, memory& reserved_memory);
    void              renderer_context_shutdown            (renderer_context* ctx);
    
    // memory
    void*             renderer_memory_commit               (renderer_context* ctx);
    void              renderer_memory_decommit             (renderer_context* ctx, void* mem);
    
    // quads
    void              renderer_quad_shader_init            (renderer_context* ctx, const shader_source& src_vertex, const shader_source& src_fragment);
    quad_buffer*      renderer_quad_buffer_create          (renderer_context* ctx);
    void              renderer_quad_buffer_destroy         (renderer_context* ctx, quad_buffer* qb);
    u32               renderer_quad_push                   (renderer_context* ctx, const quad* q_ptr, const u32 q_count = 1);

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
        f32            scale;
        color_rgba_u32 color;
    };

    struct quad_buffer {
        quad*     ptr;
        u32       capacity;
        u32       count;
        gl_buffer gl_buf;
        gl_vertex gl_vtx;
    };

    struct quad_shader {
        gl_program   program;
        quad_buffer* buffer;

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