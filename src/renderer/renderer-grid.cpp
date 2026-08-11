#pragma once

#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math-types.hpp"
#include "sld-opengl.hpp"

namespace ifb {

    static const char* GRID_UNIFORM_NAME_VIEW_PROJ = "view_projection";

    static const f32 _grid_coordinates[] = {
         1.0f,  1.0f, 0.0f, 
        -1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f
    };

    static const u32 _grid_vertices_count = 6;

    struct renderer_grid_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_uniform u_mat4_view_proj;
        } gl;
    };

    IFB_INTERNAL void
    renderer_grid_shader_create(void) {
      
        assert(_renderer_ctx);

        _renderer_ctx->shader.grid = _renderer_ctx->memory.stack.push_struct<renderer_grid_shader>();
       assert(_renderer_ctx->shader.grid); 
    }

    IFB_INTERNAL void
    renderer_grid_shader_init(
        const renderer_shader_source &src_vertex,
        const renderer_shader_source &src_fragment) {

        assert(_renderer_ctx);

        auto shdr = _renderer_ctx->shader.grid;
        auto gl   = _renderer_ctx->gl;
        assert(shdr);
        assert(gl);

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
        const u32 vtx_size = sizeof(vec3);
        gl_ok &= gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr->gl.buf_vertex, (byte*)_grid_coordinates, sizeof(_grid_coordinates));
        gl_ok &= gl_vertex_add_attribute_f32x3 (gl, shdr->gl.vertex, sizeof(vec3), 0, 0);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_grid_draw(
        const mat4& view_proj_xform) {

        assert(_renderer_ctx);

        auto shdr = _renderer_ctx->shader.grid;
        auto gl   = _renderer_ctx->gl;
        assert(shdr);
        assert(gl);

        mat4 ident = mat4_identity();

        gl_context_set_shader_program (gl, shdr->gl.program);
        gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);
        gl_uniform_set_mat4           (gl, shdr->gl.u_mat4_view_proj, view_proj_xform.m); 
        gl_context_draw_vertices      (gl, _grid_vertices_count);
    }
};
