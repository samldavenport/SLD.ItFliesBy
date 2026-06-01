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
        const gl_shader  shader_vtx = gl_shader_stage_create_vertex   (ctx->gl);
        const gl_shader  shader_frg = gl_shader_stage_create_fragment (ctx->gl);
        const gl_program shader_prg = gl_shader_program_create        (ctx->gl);

        // compile and link shader pipeline
        bool shader_is_valid = true;
        shader_is_valid &= gl_shader_stage_compile_from_source (ctx->gl, shader_vtx, src_vertex.data, src_vertex.size);
        shader_is_valid &= gl_shader_stage_compile_from_source (ctx->gl, shader_frg, src_fragment.data, src_fragment.size);
        shader_is_valid &= gl_shader_program_attach_stage      (ctx->gl, shader_prg, shader_vtx);
        shader_is_valid &= gl_shader_program_attach_stage      (ctx->gl, shader_prg, shader_frg);
        shader_is_valid &= gl_shader_program_link              (ctx->gl, shader_prg);
        assert(shader_is_valid);

        // destroy shader stages
        gl_shader_stage_destroy (ctx->gl, shader_vtx);
        gl_shader_stage_destroy (ctx->gl, shader_frg);

        // update renderer
        ctx->quad_shader.program = shader_prg;
    }

    
    IFB_INTERNAL quad_buffer*
    renderer_quad_buffer_create(
        renderer_context* ctx) {

        assert(ctx);

        const u32 mem_size = ctx->mem.granularity;
        void*     mem      = renderer_memory_commit(ctx);
        assert(mem);

        zero_memory(mem, mem_size);

        auto qb      = (quad_buffer*)mem;
        qb->ptr      = (quad*)((addr)mem + sizeof(quad_buffer));
        qb->capacity = (mem_size - sizeof(quad_buffer)) / sizeof(quad);  
        qb->count    = 0;
        qb->gl_buf   = gl_buffer_create(ctx->gl);

        assert(
            qb->ptr      != NULL &&
            qb->capacity != 0    &&
            qb->gl_buf   != GL_ID_INVALID
        );

        return(qb);
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

        assert(
            ctx     != NULL &&
            q_ptr   != NULL &&
            q_count != 0
        );

        quad_shader& qs           = ctx->quad_shader;
        u32          count_pushed = 0;

        for (
            u32 index = qs.buffer->count;
                index < qs.buffer->capacity;
              ++index) {

            qs.buffer->ptr[index] = q_ptr[index];
            ++count_pushed;
        }

        return(count_pushed);
    }
};