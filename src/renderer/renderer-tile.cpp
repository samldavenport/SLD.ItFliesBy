#include "renderer.hpp"
#include "sld-opengl.hpp"
#include <cassert>

namespace ifb {

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
           gl_buffer  buf_vertices;
           gl_buffer  buf_instances;
           gl_buffer  buf_elements;
           gl_uniform u_sampler2d_texture;
           gl_uniform u_mat4_view_proj;
           gl_uniform u_mat4_model;
           gl_uniform u_vec2_tile_size;
           gl_uniform u_vec2_map_dimensions;
       } gl;
       struct {
           renderer_tile_instance_buffer instance;
       } buffers;

    };

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
        shdr->gl.u_sampler2d_texture   = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_SAMPLER2D_TEXTURE);
        shdr->gl.u_mat4_view_proj      = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_MAT4_VIEW_PROJ);
        shdr->gl.u_mat4_model          = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_MAT4_MODEL);
        shdr->gl.u_vec2_tile_size      = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_VEC2_TILE_SIZE);
        shdr->gl.u_vec2_map_dimensions = gl_uniform_get_location (gl_ctx, shdr->gl.program, TILE_UNIFORM_VEC2_MAP_DIMENSIONS);
        gl_ok &= (shdr->gl.u_mat4_view_proj      != GL_UNIFORM_INVALID);
        gl_ok &= (shdr->gl.u_mat4_model          != GL_UNIFORM_INVALID);
        gl_ok &= (shdr->gl.u_vec2_tile_size      != GL_UNIFORM_INVALID);
        gl_ok &= (shdr->gl.u_vec2_map_dimensions != GL_UNIFORM_INVALID);
        assert(gl_ok);

        // define vertex
        const u32 size_vtx  = sizeof(renderer_tile_vertex);
        const u32 size_inst = sizeof(renderer_tile_instance);
        gl_ok &= gl_context_set_shader_program  (gl_ctx, shdr->gl.program);     
        gl_ok &= gl_context_set_vertex_object   (gl_ctx, shdr->gl.vertex);
        gl_ok &= gl_buffer_set_vertex_data      (gl_ctx, shdr->gl.buf_vertices,  shdr->buffers.instance.data.bytes, shdr->buffers.instance.data_size);
        gl_ok &= gl_buffer_set_vertex_data      (gl_ctx, shdr->gl.buf_instances, (byte*)TILE_VERTICES, sizeof(TILE_VERTICES));
        // TODO(SLD): why the FUCK does adding this mess with the quad shader? Jig no longer renders
        gl_ok &= gl_buffer_set_element_data     (gl_ctx, shdr->gl.buf_elements, (byte*)TILE_INDICES, sizeof(TILE_INDICES));
        gl_ok &= gl_vertex_add_attribute_f32x2  (gl_ctx, shdr->gl.vertex, size_vtx,  0, offsetof(renderer_tile_vertex, corner)); 
        gl_ok &= gl_vertex_add_attribute_u32x1  (gl_ctx, shdr->gl.vertex, size_inst, 1, offsetof(renderer_tile_instance, id));
        gl_ok &= gl_vertex_add_attribute_u32x1  (gl_ctx, shdr->gl.vertex, size_inst, 2, offsetof(renderer_tile_instance, color));
        gl_ok &= gl_vertex_divisor              (gl_ctx, shdr->gl.vertex, 0, 0);
        gl_ok &= gl_vertex_divisor              (gl_ctx, shdr->gl.vertex, 1, 1);
        gl_ok &= gl_vertex_divisor              (gl_ctx, shdr->gl.vertex, 2, 1);
        assert(gl_ok);
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

    IFB_INTERNAL void
    renderer_tile_draw(
        const mat4& view_proj_xform) {

        assert(_renderer_ctx);

        auto shdr   = _renderer_ctx->shader.tile;
        auto gl_ctx = _renderer_ctx->gl;

        mat4 m = mat4_identity();
    
        gl_context_set_shader_program (gl_ctx, shdr->gl.program);
        gl_context_set_vertex_object  (gl_ctx, shdr->gl.vertex);
        gl_uniform_set_mat4           (gl_ctx, shdr->gl.u_mat4_view_proj, view_proj_xform.m);
        gl_uniform_set_mat4           (gl_ctx, shdr->gl.u_mat4_model, m.m);

        assert(gl_ctx);
        assert(shdr);
        
    }
};
