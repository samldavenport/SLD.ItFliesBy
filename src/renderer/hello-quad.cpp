#pragma once

#include "renderer.hpp"

namespace ifb {

    static constexpr f32 HELLO_QUAD_VERTICES[] = {
        
        // first triangle
         0.1f,  0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top right
         0.1f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom right
        -0.1f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left
        -0.1f,  0.1f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left 
    };

    static constexpr u32 HELLO_QUAD_ELEMENTS[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    IFB_INTERNAL void
    renderer_hello_quad_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        assert(ctx);
        assert(sizeof(vec3)           == 12);
        assert(sizeof(color_rgba_f32) == 16);
        
        auto& shdr = ctx->hello_quad_shader;
        
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
        const u32   vertex_count           = 6;
        const byte* vertex_data_ptr        = (const byte*)HELLO_QUAD_VERTICES;
        const byte* vertex_elmnt_ptr       = (const byte*)HELLO_QUAD_ELEMENTS;
        const u32   vertex_data_size       = sizeof(HELLO_QUAD_VERTICES);
        const u32   vertex_elmnt_size      = sizeof(HELLO_QUAD_ELEMENTS);
        const u32   vertex_size            = 28; 
        const u32   vertex_offset_position = 0;
        const u32   vertex_offset_color    = vertex_offset_position + 12;
        gl_ok&= gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_ok&= gl_context_set_buffer_vertex  (ctx->gl, shdr.gl.buf_vertex);
        gl_ok&= gl_context_set_buffer_element (ctx->gl, shdr.gl.buf_element);
        gl_ok&= gl_buffer_set_vertex_data     (ctx->gl, shdr.gl.buf_vertex, vertex_data_ptr,  vertex_data_size);
        gl_ok&= gl_buffer_set_element_data    (ctx->gl, shdr.gl.buf_element,vertex_elmnt_ptr, vertex_elmnt_size);
        gl_ok&= gl_vertex_add_attribute_f32x3 (ctx->gl, shdr.gl.vertex,     vertex_size, 0,   vertex_offset_position);
        gl_ok&= gl_vertex_add_attribute_f32x4 (ctx->gl, shdr.gl.vertex,     vertex_size, 1,   vertex_offset_color);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_hello_quad_draw(
        renderer_context* ctx) {

        auto& shdr = ctx->hello_quad_shader;
        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (ctx->gl, 6);
    }
};