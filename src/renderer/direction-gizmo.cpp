#pragma once

#include "renderer.hpp"

namespace ifb {
     
    static constexpr char* DIR_GIZ_UNIFORM_NAME_MAT4_VIEW  = "u_mat4_view";
    static constexpr char* DIR_GIZ_UNIFORM_NAME_MAT4_PROJ  = "u_mat4_proj";
    static constexpr char* DIR_GIZ_UNIFORM_NAME_MAT4_MODEL = "u_mat4_model";
    static constexpr u32   DIR_GIZ_VERT_COUNT              = 6;

    IFB_INTERNAL void
    renderer_direciton_gizmo_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        assert(
            ctx               != NULL &&
            src_vertex.data   != NULL &&
            src_vertex.size   != 0    &&
            src_fragment.data != NULL &&
            src_fragment.size != 0
        );

        // cache properties
        auto& shdr  = ctx->shader.direction_gizmo;
        auto  gl    = ctx->gl;
        bool  gl_ok = true;

        // create gl objects
        shdr.program   = gl_shader_program_create        (gl);
        shdr.vert_shdr = gl_shader_stage_create_vertex   (gl); 
        shdr.frag_shdr = gl_shader_stage_create_fragment (gl); 
        shdr.vertex    = gl_vertex_create                (gl);

        gl_ok &= (
            shdr.program   != GL_ID_INVALID &&
            shdr.vert_shdr != GL_ID_INVALID &&
            shdr.frag_shdr != GL_ID_INVALID &&
            shdr.vertex    != GL_ID_INVALID
        );
        assert(gl_ok);

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr.vert_shdr, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr.frag_shdr, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr.program,  shdr.vert_shdr);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr.program,  shdr.frag_shdr);
        gl_ok &= gl_shader_program_link              (gl, shdr.program);
        gl_shader_stage_destroy                      (gl, shdr.vert_shdr);
        gl_shader_stage_destroy                      (gl, shdr.frag_shdr);
        shdr.vert_shdr = GL_ID_INVALID;
        shdr.frag_shdr = GL_ID_INVALID;
        assert(gl_ok);

        // get the uniform locations
        shdr.unif_mat4_proj = gl_uniform_get_location(gl, shdr.program, DIR_GIZ_UNIFORM_NAME_MAT4_PROJ);
        shdr.unif_mat4_view = gl_uniform_get_location(gl, shdr.program, DIR_GIZ_UNIFORM_NAME_MAT4_VIEW);
        shdr.unif_mat4_model     = gl_uniform_get_location(gl, shdr.program, DIR_GIZ_UNIFORM_NAME_MAT4_MODEL);
        gl_ok &= (
            shdr.unif_mat4_proj  != GL_UNIFORM_INVALID &&
            shdr.unif_mat4_view  != GL_UNIFORM_INVALID &&
            shdr.unif_mat4_model != GL_UNIFORM_INVALID
        );
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_direction_gizmo_draw(
        renderer_context* ctx) {

        assert(ctx);

        auto  gl   = ctx->gl;
        auto& shdr = ctx->shader.direction_gizmo;

        assert(
            shdr.program         != GL_ID_INVALID      &&
            shdr.vertex          != GL_ID_INVALID      &&
            shdr.unif_mat4_proj  != GL_UNIFORM_INVALID &&
            shdr.unif_mat4_view  != GL_UNIFORM_INVALID &&
            shdr.unif_mat4_model != GL_UNIFORM_INVALID
        );

        const mat4&       mat4_proj  = ctx->xform_proj; 
        const mat4&       mat4_view  = ctx->xform_view;
        static const mat4 mat4_model = mat4_identity();

        gl_context_set_shader_program (gl, shdr.program);
        gl_uniform_set_mat4           (gl, shdr.unif_mat4_proj,  (const f32*)&mat4_proj.m);
        gl_uniform_set_mat4           (gl, shdr.unif_mat4_view,  (const f32*)&mat4_view.m);
        gl_uniform_set_mat4           (gl, shdr.unif_mat4_model, (const f32*)&mat4_model.m);
        gl_context_draw_lines         (gl, DIR_GIZ_VERT_COUNT);
    }
};