#pragma once

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "render-buffers.cpp"
#include "eng-renderer.hpp"
#include "sld-opengl.hpp"
#include "sld.hpp"
#include "eng-context.hpp"
#include <cassert>
#include "eng-map.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

struct renderer_tile_shader {
       struct {
           gl_program program;
           gl_vertex  vertex;
           gl_buffer  instance_buffer;
           gl_uniform u_view_proj;
           gl_uniform u_map_count_cols;
           gl_uniform u_map_origin;
           gl_uniform u_map_offset_rows;
           gl_uniform u_map_offset_cols;
           gl_uniform u_tile_unit_size;
           gl_uniform u_color_table;
       } gl;
    };

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL renderer_tile_shader*
    renderer_tile_shader_create(
        void) {
        
        // allocate shader
        auto shdr = (renderer_tile_shader*)global_alloc(sizeof(renderer_tile_shader)); 
        assert(shdr);

        return(shdr);
}

    IFB_INTERNAL void
    renderer_tile_shader_init(
        renderer_tile_shader*         shdr,
        gl_context*                   gl,
        render_buffers*               buffers,
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(shdr);
        assert(gl);
        assert(buffers);

        // the buffers need to be allocated before the shader is initialized
        auto* tile_buf_vtx = buffers->tile_vertices;
        assert(tile_buf_vtx);
        assert(tile_buf_vtx->data     != NULL);
        assert(tile_buf_vtx->capacity != 0);
        assert(tile_buf_vtx->stride   == sizeof(u32) + (sizeof(f32) * 4));

        // create gl objects
        shdr->gl.program         = gl_shader_program_create        (gl);
        shdr->gl.vertex          = gl_vertex_create                (gl); 
        shdr->gl.instance_buffer = gl_buffer_create                (gl);
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (gl);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (gl);
        assert(shdr->gl.program         != GL_ID_INVALID);
        assert(shdr->gl.vertex          != GL_ID_INVALID);
        assert(shdr->gl.instance_buffer != GL_ID_INVALID);
        assert(shdr_vtx                 != GL_ID_INVALID);
        assert(shdr_frg                 != GL_ID_INVALID);

        // compile shader
        bool gl_ok = true;
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program, shdr_vtx); 
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program, shdr_frg); 
        gl_ok &= gl_shader_program_link              (gl, shdr->gl.program);
        gl_shader_stage_destroy                      (gl, shdr_vtx);
        gl_shader_stage_destroy                      (gl, shdr_frg);
        assert(gl_ok);

        // get uniform locations
        shdr->gl.u_view_proj       = gl_uniform_get_location(gl, shdr->gl.program, "u_view_proj"); 
        shdr->gl.u_map_count_cols  = gl_uniform_get_location(gl, shdr->gl.program, "u_map_count_cols");
        shdr->gl.u_map_origin      = gl_uniform_get_location(gl, shdr->gl.program, "u_map_origin");
        shdr->gl.u_map_offset_rows = gl_uniform_get_location(gl, shdr->gl.program, "u_map_offset_rows"); 
        shdr->gl.u_map_offset_cols = gl_uniform_get_location(gl, shdr->gl.program, "u_map_offset_cols"); 
        shdr->gl.u_tile_unit_size  = gl_uniform_get_location(gl, shdr->gl.program, "u_tile_unit_size"); 
        shdr->gl.u_color_table     = gl_uniform_get_location(gl, shdr->gl.program, "u_color_table"); 
        assert(shdr->gl.u_view_proj       != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_count_cols  != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_origin      != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_offset_rows != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_offset_cols != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_tile_unit_size  != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_color_table     != GL_UNIFORM_INVALID);

        // define vertex
        // the gl buffer is sized to hold everything the render buffer can
        gl_ok &= gl_context_set_shader_program (gl, shdr->gl.program);
        gl_ok &= gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr->gl.instance_buffer);
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr->gl.instance_buffer, tile_buf_vtx->data, tile_buf_vtx->capacity);
        gl_ok &= gl_vertex_add_u32x1           (gl, shdr->gl.vertex, tile_buf_vtx->stride, 0, 0);
        gl_ok &= gl_vertex_add_f32x4           (gl, shdr->gl.vertex, tile_buf_vtx->stride, 1, sizeof(u32));
        gl_ok &= gl_vertex_divisor             (gl, shdr->gl.vertex, 0, 1);
        gl_ok &= gl_vertex_divisor             (gl, shdr->gl.vertex, 1, 1);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_tile_draw(
        renderer_tile_shader* shdr,
        gl_context*           gl,
        render_buffers*       buffers,
        const hnd_map         map_hnd,
        const mat4&           view_proj_xform) {

        assert(shdr);
        assert(gl);
        assert(buffers);

        // if nothing was rendered to the buffer, we're done
        auto* tile_buf_vtx = buffers->tile_vertices;
        if (map_hnd == INVALID_HANDLE || tile_buf_vtx->size == 0) {
            return;
        }

        // get the color table
        // TODO(SLD): we probably only need to do this
        // once on startup
        const auto& color_tbl   = map_mngr_get_color_table();
        const u32*  color_array = (u32*)&color_tbl;
        const u32   color_count = sizeof(map_color_table) / sizeof(color_rgba_u32); 
        color_rgba_f32 color_vec4_array[color_count];
        for (u32 i = 0; i < color_count; ++i) {
            color_vec4_array[i] = color_rgba_f32(color_array[i]);
        }
       
        // get the map dimensions
        map_dimensions map_dims;
        map_get_dimensions(map_hnd, map_dims);

        // get the map origin
        vec3 map_origin;
        map_get_origin(map_hnd, map_origin);

        // get tile count and config for tile size
        const u32   tile_count = tile_buf_vtx->count();
        const auto& cfg        = config_instance();

        // set the texture
        renderer_use_sprite_atlas_texture();

        // update the shader and draw vertices
        bool gl_ok = true;
        gl_ok &= gl_context_set_shader_program      (gl, shdr->gl.program);
        gl_ok &= gl_uniform_set_u32x1               (gl, shdr->gl.u_map_count_cols,  map_dims.count_cols);
        gl_ok &= gl_uniform_set_f32x3               (gl, shdr->gl.u_map_origin,      map_origin.v);
        gl_ok &= gl_uniform_set_s32x1               (gl, shdr->gl.u_map_offset_rows, 0);
        gl_ok &= gl_uniform_set_s32x1               (gl, shdr->gl.u_map_offset_cols, 0);
        gl_ok &= gl_uniform_set_f32x1               (gl, shdr->gl.u_tile_unit_size,  cfg.map_tile_unit_size);
        gl_ok &= gl_uniform_set_mat4                (gl, shdr->gl.u_view_proj,       view_proj_xform.m);
        gl_ok &= gl_uniform_set_f32_array           (gl, shdr->gl.u_color_table,     (f32*)color_vec4_array, color_count);
        gl_ok &= gl_context_set_vertex_object       (gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex       (gl, shdr->gl.instance_buffer);
        gl_ok &= gl_buffer_update_vertex_data       (gl, shdr->gl.instance_buffer, tile_buf_vtx->data, tile_buf_vtx->size);

        // the depth test normally always passes, but the walls stand up off the floor
        // and have to hide each other, so the map gets a real depth test
        // everything drawn after it still goes on top
        glDepthFunc(GL_LESS);
        gl_ok &= gl_context_draw_vertices_instanced (gl, 6, tile_count);
        glDepthFunc(GL_ALWAYS);
        assert(gl_ok);

        }
};
