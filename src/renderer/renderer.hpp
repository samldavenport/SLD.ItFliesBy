#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer;
    struct renderer_memory;
    struct shader_source;
    struct quad;
    struct quad_buffer;
    struct quad_shader;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // renderer context
    u32          renderer_memory_requirement  (void);
    renderer*    renderer_init_from_memory    (memory&   mem);
    void         renderer_startup             (renderer* rndr, memory& reserved_memory);
    void         renderer_shutdown            (renderer* rndr);
    
    // memory
    void*        renderer_memory_commit       (renderer* rndr);
    void         renderer_memory_decommit     (renderer* rndr, void* mem);
    
    // quads
    void         renderer_quad_shader_init    (renderer* rndr, const shader_source& src_vertex, const shader_source& src_fragment);
    quad_buffer* renderer_quad_buffer_create  (renderer* rndr);
    void         renderer_quad_buffer_destroy (renderer* rndr, quad_buffer* qb);
    u32          renderer_quad_push           (renderer* rndr, const quad* q_ptr, const u32 q_count = 1);

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
    };

    struct quad_shader {
        gl_program   program;
        quad_buffer* buffer;

    };

    struct renderer {
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