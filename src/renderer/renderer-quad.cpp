#pragma once

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "renderer.hpp"
#include "sld-math-mat4.hpp"
#include "sld-opengl.hpp"
#include <cassert>
#include "collections-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS 
    //--------------------------------------------------------------------
    
    static constexpr char* QUAD_UNIFORM_NAME_MAT4_VIEW_PROJ  = "u_mat4_view_proj";
    static constexpr char* QUAD_UNIFORM_NAME_MAT4_MODEL      = "u_mat4_model";

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------
    
    struct renderer_quad_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buf_vertex;
            gl_buffer  buf_element;
            gl_uniform unif_mat4_view_proj;
            gl_uniform unif_mat4_model;
        } gl;
        struct {
            renderer_quad_vertex_buffer  vertex;
            renderer_quad_element_buffer element;
        } buffers;
        entity_list render_list;
    };

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_quad_shader_create(
        void) {

        assert(_renderer_ctx);

        auto shdr = _renderer_ctx->memory.stack.push_struct<renderer_quad_shader>();
        assert(shdr); 
        _renderer_ctx->shader.quad = shdr;

        const auto& cfg     = config_instance();
        auto&       buffers = shdr->buffers;
        auto&       list    = shdr->render_list;

        auto* quad_entities       = (entity_id*)renderer_context_memory_alloc(cfg.quad_capacity * sizeof(entity_id));
        buffers.vertex.size       = (cfg.quad_capacity  * sizeof(renderer_quad_vertices)); 
        buffers.vertex.data.vptr  = renderer_context_memory_alloc(buffers.vertex.size);
        buffers.element.size      = (cfg.quad_capacity * sizeof(u32) * 6);
        buffers.element.data.vptr = renderer_context_memory_alloc(buffers.element.size);

        assert(buffers.vertex.size       != 0);
        assert(buffers.vertex.data.vptr  != 0);
        assert(buffers.element.size      != 0);
        assert(buffers.element.data.vptr != 0);
        assert(quad_entities             != NULL);

        list.stack_init(_renderer_ctx->memory.stack); 
    }
    
    IFB_INTERNAL void
    renderer_quad_shader_init(
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(_renderer_ctx);
        assert(sizeof(vec3)           == 12);
        assert(sizeof(color_rgba_f32) == 16);

        auto shdr = _renderer_ctx->shader.quad;

        // set the element data
        const auto& cfg = config_instance();
        for (
            u32 i = 0;
            i < cfg.quad_capacity;
            ++i) {

            const u32 offset   = (i * 4);
            auto&     elements = shdr->buffers.element.data.elements[i];
            elements.elmnt_0_index_0 = (offset);  
            elements.elmnt_1_index_1 = (offset + 1); 
            elements.elmnt_2_index_3 = (offset + 3); 
            elements.elmnt_3_index_1 = (offset + 1); 
            elements.elmnt_4_index_2 = (offset + 2); 
            elements.elmnt_5_index_3 = (offset + 3); 
        }

        // create gl objects
        shdr->gl.program          = gl_shader_program_create        (_renderer_ctx->gl);
        shdr->gl.vertex           = gl_vertex_create                (_renderer_ctx->gl);
        shdr->gl.buf_vertex       = gl_buffer_create                (_renderer_ctx->gl); 
        shdr->gl.buf_element      = gl_buffer_create                (_renderer_ctx->gl);
        const gl_shader shdr_vtx  = gl_shader_stage_create_vertex   (_renderer_ctx->gl);
        const gl_shader shdr_frg  = gl_shader_stage_create_fragment (_renderer_ctx->gl);

        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (_renderer_ctx->gl, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (_renderer_ctx->gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (_renderer_ctx->gl, shdr->gl.program,  shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (_renderer_ctx->gl, shdr->gl.program,  shdr_frg);
        gl_ok &= gl_shader_program_link              (_renderer_ctx->gl, shdr->gl.program);
        gl_shader_stage_destroy                      (_renderer_ctx->gl, shdr_vtx);
        gl_shader_stage_destroy                      (_renderer_ctx->gl, shdr_frg);
        assert(gl_ok);

        // get uniform locations
        shdr->gl.unif_mat4_view_proj   = gl_uniform_get_location(_renderer_ctx->gl, shdr->gl.program, QUAD_UNIFORM_NAME_MAT4_VIEW_PROJ);
        shdr->gl.unif_mat4_model       = gl_uniform_get_location(_renderer_ctx->gl, shdr->gl.program, QUAD_UNIFORM_NAME_MAT4_MODEL);
        gl_ok &= (
            shdr->gl.unif_mat4_view_proj   != GL_UNIFORM_INVALID && 
            shdr->gl.unif_mat4_model       != GL_UNIFORM_INVALID
        );
        assert(gl_ok);

        // define vertex
        const u32 vertex_size  = sizeof(vec3) + sizeof(vec4); 
        gl_ok &= gl_context_set_vertex_object  (_renderer_ctx->gl, shdr->gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (_renderer_ctx->gl, shdr->gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (_renderer_ctx->gl, shdr->gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (_renderer_ctx->gl, shdr->gl.buf_vertex,  shdr->buffers.vertex.data.bytes,  shdr->buffers.vertex.size);
        gl_ok &= gl_buffer_set_element_data    (_renderer_ctx->gl, shdr->gl.buf_element, shdr->buffers.element.data.bytes, shdr->buffers.element.size);
        gl_ok &= gl_vertex_add_attribute_f32x3 (_renderer_ctx->gl, shdr->gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (_renderer_ctx->gl, shdr->gl.vertex, vertex_size, 1, 12);
        assert(gl_ok);
    
    }

    IFB_INTERNAL bool
    renderer_quad_push(
        const entity_id id) {

        assert(_renderer_ctx);
        assert(id != ENTITY_ID_INVALID);
        auto shdr = _renderer_ctx->shader.quad;

        const bool does_exist = quad_does_exist(id);
        assert(does_exist);
        

        const bool did_add = shdr->render_list.add(id);
        return(did_add);
    }

    IFB_INTERNAL void
    renderer_quad_draw(
        const mat4& view_proj_xform) {

        assert(_renderer_ctx);
        auto shdr = _renderer_ctx->shader.quad;
        assert(shdr);

        // get the number of quads and elements
        const u32 quad_count    = shdr->render_list.count();
        const u32 element_count = (quad_count * 6);
        if (element_count == 0) {
            return;
        } 

        // calculate the vertices
        for (
            u32 i = 0;
            i < quad_count;
            ++i) {

            const entity_id         quad_id  = shdr->render_list[i];
            renderer_quad_vertices& vertices = shdr->buffers.vertex.data.vertices[i];

            assert(renderer_quad_get_vertices(vertices, quad_id));
        }

        mat4 m = mat4_identity();

        orientation cam_ori;
        renderer_camera_get_orientation(cam_ori);

        // draw elements
        gl_context_set_shader_program (_renderer_ctx->gl, shdr->gl.program);
        gl_context_set_vertex_object  (_renderer_ctx->gl, shdr->gl.vertex);
        gl_context_set_buffer_vertex  (_renderer_ctx->gl, shdr->gl.buf_vertex);
        gl_buffer_update_vertex_data  (_renderer_ctx->gl, shdr->gl.buf_vertex,          shdr->buffers.vertex.data.bytes,  shdr->buffers.vertex.size);
        gl_uniform_set_mat4           (_renderer_ctx->gl, shdr->gl.unif_mat4_model,     m.m);
        gl_uniform_set_mat4           (_renderer_ctx->gl, shdr->gl.unif_mat4_view_proj, view_proj_xform.m);
        gl_context_draw_elements      (_renderer_ctx->gl, element_count);

        // reset the list
        shdr->render_list.reset();
    }
    
    IFB_INTERNAL bool
    renderer_quad_get_vertices(
        renderer_quad_vertices& vertices,
        const entity_id         quad_id) {

        orientation cam_ori;
        renderer_camera_get_orientation(cam_ori);

        quad_entity quad_entity = {0};
        const bool  result      = quad_lookup_by_id(quad_entity, quad_id); 
        if (result) {
            const color_rgba_f32 color         = color_rgba_f32(quad_entity.color.hex);
            const f32            offset_width  = quad_entity.dims.width  * 0.5f;
            const f32            offset_height = quad_entity.dims.height * 0.5f;

            // Keep the quad's horizontal axis locked to world X.
            const vec3 quad_right = { 1.0f, 0.0f, 0.0f };

            // Only consider the camera's pitch.
            // Remove the X component so camera yaw has no effect.
            vec3 pitch_forward = {
                0.0f,
                cam_ori.forward.y,
                cam_ori.forward.z
            };

            pitch_forward = vec3_normalize(pitch_forward);

            // Construct an up vector perpendicular to the pitch direction.
            const vec3 quad_up = {
                0.0f,
               -pitch_forward.z,
                pitch_forward.y
            };

            const vec3 right = vec3_scalar_multiply(quad_right, offset_width);
            const vec3 up    = vec3_scalar_multiply(quad_up,    offset_height);

            // top right
            vertices.top_right.pos_x = quad_entity.pos.x + right.x + up.x;
            vertices.top_right.pos_y = quad_entity.pos.y + right.y + up.y;
            vertices.top_right.pos_z = quad_entity.pos.z + right.z + up.z;
            vertices.top_right.color_r = color.r;
            vertices.top_right.color_g = color.g;
            vertices.top_right.color_b = color.b;
            vertices.top_right.color_a = color.a;

            // bottom right
            vertices.bottom_right.pos_x = quad_entity.pos.x + right.x - up.x;
            vertices.bottom_right.pos_y = quad_entity.pos.y + right.y - up.y;
            vertices.bottom_right.pos_z = quad_entity.pos.z + right.z - up.z;
            vertices.bottom_right.color_r = color.r;
            vertices.bottom_right.color_g = color.g;
            vertices.bottom_right.color_b = color.b;
            vertices.bottom_right.color_a = color.a;

            // bottom left
            vertices.bottom_left.pos_x = quad_entity.pos.x - right.x - up.x;
            vertices.bottom_left.pos_y = quad_entity.pos.y - right.y - up.y;
            vertices.bottom_left.pos_z = quad_entity.pos.z - right.z - up.z;
            vertices.bottom_left.color_r = color.r;
            vertices.bottom_left.color_g = color.g;
            vertices.bottom_left.color_b = color.b;
            vertices.bottom_left.color_a = color.a;

            // top left
            vertices.top_left.pos_x = quad_entity.pos.x - right.x + up.x;
            vertices.top_left.pos_y = quad_entity.pos.y - right.y + up.y;
            vertices.top_left.pos_z = quad_entity.pos.z - right.z + up.z;
            vertices.top_left.color_r = color.r;
            vertices.top_left.color_g = color.g;
            vertices.top_left.color_b = color.b;
            vertices.top_left.color_a = color.a;
        }
        return(result);
    }
};
