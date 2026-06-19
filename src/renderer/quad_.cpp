#pragma once

#include "renderer.hpp"

namespace ifb {

    static constexpr u32 QUAD_ELEMENTS[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    IFB_INTERNAL void
    renderer_quad_shader_init(
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

        auto& shdr = ctx->quad_shader;
        
        // commit memory
        // NOTE(SAM): we can optimize this to use one block
        void* buf_mem_vtx  = renderer_memory_commit(ctx);
        void* buf_mem_elmt = renderer_memory_commit(ctx); 
        assert(
            buf_mem_vtx  != NULL &&
            buf_mem_elmt != NULL
        );

        // initialize vertex buffer
        shdr.vtx_buffer.count    = 0;
        shdr.vtx_buffer.capacity = renderer_memory_element_count(ctx, sizeof(quad_vertex));
        shdr.vtx_buffer.elements = (quad_vertex*)buf_mem_vtx;
       
        // initialize element buffer
        shdr.elmnt_buffer.count    = 0;
        shdr.elmnt_buffer.capacity = renderer_memory_element_count(ctx, sizeof(u32));
        shdr.elmnt_buffer.elements = (u32*)buf_mem_elmt;

        // create gl objects
        shdr.gl.program          = gl_shader_program_create        (ctx->gl);
        shdr.gl.vertex           = gl_vertex_create                (ctx->gl);
        shdr.gl.buf_vertex       = gl_buffer_create                (ctx->gl); 
        shdr.gl.buf_element      = gl_buffer_create                (ctx->gl);
        const gl_shader shdr_vtx = gl_shader_stage_create_vertex   (ctx->gl);
        const gl_shader shdr_frg = gl_shader_stage_create_fragment (ctx->gl);
        assert(
            shdr.gl.program     != GL_ID_INVALID && 
            shdr.gl.vertex      != GL_ID_INVALID && 
            shdr.gl.buf_vertex  != GL_ID_INVALID && 
            shdr.gl.buf_element != GL_ID_INVALID && 
            shdr_vtx            != GL_ID_INVALID &&
            shdr_frg            != GL_ID_INVALID
        );

        bool gl_ok = true;

        // compile shader
        gl_ok &= gl_shader_stage_compile_from_source (ctx->gl, shdr_vtx, src_vertex.data, src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (ctx->gl, shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (ctx->gl, shdr.gl.program,  shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (ctx->gl, shdr.gl.program,  shdr_frg);
        gl_ok &= gl_shader_program_link              (ctx->gl, shdr.gl.program);
        gl_shader_stage_destroy                      (ctx->gl, shdr_vtx);
        gl_shader_stage_destroy                      (ctx->gl, shdr_frg);
        assert(gl_ok);

        // define vertex
        byte*     vertex_data_ptr   = (byte*)shdr.vtx_buffer.elements;
        const u32 vertex_data_size  = ctx->mem.granularity;
        byte*     element_data_ptr  = (byte*)shdr.elmnt_buffer.elements;
        const u32 element_data_size = ctx->mem.granularity; 
        const u32 vertex_size       = sizeof(quad_vertex); 
        gl_ok &= gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (ctx->gl, shdr.gl.buf_vertex);
        gl_ok &= gl_context_set_buffer_element (ctx->gl, shdr.gl.buf_element);
        gl_ok &= gl_buffer_set_vertex_data     (ctx->gl, shdr.gl.buf_vertex,  vertex_data_ptr,  vertex_data_size);
        gl_ok &= gl_buffer_set_element_data    (ctx->gl, shdr.gl.buf_element, element_data_ptr, element_data_size);
        gl_ok &= gl_vertex_add_attribute_f32x3 (ctx->gl, shdr.gl.vertex, vertex_size, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (ctx->gl, shdr.gl.vertex, vertex_size, 1, 12);
        assert(gl_ok);
    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer_context* ctx,
        const quad*       q,
        const u32         count) {

        assert(
            ctx                                  != NULL                                 &&
            ctx->quad_shader.gl.program          != GL_ID_INVALID                        && 
            ctx->quad_shader.gl.vertex           != GL_ID_INVALID                        && 
            ctx->quad_shader.gl.buf_vertex       != GL_ID_INVALID                        && 
            ctx->quad_shader.gl.buf_element      != GL_ID_INVALID                        &&
            ctx->quad_shader.vtx_buffer.capacity != 0                                    &&
            ctx->quad_shader.vtx_buffer.count    <= ctx->quad_shader.vtx_buffer.capacity &&
            ctx->quad_shader.vtx_buffer.elements != NULL
        );

        auto& shdr = ctx->quad_shader;

        const u32 count_remaining = (shdr.vtx_buffer.capacity - shdr.vtx_buffer.count);
        const u32 count_pushed    = (count_remaining < count)
            ? count_remaining
            : count;

        if (count_pushed != 0) {

            u32& vtx_index = shdr.vtx_buffer.count; 

            for (
                u32 quad_index = 0;
                    quad_index < count;
                  ++quad_index
            ) {

                // get the quad and vertex data
                const auto&          quad             = q[quad_index]; 
                const color_rgba_f32 color_normalized = color_rgba_f32(q->color.hex);
                auto&                vtx_0            = shdr.vtx_buffer.elements[vtx_index++];
                auto&                vtx_1            = shdr.vtx_buffer.elements[vtx_index++];
                auto&                vtx_2            = shdr.vtx_buffer.elements[vtx_index++];
                auto&                vtx_3            = shdr.vtx_buffer.elements[vtx_index++];

                // calculate x and y offsets
                const f32 offset_x = ((f32)quad.dimensions.width  / 2.0f);
                const f32 offset_y = ((f32)quad.dimensions.height / 2.0f);
                const f32 x_left    = quad.position.x - offset_x; 
                const f32 x_right   = quad.position.x + offset_x; 
                const f32 y_top     = quad.position.y + offset_y; 
                const f32 y_bottom  = quad.position.y - offset_y; 

                // top right
                vtx_0.pos_x   = x_right;
                vtx_0.pos_y   = y_top;
                vtx_0.pos_z   = 0.0f;
                vtx_0.color_r = color_normalized.r;
                vtx_0.color_g = color_normalized.g;
                vtx_0.color_b = color_normalized.b;
                vtx_0.color_a = color_normalized.a;

                // bottom right
                vtx_1.pos_x   = x_right;
                vtx_1.pos_y   = y_bottom;
                vtx_1.pos_z   = 0.0f;
                vtx_1.color_r = color_normalized.r;
                vtx_1.color_g = color_normalized.g;
                vtx_1.color_b = color_normalized.b;
                vtx_1.color_a = color_normalized.a;

                // bottom left
                vtx_2.pos_x   = x_left;
                vtx_2.pos_y   = y_bottom;
                vtx_2.pos_z   = 0.0f;
                vtx_2.color_r = color_normalized.r;
                vtx_2.color_g = color_normalized.g;
                vtx_2.color_b = color_normalized.b;
                vtx_2.color_a = color_normalized.a;

                // top left
                vtx_3.pos_x   = x_left;
                vtx_3.pos_y   = y_top;
                vtx_3.pos_z   = 0.0f;
                vtx_3.color_r = color_normalized.r;
                vtx_3.color_g = color_normalized.g;
                vtx_3.color_b = color_normalized.b;
                vtx_3.color_a = color_normalized.a;

                // elements
                const u32 elemnt_offset = (quad_index * 4); // 4 vertices per quad
                const u32 elment_start  = (quad_index * 6); // 6 elements per quad
                shdr.elmnt_buffer.elements[elment_start + 0] = (QUAD_ELEMENTS[0] + elemnt_offset);
                shdr.elmnt_buffer.elements[elment_start + 1] = (QUAD_ELEMENTS[1] + elemnt_offset);
                shdr.elmnt_buffer.elements[elment_start + 2] = (QUAD_ELEMENTS[2] + elemnt_offset);
                shdr.elmnt_buffer.elements[elment_start + 3] = (QUAD_ELEMENTS[3] + elemnt_offset);
                shdr.elmnt_buffer.elements[elment_start + 4] = (QUAD_ELEMENTS[4] + elemnt_offset);
                shdr.elmnt_buffer.elements[elment_start + 5] = (QUAD_ELEMENTS[5] + elemnt_offset);
            }
        }

        return(count_pushed);
    }

    IFB_INTERNAL u32
    renderer_quad_draw(
        renderer_context* ctx) {
        
        assert(
            ctx                                  != NULL                                 &&
            ctx->quad_shader.gl.program          != GL_ID_INVALID                        && 
            ctx->quad_shader.gl.vertex           != GL_ID_INVALID                        && 
            ctx->quad_shader.gl.buf_vertex       != GL_ID_INVALID                        && 
            ctx->quad_shader.gl.buf_element      != GL_ID_INVALID                        &&
            ctx->quad_shader.vtx_buffer.capacity != 0                                    &&
            ctx->quad_shader.vtx_buffer.count    <= ctx->quad_shader.vtx_buffer.capacity &&
            ctx->quad_shader.vtx_buffer.elements != NULL
        );

        auto& shdr = ctx->quad_shader;

        // calculate total elements
        // should be 6 indexes per quad
        static const u32 vertex_count_per_quad = 4; 
        static const u32 element_count_per_quad = sizeof(QUAD_ELEMENTS) / sizeof(u32);
        const u32        element_count_total    = (element_count_per_quad * shdr.vtx_buffer.count);  

        // draw quads
        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (ctx->gl, 6);
    
        // reset the counter
        const u32 count_drawn = shdr.vtx_buffer.count;
        shdr.vtx_buffer.count = 0;
        return(count_drawn);
    }
};