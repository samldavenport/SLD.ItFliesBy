#pragma once

#include "renderer.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void quad_shader_validate                 (const quad_shader& shdr);
    IFB_INLINE void quad_shader_create_gl_objects        (quad_shader& shdr, gl_context* gl);
    IFB_INLINE void quad_shader_compile_and_link_program (quad_shader& shdr, gl_context* gl, const shader_source& src_vertex, const shader_source& src_fragment);
    IFB_INLINE void quad_shader_define_vertex            (quad_shader& shdr, gl_context* gl);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_quad_shader_init(
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        quad_shader_create_gl_objects        (_renderer_ctx->shader.quad, _renderer_ctx->gl);
        quad_shader_compile_and_link_program (_renderer_ctx->shader.quad, _renderer_ctx->gl, src_vertex, src_fragment);
        quad_shader_define_vertex            (_renderer_ctx->shader.quad, _renderer_ctx->gl);
    }

    IFB_INTERNAL void 
    renderer_quad_push(
        const entity_id quad_id) {

        assert(quad_does_exist(quad_id));
        _renderer_ctx->shader.quad.list.add(quad_id);
    }

    IFB_INTERNAL void
    renderer_quad_draw_list(
        void) {

        auto& shdr = _renderer_ctx->shader.quad;        
        quad_shader_validate(shdr);

        for (
            u32 i = 0;
            i < shdr.list.count();
            ++i) {

            const entity_id id       = shdr.list[i];
            quad_vertices&  vertices = shdr.buffers.vertices.vertices[i];

            assert(id != ENTITY_ID_INVALID);
            assert(quad_get_vertices(vertices, id));
        }

        const u32 element_draw_count = (6 * shdr.list.count()); 
        
        gl_context_set_shader_program (_renderer_ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (_renderer_ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (_renderer_ctx->gl, element_draw_count);

    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline void
    quad_shader_validate(
        const quad_shader& shdr) {

        shdr.list.validate();
        assert(shdr.gl.program      != GL_ID_INVALID);
        assert(shdr.gl.vertex       != GL_ID_INVALID);
        assert(shdr.gl.buffer_vtx   != GL_ID_INVALID);
        assert(shdr.gl.buffer_elmnt != GL_ID_INVALID);
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