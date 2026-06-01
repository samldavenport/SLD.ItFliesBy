#pragma once

#include "renderer.hpp"

namespace ifb {
    
    IFB_INTERNAL void
    renderer_quad_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        assert(
            ctx              != NULL &&
            src_vertex.data   != NULL &&
            src_vertex.size   != 0    &&
            src_fragment.data != NULL &&
            src_fragment.size != 0
        );

        // create gl objects
        const gl_shader  shader_vtx   = gl_shader_stage_create_vertex   (ctx->gl);
        const gl_shader  shader_frg   = gl_shader_stage_create_fragment (ctx->gl);
        const gl_program shader_prg   = gl_shader_program_create        (ctx->gl);
        const gl_buffer  buffer_vtx   = gl_buffer_create                (ctx->gl);
        const gl_buffer  buffer_elmnt = gl_buffer_create                (ctx->gl);
        const gl_vertex  vertex       = gl_vertex_create                (ctx->gl); 
        assert(
            shader_vtx   != GL_ID_INVALID &&
            shader_frg   != GL_ID_INVALID &&
            shader_prg   != GL_ID_INVALID &&
            buffer_vtx   != GL_ID_INVALID &&
            buffer_elmnt != GL_ID_INVALID && 
            vertex       != GL_ID_INVALID 
        );

        // compile and link shader pipeline
        bool shader_is_valid = true;
        shader_is_valid &= gl_shader_stage_compile_from_source (ctx->gl, shader_vtx, src_vertex.data,   src_vertex.size);
        shader_is_valid &= gl_shader_stage_compile_from_source (ctx->gl, shader_frg, src_fragment.data, src_fragment.size);
        shader_is_valid &= gl_shader_program_attach_stage      (ctx->gl, shader_prg, shader_vtx);
        shader_is_valid &= gl_shader_program_attach_stage      (ctx->gl, shader_prg, shader_frg);
        shader_is_valid &= gl_shader_program_link              (ctx->gl, shader_prg);
        assert(shader_is_valid);

        // destroy shader stages
        gl_shader_stage_destroy (ctx->gl, shader_vtx);
        gl_shader_stage_destroy (ctx->gl, shader_frg);

        // allocate memory
        auto buffer_vertex  = (quad_vertices*)renderer_memory_commit(ctx);
        auto buffer_element = (quad_elements*)renderer_memory_commit(ctx);
        assert(
            buffer_vertex  != NULL &&
            buffer_element != NULL            
        );

        // update renderer
        ctx->quad_shader.quad_buffer_vertices = buffer_vertex;
        ctx->quad_shader.quad_buffer_elements = buffer_element;
        ctx->quad_shader.quad_capacity        = ctx->mem.granularity / sizeof(quad_vertices);  
        ctx->quad_shader.quad_count           = 0;
        ctx->quad_shader.gl.program           = shader_prg;
        ctx->quad_shader.gl.vertex            = vertex;
        ctx->quad_shader.gl.buf_vertex        = buffer_vtx;
        ctx->quad_shader.gl.buf_element       = buffer_elmnt;
    }

    IFB_INTERNAL void
    renderer_quad_buffer_destroy(
        renderer_context* ctx,
        quad_buffer*      qb) {

        assert(
            ctx != NULL &&
            qb  != NULL &&
            gl_buffer_destroy(ctx->gl, qb->gl_buf)
        );

        renderer_memory_decommit(ctx, (void*)qb);
    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer_context* ctx,
        const quad*       q_ptr,
        const u32         q_count) {

    }
};