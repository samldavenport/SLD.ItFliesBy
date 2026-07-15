#pragma once

#include "renderer.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline void quad_shader_commit_memory            (renderer_context* _renderer_ctx);
    inline void quad_shader_validate                 (const quad_shader& shdr);
    inline void quad_shader_create_gl_objects        (quad_shader& shdr, gl_context* gl);
    inline void quad_shader_compile_and_link_program (quad_shader& shdr, gl_context* gl, const shader_source& src_vertex, const shader_source& src_fragment);
    inline void quad_shader_define_vertex            (quad_shader& shdr, gl_context* gl);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_quad_shader_init(
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        quad_shader_commit_memory            (_renderer_ctx);
        quad_shader_create_gl_objects        (_renderer_ctx->shader.quad, _renderer_ctx->gl);
        quad_shader_compile_and_link_program (_renderer_ctx->shader.quad, _renderer_ctx->gl, src_vertex, src_fragment);
        quad_shader_define_vertex            (_renderer_ctx->shader.quad, _renderer_ctx->gl);
    }


    IFB_INTERNAL u32
    renderer_quad_draw(
        void) {

        assert(_renderer_ctx);

        // validate shader
        auto& shdr = _renderer_ctx->shader.quad;
        quad_shader_validate(shdr);

        // calculate counts
        const u32 quad_count    = shdr.buffers.quad_count;
        const u32 element_count = quad_count * QUAD_ELEMENT_COUNT;

        // draw elements
        gl_context_set_shader_program (_renderer_ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (_renderer_ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (_renderer_ctx->gl, element_count);

        // reset counts and return
        shdr.buffers.quad_count = 0;
        return(quad_count);
    }

    IFB_INTERNAL void
    renderer_quad_draw_buffer(
        const quad_vertex_buffer* vtx_buffer) {

        assert(
            vtx_buffer               != NULL &&
            vtx_buffer->data.bytes   != NULL &&
            vtx_buffer->vertex_count != 0
        );
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline void
    quad_shader_commit_memory(
        renderer_context* _renderer_ctx) {

        assert(_renderer_ctx);
        auto& shdr = _renderer_ctx->shader.quad;

        // commit memory
        void* mem = renderer_memory_commit(_renderer_ctx);
        assert(mem);

        // calculate sizes
        const u32 size_mem          = _renderer_ctx->mem.granularity;
        const u32 size_per_quad     = (QUAD_DATA_SIZE + QUAD_ELEMENT_DATA_SIZE);
        const u32 count_quads_total = (size_mem / size_per_quad);
        const u32 size_data_vtx     = count_quads_total * QUAD_VERTEX_SIZE;
        const u32 size_data_elmnt   = count_quads_total * QUAD_ELEMENT_DATA_SIZE;
        
        // quad capacity / count
        shdr.buffers.quad_capacity  = count_quads_total;
        shdr.buffers.quad_count     = 0;

        // vertex buffer
        shdr.buffers.vertices.size  = size_data_vtx; 
        shdr.buffers.vertices.vptr  = mem;

        // element buffer
        shdr.buffers.elements.size = size_data_elmnt;
        shdr.buffers.elements.addr = (shdr.buffers.vertices.addr + size_data_vtx);

        // make sure we are within bounds
        assert((size_data_vtx + size_data_elmnt) <= size_mem);
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

        // set context objects
        gl_ok &= gl_context_set_vertex_object  (gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr.gl.buffer_vtx);
        gl_ok &= gl_context_set_buffer_element (gl, shdr.gl.buffer_elmnt);
        
        // set data
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr.gl.buffer_vtx,   shdr.buffers.vertices.data, shdr.buffers.vertices.size);
        gl_ok &= gl_buffer_set_element_data    (gl, shdr.gl.buffer_elmnt, shdr.buffers.elements.data, shdr.buffers.elements.size);
        
        // set attributes
        const u32 size_vtx = sizeof(quad_vertex);
        gl_ok &= gl_vertex_add_attribute_f32x3 (gl, shdr.gl.vertex, size_vtx, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (gl, shdr.gl.vertex, size_vtx, 1, 12);
    }
};