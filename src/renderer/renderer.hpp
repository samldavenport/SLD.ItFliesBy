#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb.hpp"

namespace ifb {

    struct renderer {
        gl_context* gl;
        memory      reserved_memory;
        struct {
            gl_program quad;
        } shader;
    };

    struct shader_source {
        const cchar8* data;
        u32           size;
    };

    u32       renderer_memory_requirement (void);
    renderer* renderer_init_from_memory   (memory&   mem);
    void      renderer_startup            (renderer* rndr, memory& reserved_memory);
    void      renderer_add_quad_shader    (renderer* rndr, const shader_source& src_vertex, const shader_source& src_fragment);
    void      renderer_shutdown           (renderer* rndr);

};

#endif //RENDERER_HPP