#pragma once

#include "renderer.hpp"

namespace ifb {

    static constexpr f32 HELLO_QUAD_VERTICES[] = {
        
        //position---------|color-----------------|
         0.1f,  0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top right
         0.1f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom right
        -0.1f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
        -0.1f,  0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
    };

    static constexpr u32 HELLO_QUAD_ELEMENTS[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    static quad_buffers _quad_buffers;

    IFB_INTERNAL void
    renderer_hello_quad_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        assert(ctx);
        assert(sizeof(vec3)           == 12);
        assert(sizeof(color_rgba_f32) == 16);
        
        _quad_buffers.quad_capacity   = 1;
        _quad_buffers.quad_count      = 1;
        _quad_buffers.vertices.floats = (f32*)HELLO_QUAD_VERTICES;
        _quad_buffers.vertices.size   = sizeof(HELLO_QUAD_VERTICES); 
        _quad_buffers.elements.uints  = (u32*)HELLO_QUAD_ELEMENTS;
        _quad_buffers.elements.size   = sizeof(HELLO_QUAD_ELEMENTS);

        auto& shdr = ctx->shader.hello_quad;
        
        // create gl objects
        shdr.gl.program          = gl_shader_program_create        (ctx->gl);
        shdr.gl.vertex           = gl_vertex_create                (ctx->gl);
        shdr.gl.buf_vertex       = gl_buffer_create                (ctx->gl); 
        shdr.gl.buf_element      = gl_buffer_create                (ctx->gl);
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
        const u32 vertex_count = 6;
        const u32 vertex_size  = sizeof(quad_vertex); 
        gl_ok &= gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (ctx->gl, shdr.gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (ctx->gl, shdr.gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (ctx->gl, shdr.gl.buf_vertex,  _quad_buffers.vertices.data, _quad_buffers.vertices.size);
        gl_ok &= gl_buffer_set_element_data    (ctx->gl, shdr.gl.buf_element, _quad_buffers.elements.data, _quad_buffers.elements.size);
        gl_ok &= gl_vertex_add_attribute_f32x3 (ctx->gl, shdr.gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (ctx->gl, shdr.gl.vertex, vertex_size, 1, 12);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_hello_quad_draw(
        renderer_context* ctx) {

        auto& shdr = ctx->shader.hello_quad;
        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (ctx->gl, 6);
    }
};