#include "ifb-types.hpp"
#include "renderer.hpp"
#include "sld-opengl.hpp"
#include "map.hpp"
#include "map.cpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct renderer_tile_instance {
        u32 color;
    };

    struct renderer_tile_instance_buffer {
        u32 data_size;
        u32 data_length;
        union {
            renderer_tile_instance* array; 
            byte*                   bytes;
            void*                   vptr;
            f32*                    floats;
            addr                    addr;
        } data;
    };

    struct renderer_tile_shader {
       struct {
           gl_program program;
           gl_vertex  vertex;
           gl_buffer  instance_buffer;
           gl_uniform u_view_proj;
           gl_uniform u_map_count_rows;
           gl_uniform u_map_count_cols;
           gl_uniform u_map_offset_rows;
           gl_uniform u_map_offset_cols;
           gl_uniform u_tile_unit_size;
       } gl;
       struct {
           renderer_tile_instance_buffer instance;
       } buffers;
       map_handle map_hnd;
    };

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_tile_shader_create(
        void) {

        assert(_renderer_ctx);

        const auto& cfg = config_instance();
        
        // allocate shader
        auto shdr = _renderer_ctx->memory.stack.push_struct<renderer_tile_shader>();
        assert(shdr);
        _renderer_ctx->shader.tile = shdr;

        // allocate buffer memory
        shdr->buffers.instance.data_size   = cfg.tile_capacity * sizeof(renderer_tile_instance); 
        shdr->buffers.instance.data_length = 0;
        shdr->buffers.instance.data.vptr   = renderer_context_memory_alloc(shdr->buffers.instance.data_size);   
        assert(shdr->buffers.instance.data_size != 0);
        assert(shdr->buffers.instance.data.vptr != NULL);
    }

    IFB_INTERNAL void
    renderer_tile_shader_init(
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(_renderer_ctx);

        const auto& cfg    = config_instance();
        auto        shdr   = _renderer_ctx->shader.tile;
        auto        gl_ctx = _renderer_ctx->gl;
        assert(shdr);

        // create gl objects
        shdr->gl.program         = gl_shader_program_create        (gl_ctx);
        shdr->gl.vertex          = gl_vertex_create                (gl_ctx); 
        shdr->gl.instance_buffer = gl_buffer_create                (gl_ctx);
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (gl_ctx);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (gl_ctx);
        assert(shdr->gl.program         != GL_ID_INVALID);
        assert(shdr->gl.vertex          != GL_ID_INVALID);
        assert(shdr->gl.instance_buffer != GL_ID_INVALID);
        assert(shdr_vtx                 != GL_ID_INVALID);
        assert(shdr_frg                 != GL_ID_INVALID);

        // compile shader
        bool gl_ok = true;
        gl_ok &= gl_shader_stage_compile_from_source (gl_ctx, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl_ctx, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl_ctx, shdr->gl.program, shdr_vtx); 
        gl_ok &= gl_shader_program_attach_stage      (gl_ctx, shdr->gl.program, shdr_frg); 
        gl_ok &= gl_shader_program_link              (gl_ctx, shdr->gl.program);
        gl_shader_stage_destroy                      (gl_ctx, shdr_vtx);
        gl_shader_stage_destroy                      (gl_ctx, shdr_frg);
        assert(gl_ok);

        // get uniform locations
        shdr->gl.u_view_proj       = gl_uniform_get_location(gl_ctx, shdr->gl.program, "u_view_proj"); 
        shdr->gl.u_map_count_rows  = gl_uniform_get_location(gl_ctx, shdr->gl.program, "u_map_count_rows"); 
        shdr->gl.u_map_count_cols  = gl_uniform_get_location(gl_ctx, shdr->gl.program, "u_map_count_cols"); 
        shdr->gl.u_map_offset_rows = gl_uniform_get_location(gl_ctx, shdr->gl.program, "u_map_offset_rows"); 
        shdr->gl.u_map_offset_cols = gl_uniform_get_location(gl_ctx, shdr->gl.program, "u_map_offset_cols"); 
        shdr->gl.u_tile_unit_size  = gl_uniform_get_location(gl_ctx, shdr->gl.program, "u_tile_unit_size"); 
        assert(shdr->gl.u_view_proj       != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_count_rows  != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_count_cols  != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_offset_rows != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_map_offset_cols != GL_UNIFORM_INVALID);
        assert(shdr->gl.u_tile_unit_size  != GL_UNIFORM_INVALID);

        // define vertex
        gl_ok &= gl_context_set_shader_program (gl_ctx, shdr->gl.program);
        gl_ok &= gl_context_set_vertex_object  (gl_ctx, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl_ctx, shdr->gl.instance_buffer);
        gl_ok &= gl_buffer_set_vertex_data     (gl_ctx, shdr->gl.instance_buffer, shdr->buffers.instance.data.bytes, shdr->buffers.instance.data_size);
        gl_ok &= gl_vertex_add_u32x1           (gl_ctx, shdr->gl.vertex, sizeof(renderer_tile_instance), 0, 0);
        gl_ok &= gl_vertex_divisor             (gl_ctx, shdr->gl.vertex, 0, 1);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_tile_set_map(
        const map_handle map_hnd) {

        assert(map_hnd != INVALID_ID);
        assert(_renderer_ctx);
       
        auto shdr = _renderer_ctx->shader.tile;
        assert(shdr);

        shdr->map_hnd = map_hnd;
    }
    
    IFB_INTERNAL void
    renderer_tile_draw(
        const mat4& view_proj_xform) {

        assert(_renderer_ctx);

        auto shdr   = _renderer_ctx->shader.tile;
        auto gl_ctx = _renderer_ctx->gl;

        assert(gl_ctx);
        assert(shdr);

        // look up the map
        map map;
        const bool found_map = map_get_info(shdr->map_hnd, map);
        assert(found_map);

        // get the tile size
        const f32 tile_unit_size = map_mngr_get_tile_unit_size();

        // check our render buffer is large enough
        const u32 buffer_size_required = map_get_render_buffer_size(shdr->map_hnd);
        assert(shdr->buffers.instance.data_size >= buffer_size_required);

        // calculate tile count
        const u32 tile_count = map.count_rows * map.count_cols;
        
        // copy the render buffer data
        const u32 buffer_size_actual = map_get_render_buffer_data(
                shdr->map_hnd,
                shdr->buffers.instance.data_size,
                shdr->buffers.instance.data.bytes
        );
 
        // update the shader and draw vertices
        bool gl_ok = true;
        gl_ok &= gl_context_set_shader_program      (gl_ctx, shdr->gl.program);
        gl_ok &= gl_uniform_set_u32x1               (gl_ctx, shdr->gl.u_map_count_rows, map.count_rows); 
        gl_ok &= gl_uniform_set_u32x1               (gl_ctx, shdr->gl.u_map_count_cols, map.count_cols); 
        gl_ok &= gl_uniform_set_s32x1               (gl_ctx, shdr->gl.u_map_offset_rows, map.offset_row); 
        gl_ok &= gl_uniform_set_s32x1               (gl_ctx, shdr->gl.u_map_offset_cols, map.offset_col); 
        gl_ok &= gl_uniform_set_f32x1               (gl_ctx, shdr->gl.u_tile_unit_size, tile_unit_size); 
        gl_ok &= gl_uniform_set_mat4                (gl_ctx, shdr->gl.u_view_proj, view_proj_xform.m);
        gl_ok &= gl_context_set_vertex_object       (gl_ctx, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex       (gl_ctx, shdr->gl.instance_buffer);
        gl_ok &= gl_buffer_update_vertex_data       (gl_ctx, shdr->gl.instance_buffer, shdr->buffers.instance.data.bytes, shdr->buffers.instance.data_size);
        gl_ok &= gl_context_draw_vertices_instanced (gl_ctx, 6, tile_count);
        assert(gl_ok);
    }
};
