#pragma once

#include "renderer.hpp"

namespace ifb {

    static entity_id   _test_quad_id =  0;
    static quad_entity _test_quad    = {0};

    IFB_INTERNAL void
    renderer_quad_shader_init(
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(_renderer_ctx);
        assert(sizeof(vec3)           == 12);
        assert(sizeof(color_rgba_f32) == 16);

        // create a test quad
        quad q = {0};
        q.color.hex         = 0xFF0000FF;
        q.dimensions.width  = 0.2;
        q.dimensions.height = 0.2;
        q.position          = {0};
        _test_quad_id       = quad_create("HELLO-QUAD",q);
        assert(_test_quad_id != ENTITY_ID_INVALID);

        auto& shdr = _renderer_ctx->shader.quad;
        

        // set the element data
        const auto& cfg = config_instance();
        for (
            u32 i = 0;
            i < cfg.quad_capacity;
            ++i) {

            const u32 offset   = (i * 4);
            auto&     elements = shdr.buffers.element.data.elements[i];
            elements.elmnt_0_index_0 = (offset);  
            elements.elmnt_1_index_1 = (offset + 1); 
            elements.elmnt_2_index_3 = (offset + 3); 
            elements.elmnt_3_index_1 = (offset + 1); 
            elements.elmnt_4_index_2 = (offset + 2); 
            elements.elmnt_5_index_3 = (offset + 3); 
        }

        // create gl objects
        shdr.gl.program          = gl_shader_program_create        (_renderer_ctx->gl);
        shdr.gl.vertex           = gl_vertex_create                (_renderer_ctx->gl);
        shdr.gl.buf_vertex       = gl_buffer_create                (_renderer_ctx->gl); 
        shdr.gl.buf_element      = gl_buffer_create                (_renderer_ctx->gl);
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (_renderer_ctx->gl);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (_renderer_ctx->gl);


        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (_renderer_ctx->gl, shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (_renderer_ctx->gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (_renderer_ctx->gl, shdr.gl.program,  shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (_renderer_ctx->gl, shdr.gl.program,  shdr_frg);
        gl_ok &= gl_shader_program_link              (_renderer_ctx->gl, shdr.gl.program);
        gl_shader_stage_destroy                      (_renderer_ctx->gl, shdr_vtx);
        gl_shader_stage_destroy                      (_renderer_ctx->gl, shdr_frg);
        assert(gl_ok);

        // define vertex
        const u32 vertex_size  = sizeof(vec3) + sizeof(vec4); 
        gl_ok &= gl_context_set_vertex_object  (_renderer_ctx->gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (_renderer_ctx->gl, shdr.gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (_renderer_ctx->gl, shdr.gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (_renderer_ctx->gl, shdr.gl.buf_vertex,  shdr.buffers.vertex.data.bytes,  shdr.buffers.vertex.size);
        gl_ok &= gl_buffer_set_element_data    (_renderer_ctx->gl, shdr.gl.buf_element, shdr.buffers.element.data.bytes, shdr.buffers.element.size);
        gl_ok &= gl_vertex_add_attribute_f32x3 (_renderer_ctx->gl, shdr.gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (_renderer_ctx->gl, shdr.gl.vertex, vertex_size, 1, 12);
        assert(gl_ok);
    }

    IFB_INTERNAL void
    renderer_quad_draw(
        void) {

        renderer_quad_vertices vertices;
        assert(renderer_quad_get_vertices(vertices, _test_quad_id));

        auto& shdr = _renderer_ctx->shader.quad;

        for (
            u32 i = 0;
            i < 28;
            ++i) {

            shdr.buffers.vertex.data.floats[i] = vertices.floats[i];
        }

        gl_context_set_shader_program (_renderer_ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (_renderer_ctx->gl, shdr.gl.vertex);
        gl_buffer_update_vertex_data  (_renderer_ctx->gl, shdr.gl.buf_vertex,  shdr.buffers.vertex.data.bytes,  shdr.buffers.vertex.size);
        gl_context_draw_elements      (_renderer_ctx->gl, 6);
    }

    
    IFB_INTERNAL bool
    renderer_quad_get_vertices(
        renderer_quad_vertices& vertices,
        const entity_id         quad_id) {

        quad_entity quad_entity = {0};
        const bool  result      = quad_lookup_by_id(quad_entity, quad_id); 
        if (result) {

            const color_rgba_f32 color         = color_rgba_f32(quad_entity.color.hex);
            const f32            offset_width  = (quad_entity.dims.width  * 0.5f);
            const f32            offset_height = (quad_entity.dims.height * 0.5f);

            // top right
            vertices.top_right.pos_x      = (quad_entity.pos.x + offset_width);
            vertices.top_right.pos_y      = (quad_entity.pos.y + offset_height);
            vertices.top_right.pos_z      = 0.0f;
            vertices.top_right.color_r    = color.r;
            vertices.top_right.color_g    = color.g;
            vertices.top_right.color_b    = color.b;
            vertices.top_right.color_a    = color.a;

            // bottom right
            vertices.bottom_right.pos_x   = (quad_entity.pos.x + offset_width);
            vertices.bottom_right.pos_y   = (quad_entity.pos.y - offset_height);
            vertices.bottom_right.pos_z   = 0.0f;
            vertices.bottom_right.color_r = color.r;
            vertices.bottom_right.color_g = color.g;
            vertices.bottom_right.color_b = color.b;
            vertices.bottom_right.color_a = color.a;

            // bottom left
            vertices.bottom_left.pos_x    = (quad_entity.pos.x - offset_width);
            vertices.bottom_left.pos_y    = (quad_entity.pos.y - offset_height);
            vertices.bottom_left.pos_z    = 0.0f;
            vertices.bottom_left.color_r  = color.r;
            vertices.bottom_left.color_g  = color.g;
            vertices.bottom_left.color_b  = color.b;
            vertices.bottom_left.color_a  = color.a;

            // top left
            vertices.top_left.pos_x       = (quad_entity.pos.x - offset_width);
            vertices.top_left.pos_y       = (quad_entity.pos.y + offset_height);
            vertices.top_left.pos_z       = 0.0f;
            vertices.top_left.color_r     = color.r;
            vertices.top_left.color_g     = color.g;
            vertices.top_left.color_b     = color.b;
            vertices.top_left.color_a     = color.a;
        }
        return(result);
    }
};