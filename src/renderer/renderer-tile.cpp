#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-arena.cpp"
#include "renderer.hpp"
#include "sld-opengl.hpp"
#include "map.hpp"
#include "sld.hpp"
#include "eng-internal.hpp"
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
           gl_uniform u_map_count_cols;
           gl_uniform u_map_origin;
           gl_uniform u_map_offset_rows;
           gl_uniform u_map_offset_cols;
           gl_uniform u_tile_unit_size;
           gl_uniform u_color_table;
       } gl;
       hnd_map               map_hnd;
       hnd_arena             arena;
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

        // allocate buffer memory
        shdr->arena = arena_alloc();
        assert(shdr->arena != INVALID_HANDLE);

        return(shdr);
    }

    IFB_INTERNAL void
    renderer_tile_shader_init(
        renderer_tile_shader*         shdr,
        gl_context*                   gl,
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        const auto& cfg    = config_instance();

        // set the map info
        shdr->map_hnd       = INVALID_HANDLE;

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

        // define verte
        auto& render_buffer = map_mngr_get_render_buffer();
        assert(render_buffer.data.bytes   != NULL);
        assert(cfg.map_render_buffer_size != 0);
        gl_ok &= gl_context_set_shader_program (gl, shdr->gl.program);
        gl_ok &= gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr->gl.instance_buffer);
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr->gl.instance_buffer, render_buffer.data.bytes, cfg.map_render_buffer_size);
        gl_ok &= gl_vertex_add_u32x1           (gl, shdr->gl.vertex, sizeof(renderer_tile_instance), 0, 0);
        gl_ok &= gl_vertex_divisor             (gl, shdr->gl.vertex, 0, 1);
        assert(gl_ok);
    }

    IFB_INTERNAL bool 
    renderer_tile_set_map(
        renderer_tile_shader* shdr,
        const hnd_map         map_hnd) {

        assert(shdr);
        assert(map_hnd != INVALID_ID);

        if (map_hnd == shdr->map_hnd) {
            return(true);
        }

        shdr->map_hnd = map_hnd;
        const bool result = map_render(map_hnd);
        return(result);
    }
    
    IFB_INTERNAL void
    renderer_tile_draw(
        renderer_tile_shader* shdr,
        gl_context*           gl,
        const mat4&           view_proj_xform) {

        assert(shdr);
        assert(gl);

        if (shdr->map_hnd == INVALID_HANDLE) {
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
        map_get_dimensions(shdr->map_hnd, map_dims);

        // get the map origin
        vec3 map_origin;
        map_get_origin(shdr->map_hnd, map_origin);

        // get tile count, buffer, and config for tile size    
        const auto& buffer     = map_mngr_get_render_buffer();
        const u32   tile_count = buffer.data_size / sizeof(map_tile);
        const auto& cfg        = config_instance();

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
        gl_ok &= gl_buffer_update_vertex_data       (gl, shdr->gl.instance_buffer, buffer.data.bytes, buffer.data_size); 
        gl_ok &= gl_context_draw_vertices_instanced (gl, 6, tile_count);
        assert(gl_ok);

        }
};
