#pragma once

#include "renderer.hpp"

namespace ifb {
    
    IFB_INTERNAL void
    renderer_init_quad_shader(
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
        rndr->quad_shader.program = shader_prg;
    }

    
    IFB_INTERNAL quad_buffer*
    renderer_quad_buffer_create(
        renderer* rndr) {

        assert(rndr);

        const u32 mem_size = rndr->mem.granularity;
        void*     mem      = renderer_memory_commit(rndr);
        assert(mem);

        zero_memory(mem, mem_size);

        auto qb      = (quad_buffer*)mem;
        qb->ptr      = (quad*)((addr)mem + sizeof(quad_buffer));
        qb->capacity = (mem_size - sizeof(quad_buffer)) / sizeof(quad);  
        qb->count    = 0;
        qb->gl_buf   = gl_buffer_create(rndr->gl);

        assert(
            qb->ptr      != NULL &&
            qb->capacity != 0    &&
            qb->gl_buf   != GL_ID_INVALID
        );

        return(qb);
    }

    IFB_INTERNAL void
    renderer_quad_buffer_destroy(
        renderer*    rndr,
        quad_buffer* qb) {

        assert(
            rndr != NULL &&
            qb   != NULL &&
            gl_buffer_destroy(rndr->gl, qb->gl_buf);
        );

        renderer_memory_decommit(rndr, (void*)qb);
    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer*   rndr,
        const quad* q_ptr,
        const u32   q_count) {

        assert(
            rndr    != NULL &&
            q_ptr   != NULL &&
            q_count != 0
        );

        quad_shader& qs           = rndr->quad_shader;
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