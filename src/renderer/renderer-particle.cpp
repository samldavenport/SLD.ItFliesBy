#pragma once

#include "renderer.hpp"

namespace ifb {

    struct renderer_particle_shader {

        struct {
            gl_program program;
            gl_vertex  vertex;
        } gl;
    };

    IFB_INTERNAL void
    renderer_particle_shader_create(
        void) {

    }

    IFB_INTERNAL void
    renderer_particle_shader_init(
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

    }

    IFB_INTERNAL void
    renderer_particle_system_push(
        void) {

    }
};
