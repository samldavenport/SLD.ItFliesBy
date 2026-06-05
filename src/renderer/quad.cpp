#pragma once

#include "renderer.hpp"

namespace ifb {
    
    IFB_INTERNAL void
    renderer_quad_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        assert(
            ctx              != NULL &&
            src_vertex.data   != NULL &&
            src_vertex.size   != 0    &&
            src_fragment.data != NULL &&
            src_fragment.size != 0
        );

        // create gl objects
        const gl_shader  shader_vtx   = gl_shader_stage_create_vertex   (ctx->gl);
        const gl_shader  shader_frg   = gl_shader_stage_create_fragment (ctx->gl);
        const gl_program shader_prg   = gl_shader_program_create        (ctx->gl);
        const gl_vertex  vertex       = gl_vertex_create                (ctx->gl); 
        const gl_buffer  buffer_vtx   = gl_buffer_create                (ctx->gl);
        const gl_buffer  buffer_elmnt = gl_buffer_create                (ctx->gl);
        assert(
            shader_vtx   != GL_ID_INVALID &&
            shader_frg   != GL_ID_INVALID &&
            shader_prg   != GL_ID_INVALID &&
            buffer_vtx   != GL_ID_INVALID &&
            buffer_elmnt != GL_ID_INVALID && 
            vertex       != GL_ID_INVALID 
        );

        // compile and link shader pipeline
        bool shader_is_valid = true;
        shader_is_valid &= gl_shader_stage_compile_from_source (ctx->gl, shader_vtx, src_vertex.data,   src_vertex.size);
        shader_is_valid &= gl_shader_stage_compile_from_source (ctx->gl, shader_frg, src_fragment.data, src_fragment.size);
        shader_is_valid &= gl_shader_program_attach_stage      (ctx->gl, shader_prg, shader_vtx);
        shader_is_valid &= gl_shader_program_attach_stage      (ctx->gl, shader_prg, shader_frg);
        shader_is_valid &= gl_shader_program_link              (ctx->gl, shader_prg);
        assert(shader_is_valid);

        // destroy shader stages
        gl_shader_stage_destroy (ctx->gl, shader_vtx);
        gl_shader_stage_destroy (ctx->gl, shader_frg);

        // allocate memory
        auto buffer_vertex  = (quad_vertices*)renderer_memory_commit(ctx);
        auto buffer_element = (quad_elements*)renderer_memory_commit(ctx);
        assert(
            buffer_vertex  != NULL &&
            buffer_element != NULL            
        );

        // set buffer data
        bool did_set_data = true;
        did_set_data &= gl_buffer_set_vertex_data  (ctx->gl, buffer_vtx,   (byte*)buffer_vertex,  ctx->mem.granularity);
        did_set_data &= gl_buffer_set_element_data (ctx->gl, buffer_elmnt, (byte*)buffer_element, ctx->mem.granularity);
        assert(did_set_data);

        // set vertex attributes
        const u32 vertex_size         = sizeof(quad_vertex);
        const u32 vertex_offset_pos   = 0;
        const u32 vertex_offset_color = sizeof(vec3);
        const u32 vertex_offset_scale = vertex_offset_color + sizeof(color_rgba_f32);  
        bool      attribs_are_valid   = true;
        attribs_are_valid &= gl_vertex_add_attribute_f32x3 (ctx->gl, vertex, vertex_size, 0, vertex_offset_pos);
        attribs_are_valid &= gl_vertex_add_attribute_f32x4 (ctx->gl, vertex, vertex_size, 1, vertex_offset_color);
        attribs_are_valid &= gl_vertex_add_attribute_f32x1 (ctx->gl, vertex, vertex_size, 2, vertex_offset_scale);
        assert(attribs_are_valid);

        // update renderer
        ctx->quad_shader.quad_buffer_vertices = buffer_vertex;
        ctx->quad_shader.quad_buffer_elements = buffer_element;
        ctx->quad_shader.quad_capacity        = ctx->mem.granularity / sizeof(quad_vertices);  
        ctx->quad_shader.quad_count           = 0;
        ctx->quad_shader.gl.program           = shader_prg;
        ctx->quad_shader.gl.vertex            = vertex;
        ctx->quad_shader.gl.buf_vertex        = buffer_vtx;
        ctx->quad_shader.gl.buf_element       = buffer_elmnt;
    }

    IFB_INTERNAL void
    renderer_quad_buffer_destroy(
        renderer_context* ctx,
        quad_buffer*      qb) {

        assert(
            ctx != NULL &&
            qb  != NULL &&
            gl_buffer_destroy(ctx->gl, qb->gl_buf)
        );

        renderer_memory_decommit(ctx, (void*)qb);
    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer_context* ctx,
        const quad*       q_ptr,
        const u32         q_count) {

        assert(
            ctx      != NULL &&
            q_ptr    != NULL &&
            q_count  != 0
        );

        u32 num_pushed = 0;

        for (
            u32& index = ctx->quad_shader.quad_count;
            (
                index < q_count && 
                index < ctx->quad_shader.quad_capacity
            );
            ++index
        ) {

            const auto& quad               = q_ptr[index]; 
            const auto  color              = color_rgba_f32(quad.color.hex);   
            const f32   x_offset           = (quad.width  / 2.0f);
            const f32   y_offset           = (quad.height / 2.0f);
            const u32   element_offset     = (6 * index); 
            auto&       vtx                = ctx->quad_shader.quad_buffer_vertices [index];
            auto&       elmnt              = ctx->quad_shader.quad_buffer_elements [index];

            vtx.bottom_left.position.x     = quad.pos.x - x_offset;
            vtx.bottom_left.position.y     = quad.pos.y - y_offset;
            vtx.bottom_left.position.z     = quad.pos.z; 
            vtx.bottom_left.color          = color;
            vtx.bottom_left.scale          = quad.scale; 
            
            vtx.bottom_right.position.x    = quad.pos.x + x_offset;
            vtx.bottom_right.position.y    = quad.pos.y - y_offset;
            vtx.bottom_right.position.z    = quad.pos.z; 
            vtx.bottom_right.color         = color;
            vtx.bottom_right.scale         = quad.scale; 
            
            vtx.top_left.position.x        = quad.pos.x - x_offset;
            vtx.top_left.position.y        = quad.pos.y + y_offset;
            vtx.top_left.position.z        = quad.pos.z; 
            vtx.top_left.color             = color;
            vtx.top_left.scale             = quad.scale; 
            
            vtx.top_right.position.x       = quad.pos.x + x_offset;
            vtx.top_right.position.y       = quad.pos.y + y_offset;
            vtx.top_right.position.z       = quad.pos.z; 
            vtx.top_right.color            = color;
            vtx.top_right.scale            = quad.scale;

            elmnt.data[element_offset    ] = QUAD_BASE_INDICES[1];             
            elmnt.data[element_offset + 1] = QUAD_BASE_INDICES[2];             
            elmnt.data[element_offset + 2] = QUAD_BASE_INDICES[3];             
            elmnt.data[element_offset + 3] = QUAD_BASE_INDICES[4];             
            elmnt.data[element_offset + 4] = QUAD_BASE_INDICES[5];             
            elmnt.data[element_offset + 5] = QUAD_BASE_INDICES[6];             

            ++num_pushed;
        }

        return(num_pushed);
    }

    IFB_INTERNAL u32
    renderer_quad_draw_all(
        renderer_context* ctx) {

        assert(ctx);

        auto& shdr = ctx->quad_shader;

        const u32 element_count = (shdr.quad_count * 6);

        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_set_buffer_vertex  (ctx->gl, shdr.gl.buf_element);
        gl_context_set_buffer_element (ctx->gl, shdr.gl.buf_element);
        gl_context_draw_elements      (ctx->gl, element_count);

        shdr.quad_count = 0;

        return(element_count);
    }
};