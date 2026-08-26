#include "renderer.hpp"
#include "sld-opengl.hpp"
#include <cassert>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS 
    //--------------------------------------------------------------------
    
    static constexpr char TILE_UNIFORM_SAMPLER2D_TEXTURE   [] = "u_sampler2d_texture";
    static constexpr char TILE_UNIFORM_MAT4_VIEW_PROJ      [] = "u_mat4_view_proj";
    static constexpr char TILE_UNIFORM_MAT4_MODEL          [] = "u_mat4_model";
    static constexpr char TILE_UNIFORM_VEC2_TILE_SIZE      [] = "u_vec2_tile_size";
    static constexpr char TILE_UNIFORM_VEC2_MAP_DIMENSIONS [] = "u_vec2_map_dimensions";

    static constexpr renderer_tile_vertex TILE_VERTICES [] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
    };

    static constexpr u32 TILE_INDICES[] = {
        0, 1, 2,
        2, 3, 0
    };

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
   
    struct renderer_tile_vertex {
        vec2 corner;
    };

    struct renderer_tile_instance {
        u32 id;
        u32 color;
    };

    struct renderer_tile_instance_buffer {
        u32                     capacity;
        u32                     count;
        union {
            renderer_tile_instance* data; 
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
           gl_buffer  buf_vertices;
           gl_buffer  buf_instances;
           gl_buffer  buf_elements;
           gl_uniform u_sampler2d_texture;
           gl_uniform u_mat4_view_proj;
           gl_uniform u_mat4_model;
           gl_uniform u_vec2_tile_size;
           gl_uniform u_vec2_map_dimensions;
       } gl;
    };

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_tile_shader_create(
        void) {

        assert(_renderer_ctx);

        auto shdr = _renderer_ctx->memory.stack.push_struct<renderer_tile_shader>();
        assert(shdr);

        _renderer_ctx->shader.tile = shdr;

        //TODO(SLD): need to create buffers for the tiles
    }

    IFB_INTERNAL void
    renderer_tile_shader_init(
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(_renderer_ctx);

        auto shdr   = _renderer_ctx->shader.tile;
        auto gl_ctx = _renderer_ctx->gl;
        assert(shdr);

        // create gl objects
        shdr->gl.program         = gl_shader_program_create        (gl_ctx);
        shdr->gl.vertex          = gl_vertex_create                (gl_ctx); 
        shdr->gl.buf_vertices    = gl_buffer_create                (gl_ctx);
        shdr->gl.buf_instances   = gl_buffer_create                (gl_ctx);
        shdr->gl.buf_elements    = gl_buffer_create                (gl_ctx);
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (gl_ctx);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (gl_ctx);
        
        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (gl_ctx, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl_ctx, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl_ctx, shdr->gl.program, shdr_vtx); 
        gl_ok &= gl_shader_program_attach_stage      (gl_ctx, shdr->gl.program, shdr_frg); 
        gl_ok &= gl_shader_program_link              (gl_ctx, shdr->gl.program);
        gl_shader_stage_destroy                      (gl_ctx, shdr_vtx);
        gl_shader_stage_destroy                      (gl_ctx, shdr_frg);
        assert(gl_ok);

        // get uniform locations
        // TODO(SAM): for some reason the commented
        // uniforms are not loading properly
        shdr->gl.u_sampler2d_texture   = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_SAMPLER2D_TEXTURE);
        shdr->gl.u_mat4_view_proj      = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_MAT4_VIEW_PROJ);
        shdr->gl.u_mat4_model          = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_MAT4_MODEL);
        shdr->gl.u_vec2_tile_size      = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_VEC2_TILE_SIZE);
        shdr->gl.u_vec2_map_dimensions = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_VEC2_MAP_DIMENSIONS);
        // gl_ok &= (shdr->gl.u_sampler2d_texture   != GL_UNIFORM_INVALID);
        gl_ok &= (shdr->gl.u_mat4_view_proj      != GL_UNIFORM_INVALID);
        //gl_ok &= (shdr->gl.u_mat4_model          != GL_UNIFORM_INVALID);
        gl_ok &= (shdr->gl.u_vec2_tile_size      != GL_UNIFORM_INVALID);
        gl_ok &= (shdr->gl.u_vec2_map_dimensions != GL_UNIFORM_INVALID);
        assert(gl_ok);

        // define vertex
        //TODO(SAM): need to finalize vertex definition
        const u32 size_vtx  = sizeof(renderer_tile_vertex);
        const u32 size_inst = sizeof(renderer_tile_instance);
    }

    IFB_INTERNAL void
    renderer_tile_shader_set_tile_size(
        const f32 width,
        const f32 height) {

        assert(_renderer_ctx);

        auto shdr   = _renderer_ctx->shader.tile;
        auto gl_ctx = _renderer_ctx->gl;
        assert(gl_ctx);
        assert(shdr);
        assert(shdr->gl.program          != GL_ID_INVALID);
        assert(shdr->gl.u_vec2_tile_size != GL_UNIFORM_INVALID);

        vec2 size;
        size.x = width;
        size.y = height;

        bool did_set = true;
        did_set &= gl_context_set_shader_program (gl_ctx, shdr->gl.program);
        did_set &= gl_uniform_set_f32x2          (gl_ctx, shdr->gl.u_vec2_tile_size, size.v); 
        assert(did_set);
    } 

    IFB_INTERNAL void
    renderer_tile_shader_set_map_size(
        const f32 count_rows,
        const f32 count_cols) {

        assert(_renderer_ctx);

        auto shdr   = _renderer_ctx->shader.tile;
        auto gl_ctx = _renderer_ctx->gl;
        assert(gl_ctx);
        assert(shdr);
        assert(shdr->gl.program               != GL_ID_INVALID);
        assert(shdr->gl.u_vec2_map_dimensions != GL_UNIFORM_INVALID);

        vec2 dims;
        dims.x = count_rows;
        dims.y = count_cols;

        bool did_set = true; 
        did_set &= gl_context_set_shader_program (gl_ctx, shdr->gl.program);
        did_set &= gl_uniform_set_f32x2          (gl_ctx, shdr->gl.u_vec2_map_dimensions, dims.v); 
        assert(did_set);
    }

};
