#pragma once

#include "renderer.hpp"

namespace ifb {

    IFB_INTERNAL u32
    renderer_memory_requirement(
        void) {

        const u32 mem_req = (
            sizeof(renderer) +
            sizeof(gl_context)
        );

        return(mem_req);
    }

    IFB_INTERNAL renderer*
    renderer_init_from_memory(
        memory& mem) {

        assert(
            mem.size == renderer_memory_requirement() &&
            mem.ptr  != NULL
        );
        auto rndr =   (renderer*)mem.ptr;
        rndr->gl  = (gl_context*)(mem.address + sizeof(renderer));
        return(rndr);
    }

    IFB_INTERNAL void
    renderer_startup(
        renderer*  rndr,
        memory&    reserved_memory) {

        assert(
            rndr                 != NULL &&
            reserved_memory.size != 0    &&
            reserved_memory.ptr  != NULL          
        );

        rndr->reserved_memory = reserved_memory;

        pfm_graphics_init_opengl(rndr->gl);
    }

    IFB_INTERNAL void
    renderer_add_quad_shader(
        renderer*            rndr,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        assert(
            rndr              != NULL &&
            src_vertex.data   != NULL &&
            src_vertex.size   != 0    &&
            src_fragment.data != NULL &&
            src_fragment.size != 0
        );

        // create gl objects
        const gl_shader  shader_vtx = gl_shader_stage_create_vertex   (rndr->gl);
        const gl_shader  shader_frg = gl_shader_stage_create_fragment (rndr->gl);
        const gl_program shader_prg = gl_shader_program_create        (rndr->gl);

        // compile and link shader pipeline
        bool shader_is_valid = true;
        shader_is_valid &= gl_shader_stage_compile_from_source (rndr->gl, shader_vtx, src_vertex.data, src_vertex.size);
        shader_is_valid &= gl_shader_stage_compile_from_source (rndr->gl, shader_frg, src_fragment.data, src_fragment.size);
        shader_is_valid &= gl_shader_program_attach_stage      (rndr->gl, shader_prg, shader_vtx);
        shader_is_valid &= gl_shader_program_attach_stage      (rndr->gl, shader_prg, shader_frg);
        shader_is_valid &= gl_shader_program_link              (rndr->gl, shader_prg);
        assert(shader_is_valid);

        // destroy shader stages
        gl_shader_stage_destroy (rndr->gl, shader_vtx);
        gl_shader_stage_destroy (rndr->gl, shader_frg);

        // update renderer
        rndr->shader.quad = shader_prg;
    }

    IFB_INTERNAL void
    renderer_shutdown(
        renderer* rndr) {

        //TODO
    }
};