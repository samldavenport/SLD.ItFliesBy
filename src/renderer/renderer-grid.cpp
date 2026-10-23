#pragma once

#include "ifb-types.hpp"
#include "render-buffers.cpp"
#include "renderer.hpp"
#include "renderer-internal.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math-types.hpp"
#include "sld-opengl.hpp"
#include "eng-internal.hpp"

namespace ifb {

    static const char* GRID_UNIFORM_NAME_VIEW_PROJ = "view_projection";

struct renderer_grid_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_uniform u_mat4_view_proj;
        } gl;
    };

    IFB_INTERNAL renderer_grid_shader*
    renderer_grid_shader_create(
        void) {
      
        auto shdr = (renderer_grid_shader*)global_alloc(sizeof(renderer_grid_shader));
        assert(shdr); 
        return(shdr);
    }

    IFB_INTERNAL void
    renderer_grid_shader_init(
        renderer_grid_shader*         shdr,
        gl_context*                   gl,
        render_buffers*               buffers,
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(shdr);
        assert(gl);
        assert(buffers);

        // the buffers need to be initialized before the shader
        auto* grid_buf_vtx = buffers->grid_vertices;
        assert(grid_buf_vtx);
        assert(grid_buf_vtx->data   != NULL);
        assert(grid_buf_vtx->size   != 0);
        assert(grid_buf_vtx->stride == sizeof(vec3));

        bool gl_ok = true;

        // create gl objects
        shdr->gl.program          = gl_shader_program_create        (gl);
        shdr->gl.vertex           = gl_vertex_create                (gl);
        shdr->gl.buf_vertex       = gl_buffer_create                (gl);
        const gl_shader shdr_vert = gl_shader_stage_create_vertex   (gl);
        const gl_shader shdr_frag = gl_shader_stage_create_fragment (gl);
        gl_ok = (
            shdr->gl.program    != GL_ID_INVALID &&
            shdr->gl.vertex     != GL_ID_INVALID &&
            shdr->gl.buf_vertex != GL_ID_INVALID &&
            shdr_vert           != GL_ID_INVALID &&
            shdr_frag           != GL_ID_INVALID 
        );
        assert(gl_ok);

        // compile and link shader
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_vert, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_frag, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program, shdr_vert);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program, shdr_frag);
        gl_ok &= gl_shader_program_link              (gl, shdr->gl.program);
        gl_shader_stage_destroy                      (gl, shdr_vert);
        gl_shader_stage_destroy                      (gl, shdr_frag);
        assert(gl_ok);

        // get the uniform locations
        shdr->gl.u_mat4_view_proj = gl_uniform_get_location(gl, shdr->gl.program, GRID_UNIFORM_NAME_VIEW_PROJ);
        assert(shdr->gl.u_mat4_view_proj != GL_UNIFORM_INVALID);
    
        // define vertex
        // the grid never changes, so it only needs to be sent once
        const u32 vtx_size = grid_buf_vtx->stride;
        gl_ok &= gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr->gl.buf_vertex, grid_buf_vtx->data, grid_buf_vtx->size);
        gl_ok &= gl_vertex_add_f32x3           (gl, shdr->gl.vertex, vtx_size, 0, 0);
assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_grid_draw(
        renderer_grid_shader* shdr,
        gl_context*           gl,
        render_buffers*       buffers,
        const mat4&           view_proj_xform) {

        assert(shdr);
        assert(gl);
        assert(buffers);

        auto* grid_buf_vtx = buffers->grid_vertices;

        gl_context_set_shader_program (gl, shdr->gl.program);
        gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);
        gl_uniform_set_mat4           (gl, shdr->gl.u_mat4_view_proj, view_proj_xform.m);

        // the grid is tested against the map's depth so walls hide it,
        // everything drawn after it still goes on top
        glDepthFunc(GL_LEQUAL);
        gl_context_draw_vertices      (gl, grid_buf_vtx->count());
        glDepthFunc(GL_ALWAYS);
}
};
