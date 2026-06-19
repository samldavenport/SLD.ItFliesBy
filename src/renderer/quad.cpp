#pragma once

#include "renderer.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline void quad_shader_commit_memory            (renderer_context* ctx);
    inline void quad_shader_validate                 (const quad_shader& shdr);
    inline void quad_shader_create_gl_objects        (quad_shader& shdr, gl_context* gl);
    inline void quad_shader_compile_and_link_program (quad_shader& shdr, gl_context* gl, const shader_source& src_vertex, const shader_source& src_fragment);
    inline void quad_shader_define_vertex            (quad_shader& shdr, gl_context* gl);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_quad_shader_init(
        renderer_context*    ctx,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        quad_shader_commit_memory            (ctx);
        quad_shader_create_gl_objects        (ctx->shader.quad, ctx->gl);
        quad_shader_compile_and_link_program (ctx->shader.quad, ctx->gl, src_vertex, src_fragment);
        quad_shader_define_vertex            (ctx->shader.quad, ctx->gl);
    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer_context* ctx,
        const quad*       q,
        const u32         count) {

        // validate args
        assert(ctx != NULL && q != 0);
        
        // cache properties and validate shader
        auto& shdr      = ctx->shader.quad;
        quad_shader_validate(shdr);


        // determine the actual push quantity
        const u32 count_available = shdr.buffers.quad_capacity - shdr.buffers.quad_count; 
        const u32 count_to_push   = (count_available > count) ? count : (count - count_available);   

        for (
            u32 index_push = 0;
                index_push < count_to_push;
              ++index_push) {

            // cache properties
            const quad& quad_curr = q[index_push];
            const u32   index_quad = shdr.buffers.quad_count; 
            auto&       vtx       = shdr.buffers.vertices.array [index_quad];
            auto&       elmnt     = shdr.buffers.elements.array [index_quad]; 
        
            // calculate position
            const f32 width_offset  = (quad_curr.dimensions.width  / 2);
            const f32 height_offset = (quad_curr.dimensions.height / 2);
            const f32 x_left        = (quad_curr.position.x - width_offset);
            const f32 x_right       = (quad_curr.position.x + width_offset);
            const f32 y_top         = (quad_curr.position.y + height_offset);
            const f32 y_bottom      = (quad_curr.position.y - height_offset);

            // normalize color
            const color_rgba_f32 color_f32(quad_curr.color.hex);

            // top right
            vtx.top_right.attrib_0_pos.x      = x_right;
            vtx.top_right.attrib_0_pos.y      = y_top;
            vtx.top_right.attrib_0_pos.z      = quad_curr.position.z;
            vtx.top_right.attrib_1_color.r    = color_f32.r;
            vtx.top_right.attrib_1_color.g    = color_f32.g;
            vtx.top_right.attrib_1_color.b    = color_f32.b;
            vtx.top_right.attrib_1_color.a    = color_f32.a;

            // bottom right
            vtx.bottom_right.attrib_0_pos.x   = x_right;
            vtx.bottom_right.attrib_0_pos.y   = y_bottom;
            vtx.bottom_right.attrib_0_pos.z   = quad_curr.position.z;
            vtx.bottom_right.attrib_1_color.r = color_f32.r;
            vtx.bottom_right.attrib_1_color.g = color_f32.g;
            vtx.bottom_right.attrib_1_color.b = color_f32.b;
            vtx.bottom_right.attrib_1_color.a = color_f32.a;

            // bottom left
            vtx.bottom_left.attrib_0_pos.x    = x_left;
            vtx.bottom_left.attrib_0_pos.y    = y_bottom;
            vtx.bottom_left.attrib_0_pos.z    = quad_curr.position.z;
            vtx.bottom_left.attrib_1_color.r  = color_f32.r;
            vtx.bottom_left.attrib_1_color.g  = color_f32.g;
            vtx.bottom_left.attrib_1_color.b  = color_f32.b;
            vtx.bottom_left.attrib_1_color.a  = color_f32.a;

            // top left
            vtx.top_left.attrib_0_pos.x       = x_left;
            vtx.top_left.attrib_0_pos.y       = y_top;
            vtx.top_left.attrib_0_pos.z       = quad_curr.position.z;
            vtx.top_left.attrib_1_color.r     = color_f32.r;
            vtx.top_left.attrib_1_color.g     = color_f32.g;
            vtx.top_left.attrib_1_color.b     = color_f32.b;
            vtx.top_left.attrib_1_color.a     = color_f32.a;

            // elements
            elmnt.triangle_1.elmnt_0_index_0  = index_quad + 0;
            elmnt.triangle_1.elmnt_1_index_1  = index_quad + 1;
            elmnt.triangle_1.elmnt_2_index_3  = index_quad + 3;
            elmnt.triangle_2.elmnt_3_index_1  = index_quad + 1;
            elmnt.triangle_2.elmnt_4_index_2  = index_quad + 2;
            elmnt.triangle_2.elmnt_5_index_3  = index_quad + 3;

            // update count
            ++shdr.buffers.quad_count;
        }

        return(count_to_push);
    }

    IFB_INTERNAL u32
    renderer_quad_draw(
        renderer_context* ctx) {

        assert(ctx);

        // validate shader
        auto& shdr = ctx->shader.quad;
        quad_shader_validate(shdr);

        // calculate counts
        const u32 quad_count    = shdr.buffers.quad_count;
        const u32 element_count = quad_count * QUAD_ELEMENT_COUNT;

        // draw elements
        gl_context_set_shader_program (ctx->gl, shdr.gl.program);
        gl_context_set_vertex_object  (ctx->gl, shdr.gl.vertex);
        gl_context_draw_elements      (ctx->gl, element_count);

        // reset counts and return
        shdr.buffers.quad_count = 0;
        return(quad_count);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    inline void
    quad_shader_commit_memory(
        renderer_context* ctx) {

        assert(ctx);
        auto& shdr = ctx->shader.quad;

        // commit memory
        void* mem = renderer_memory_commit(ctx);
        assert(mem);

        // calculate sizes
        const u32 size_mem          = ctx->mem.granularity;
        const u32 size_per_quad     = (QUAD_DATA_SIZE + QUAD_ELEMENT_DATA_SIZE);
        const u32 count_quads_total = (size_mem / size_per_quad);
        const u32 size_data_vtx     = count_quads_total * QUAD_VERTEX_SIZE;
        const u32 size_data_elmnt   = count_quads_total * QUAD_ELEMENT_DATA_SIZE;
        
        // quad capacity / count
        shdr.buffers.quad_capacity  = count_quads_total;
        shdr.buffers.quad_count     = 0;

        // vertex buffer
        shdr.buffers.vertices.size  = size_data_vtx; 
        shdr.buffers.vertices.vptr  = mem;

        // element buffer
        shdr.buffers.elements.size = size_data_elmnt;
        shdr.buffers.elements.addr = (shdr.buffers.vertices.addr + size_data_vtx);

        // make sure we are within bounds
        assert((size_data_vtx + size_data_elmnt) <= size_mem);
    }

    inline void
    quad_shader_validate(
        const quad_shader& shdr) {

        assert(
            shdr.gl.program      != GL_ID_INVALID &&
            shdr.gl.vertex       != GL_ID_INVALID &&
            shdr.gl.buffer_vtx   != GL_ID_INVALID &&
            shdr.gl.buffer_elmnt != GL_ID_INVALID
        );
    } 

    inline void
    quad_shader_create_gl_objects(
        quad_shader& shdr,
        gl_context*  gl) {
        
        shdr.gl.program      = gl_shader_program_create        (gl);
        shdr.gl.vertex       = gl_vertex_create                (gl);
        shdr.gl.buffer_vtx   = gl_buffer_create                (gl);
        shdr.gl.buffer_elmnt = gl_buffer_create                (gl);
        shdr.gl.shdr_vtx     = gl_shader_stage_create_vertex   (gl);
        shdr.gl.shdr_frg     = gl_shader_stage_create_fragment (gl);

        assert(
            shdr.gl.program      != GL_ID_INVALID &&
            shdr.gl.vertex       != GL_ID_INVALID &&
            shdr.gl.buffer_vtx   != GL_ID_INVALID &&
            shdr.gl.buffer_elmnt != GL_ID_INVALID &&
            shdr.gl.shdr_vtx     != GL_ID_INVALID &&
            shdr.gl.shdr_frg     != GL_ID_INVALID
        );
    }

    inline void
    quad_shader_compile_and_link_program(
        quad_shader&         shdr,
        gl_context*          gl,
        const shader_source& src_vertex,
        const shader_source& src_fragment) {

        bool gl_ok = true;

        // compile and link program
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr.gl.shdr_vtx, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr.gl.shdr_frg, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr.gl.program,  shdr.gl.shdr_vtx);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr.gl.program,  shdr.gl.shdr_frg);
        gl_ok &= gl_shader_program_link              (gl, shdr.gl.program);
        assert(gl_ok);

        // destroy the shader stages
        gl_shader_stage_destroy (gl, shdr.gl.shdr_vtx);
        gl_shader_stage_destroy (gl, shdr.gl.shdr_frg);
        shdr.gl.shdr_vtx = GL_ID_INVALID;
        shdr.gl.shdr_frg = GL_ID_INVALID;
    }

    inline void
    quad_shader_define_vertex(
        quad_shader& shdr,
        gl_context*  gl) {

        bool gl_ok = true;

        // set context objects
        gl_ok &= gl_context_set_vertex_object  (gl, shdr.gl.vertex);
        gl_ok &= gl_context_set_buffer_vertex  (gl, shdr.gl.buffer_vtx);
        gl_ok &= gl_context_set_buffer_element (gl, shdr.gl.buffer_elmnt);
        
        // set data
        gl_ok &= gl_buffer_set_vertex_data     (gl, shdr.gl.buffer_vtx,   shdr.buffers.vertices.data, shdr.buffers.vertices.size);
        gl_ok &= gl_buffer_set_element_data    (gl, shdr.gl.buffer_elmnt, shdr.buffers.elements.data, shdr.buffers.elements.size);
        
        // set attributes
        const u32 size_vtx = sizeof(quad_vertex);
        gl_ok &= gl_vertex_add_attribute_f32x3 (gl, shdr.gl.vertex, size_vtx, 0, 0);
        gl_ok &= gl_vertex_add_attribute_f32x4 (gl, shdr.gl.vertex, size_vtx, 1, 12);
    }
};