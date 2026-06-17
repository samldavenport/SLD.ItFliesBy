#pragma once

#include "renderer.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline void quad_shader_commit_memory            (renderer_context* ctx);
    inline void quad_shader_validate                 (const quad_shader& shdr);    
    inline void quad_shader_create_gl_objects        (quad_shader& shdr, gl_context* gl);
    inline void quad_shader_compile_and_link_program (quad_shader& shdr, gl_context* gl, const shader_source& src_vertex, const shader_source& src_fragment);
    inline void quad_shader_define_vertex            (quad_shader& shdr, gl_context* gl);


    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_quad_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        quad_shader_create_gl_objects        (ctx->quad_shader, ctx->gl);
        quad_shader_compile_and_link_program (ctx->quad_shader, ctx->gl, src_vertex, src_fragment);
        quad_shader_define_vertex            (ctx->quad_shader, ctx->gl);
    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer_context* ctx,
        const quad*       q,
        const u32         count) {

        // validate args
        assert(ctx != NULL && q != 0);


        return(0);
    }

    IFB_INTERNAL u32
    renderer_quad_draw(
        renderer_context* ctx) {

        assert(ctx);

        // validate shader
        auto& shdr = ctx->quad_shader;
        quad_shader_validate(shdr);

        // calculate counts
        const u32 quad_count    = shdr.buffer_vtx.count;
        const u32 element_count = shdr.buffer_vtx.count * QUAD_ELEMENT_COUNT;

        // draw elements
        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (ctx->gl, element_count);

        // reset counts and return
        shdr.buffer_vtx.count   = 0;
        shdr.buffer_elmnt.count = 0;
        return(quad_count);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline void
    quad_shader_commit_memory(
        renderer_context* ctx) {

        // commit memory
        void* mem = renderer_memory_commit(ctx);

        const u32 size_mem          = ctx->mem.granularity;
        const u32 size_per_quad     = (QUAD_DATA_SIZE + QUAD_ELEMENT_DATA_SIZE);
        const u32 count_quads_total = (size_mem / size_per_quad);
        const u32 

    }

    inline void
    quad_shader_validate(
        const quad_shader& shdr) {

        assert(
            shdr.gl.program      != GL_ID_INVALID &&
            shdr.gl.vertex       != GL_ID_INVALID &&
            shdr.gl.buffer_vtx   != GL_ID_INVALID &&
            shdr.gl.buffer_elmnt != GL_ID_INVALID
        );
    } 

    inline void
    quad_shader_create_gl_objects(
        quad_shader& shdr,
        gl_context*  gl) {
        
        shdr.gl.program      = gl_shader_program_create        (gl);
        shdr.gl.vertex       = gl_vertex_create                (gl);
        shdr.gl.buffer_vtx   = gl_buffer_create                (gl);
        shdr.gl.buffer_elmnt = gl_buffer_create                (gl);
        shdr.gl.shdr_vtx     = gl_shader_stage_create_vertex   (gl);
        shdr.gl.shdr_frg     = gl_shader_stage_create_fragment (gl);

        assert(
            shdr.gl.program      != GL_ID_INVALID &&
            shdr.gl.vertex       != GL_ID_INVALID &&
            shdr.gl.buffer_vtx   != GL_ID_INVALID &&
            shdr.gl.buffer_elmnt != GL_ID_INVALID &&
            shdr.gl.shdr_vtx     != GL_ID_INVALID &&
            shdr.gl.shdr_frg     != GL_ID_INVALID
        );
    }

    inline void
    quad_shader_compile_and_link_program(
        quad_shader&         shdr,
        gl_context*          gl,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        bool gl_ok = true;

        // compile and link program
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr.gl.shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr.gl.shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr.gl.program,  shdr.gl.shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr.gl.program,  shdr.gl.shdr_frg);
        gl_ok &= gl_shader_program_link              (gl, shdr.gl.program);
        assert(gl_ok);

        // destroy the shader stages
        gl_shader_stage_destroy (gl, shdr.gl.shdr_vtx);
        gl_shader_stage_destroy (gl, shdr.gl.shdr_frg);
        shdr.gl.shdr_vtx = GL_ID_INVALID;
        shdr.gl.shdr_frg = GL_ID_INVALID;
    }

    inline void
    quad_shader_define_vertex(
        quad_shader& shdr,
        gl_context*  gl) {

        bool gl_ok = true;

        // calcualte sizes
        const u32 size_vtx       = sizeof(quad_vertex);
        const u32 size_buf_vtx   = shdr.buffer_vtx.capacity  * size_vtx;
        const u32 size_buf_elmnt = shdr.buffer_elmnt.capacity * sizeof(u32);

        // set context objects
        gl_ok &= gl_context_set_vertex_object  (gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr.gl.buffer_vtx);
        gl_ok &= gl_context_set_buffer_element (gl, shdr.gl.buffer_elmnt);
        
        // set data
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr.gl.buffer_vtx,   (byte*)shdr.buffer_vtx.data,   size_buf_vtx);
        gl_ok &= gl_buffer_set_element_data    (gl, shdr.gl.buffer_elmnt, (byte*)shdr.buffer_elmnt.data, size_buf_elmnt);
        
        // set attributes
        gl_ok &= gl_vertex_add_attribute_f32x3 (gl, shdr.gl.vertex, size_vtx, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (gl, shdr.gl.vertex, size_vtx, 1, 12);
    }
};