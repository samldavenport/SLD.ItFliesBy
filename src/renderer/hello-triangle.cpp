#pragma once

#include "renderer.hpp"

namespace ifb {

    static constexpr f32 HELLO_TRIANGLE_VERTICES[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };


    IFB_INTERNAL void
    renderer_hello_triangle_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        auto& shdr = ctx->hello_triangle_shader;
        
        // create gl objects
        shdr.gl.program          = gl_shader_program_create        (ctx->gl);
        shdr.gl.vertex           = gl_vertex_create                (ctx->gl);
        shdr.gl.buf_vertex       = gl_buffer_create                (ctx->gl); 
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (ctx->gl);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (ctx->gl);

        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (ctx->gl, shdr_vtx, src_vertex.data, src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (ctx->gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (ctx->gl, shdr.gl.program,  shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (ctx->gl, shdr.gl.program,  shdr_frg);
        gl_ok &= gl_shader_program_link              (ctx->gl, shdr.gl.program);
        gl_shader_stage_destroy                      (ctx->gl, shdr_vtx);
        gl_shader_stage_destroy                      (ctx->gl, shdr_frg);
        assert(gl_ok);

        // define vertex
        gl_ok&= gl_context_set_vertex_object (ctx->gl, shdr.gl.vertex);
        gl_ok&= gl_context_set_buffer_vertex (ctx->gl, shdr.gl.buf_vertex);
        gl_ok&= gl_buffer_set_vertex_data    (ctx->gl, shdr.gl.buf_vertex, (const byte*)HELLO_TRIANGLE_VERTICES, sizeof(HELLO_TRIANGLE_VERTICES));
        gl_ok&= gl_vertex_add_attribute_f32x3(ctx->gl, shdr.gl.vertex, (3 * sizeof(float)), 0, NULL);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_hello_triangle_draw(
        renderer_context* ctx) {

        auto& shdr = ctx->hello_triangle_shader;
        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_draw_vertices      (ctx->gl, 3);
    }
};