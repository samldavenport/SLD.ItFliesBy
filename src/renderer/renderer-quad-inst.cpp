#pragma once

#include "renderer.hpp"
#include "sld-opengl.hpp"

namespace ifb {

    struct renderer_quad_inst_shader {
        struct {
            gl_program program;
            gl_uniform u_offsets;
        } gl;
    };

    struct quad_inst_vertex {
        vec2 position;
        vec3 color;
    };

    static vec2 _offsets[100];

    const f32 QUAD_INST_VERTICES [] = {
        // positions     // colors
        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,   
         0.05f,  0.05f,  0.0f, 1.0f, 1.0f		    		
    }; 

    IFB_INTERNAL void
    renderer_quad_inst_shader_create(
        void) {

        auto shdr = (renderer_quad_inst_shader*)renderer_context_memory_alloc(sizeof(renderer_quad_inst_shader)); 

        assert(shdr);

        _renderer_ctx->shader.quad_inst = shdr;
    }

    IFB_INTERNAL void
    renderer_quad_inst_shader_init(
        const renderer_shader_source& src_vert,
        const renderer_shader_source& src_frag) {

        assert(_renderer_ctx);
        assert(src_vert.size != 0);
        assert(src_vert.data != NULL);
        assert(src_frag.size != 0);
        assert(src_frag.data != NULL);

        auto shdr = _renderer_ctx->shader.quad_inst; 
        auto gl   = _renderer_ctx->gl;
        assert(shdr);
        assert(gl);

        shdr->gl.program          = gl_shader_program_create        (gl);
        const gl_shader shdr_vert = gl_shader_stage_create_vertex   (gl);
        const gl_shader shdr_frag = gl_shader_stage_create_fragment (gl);

        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_vert, src_vert.data, src_vert.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_frag, src_frag.data, src_frag.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program,  shdr_vert);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program,  shdr_frag);
        gl_ok &= gl_shader_program_link              (gl, shdr->gl.program);
        gl_shader_stage_destroy                      (gl, shdr_vert);
        gl_shader_stage_destroy                      (gl, shdr_frag);
        assert(gl_ok);

        shdr->gl.u_offsets = gl_uniform_get_location(gl, shdr->gl.program, "u_offsets");
        gl_ok &= (shdr->gl.u_offsets != GL_UNIFORM_INVALID); 

        // set the offsets
        u32       index  = 0;
        const f32 stride = 0.1f;
        for (u32 y = -10; y < 10; y+=2) {
            for (u32 x = -10; x < 10; x+=2) {
           
                auto& offset = _offsets[index++];
                offset.x = (f32)x / 10.0f + stride;
                offset.x = (f32)x / 10.0f + stride;
            }
        }
    }

    IFB_INTERNAL void
    renderer_quad_inst_shader_draw(
        void) {

    }
};
