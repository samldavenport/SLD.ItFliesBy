#pragma once

#include "renderer.hpp"

namespace ifb {

    static constexpr f32 QUAD_VERTICES[] = {
        
        //position---------|color-----------------|
         0.1f,  0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top right
         0.1f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom right
        -0.1f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
        -0.1f,  0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
    };

    static constexpr u32 QUAD_ELEMENTS[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    IFB_INTERNAL void
    renderer_quad_shader_init(
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(_renderer_ctx);
        assert(sizeof(vec3)           == 12);
        assert(sizeof(color_rgba_f32) == 16);

        auto& shdr = _renderer_ctx->shader.quad;

        for (
            u32 i = 0;
            i < 28;
            ++i) {

            shdr.buffers.vertices.floats[i] = QUAD_VERTICES[i];
        }

        for (
            u32 i = 0;
            i < 6;
            ++i) {

            shdr.buffers.elements.uints[i] = QUAD_ELEMENTS[i]; 
        }
        
        // create gl objects
        shdr.gl.program          = gl_shader_program_create        (_renderer_ctx->gl);
        shdr.gl.vertex           = gl_vertex_create                (_renderer_ctx->gl);
        shdr.gl.buf_vertex       = gl_buffer_create                (_renderer_ctx->gl); 
        shdr.gl.buf_element      = gl_buffer_create                (_renderer_ctx->gl);
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (_renderer_ctx->gl);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (_renderer_ctx->gl);

        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (_renderer_ctx->gl, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (_renderer_ctx->gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (_renderer_ctx->gl, shdr.gl.program,  shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (_renderer_ctx->gl, shdr.gl.program,  shdr_frg);
        gl_ok &= gl_shader_program_link              (_renderer_ctx->gl, shdr.gl.program);
        gl_shader_stage_destroy                      (_renderer_ctx->gl, shdr_vtx);
        gl_shader_stage_destroy                      (_renderer_ctx->gl, shdr_frg);
        assert(gl_ok);

        // define vertex
        const u32 vertex_size  = sizeof(vec3) + sizeof(vec4); 
        gl_ok &= gl_context_set_vertex_object  (_renderer_ctx->gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (_renderer_ctx->gl, shdr.gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (_renderer_ctx->gl, shdr.gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (_renderer_ctx->gl, shdr.gl.buf_vertex,  shdr.buffers.vertices.data, shdr.buffers.vertices.size);
        gl_ok &= gl_buffer_set_element_data    (_renderer_ctx->gl, shdr.gl.buf_element, shdr.buffers.elements.data, shdr.buffers.elements.size);
        gl_ok &= gl_vertex_add_attribute_f32x3 (_renderer_ctx->gl, shdr.gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (_renderer_ctx->gl, shdr.gl.vertex, vertex_size, 1, 12);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_quad_draw(
        void) {

        auto& shdr = _renderer_ctx->shader.quad;
        gl_context_set_shader_program (_renderer_ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (_renderer_ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (_renderer_ctx->gl, 6);
    }
};