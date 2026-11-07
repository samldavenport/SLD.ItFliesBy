#pragma once

#include "eng-renderer.hpp"
#include "sld-opengl.hpp"

namespace ifb {

    struct renderer_parallax_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
        } gl;
    };

    IFB_INTERNAL renderer_parallax_shader*
    renderer_parallax_shader_create(
        void) {

    }

    IFB_INTERNAL renderer_parallax_shader*
    renderer_parallax_shader_init(
        renderer_parallax_shader*     shdr,
        gl_context*                   gl,
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

    }

    IFB_INTERNAL renderer_parallax_shader*
    renderer_parallax_shader_draw(
        renderer_parallax_shader* shdr,
        gl_context*               gl) {

    }
};
