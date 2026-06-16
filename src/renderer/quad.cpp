#pragma once

#include "renderer.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 QUAD_VERTEX_SIZE       = sizeof(f32) * 7;
    static constexpr u32 QUAD_VERTEX_COUNT      = 4;
    static constexpr u32 QUAD_DATA_SIZE         = QUAD_VERTEX_SIZE * 4; 
    static constexpr u32 QUAD_ELEMENT_COUNT     = 6;
    static constexpr u32 QUAD_ELEMENT_DATA_SIZE = sizeof(u32) * QUAD_ELEMENT_COUNT;
    
    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_vertex {
        union {
            struct {
                struct {
                    f32 x;  // 0
                    f32 y;  // 4
                    f32 z;  // 8
                } attrib_0_pos;
                struct {
                    f32 r; // 12
                    f32 g; // 16
                    f32 b; // 20
                    f32 a; // 24
                } attrib_1_color;
            };
            byte data[QUAD_VERTEX_SIZE];
        };
    };

    struct quad_vertices {
        union {
            struct {
                quad_vertex top_right;
                quad_vertex bottom_right;
                quad_vertex bottom_left;
                quad_vertex top_left;
            };
            quad_vertex array[QUAD_VERTEX_COUNT];
            byte        data [QUAD_DATA_SIZE];
        };
    };

    struct quad_elements {
        union {
            struct {
                struct {
                    u32 elmnt_0_index_0;
                    u32 elmnt_1_index_1;
                    u32 elmnt_2_index_3;
                } triangle_1;
                struct {
                    u32 elmnt_3_index_1;
                    u32 elmnt_4_index_2;
                    u32 elmnt_5_index_3;
                } triangle_2;
            };
            u32  array [QUAD_ELEMENT_COUNT];
            byte data  [QUAD_ELEMENT_DATA_SIZE];
        };
    };

    struct quad_vertex_buffer {
        u32            capacity;
        u32            count;
        quad_vertices* data;
    };

    struct quad_element_buffer {
        u32            capacity;
        u32            count;
        quad_elements* data;
    };

    struct quad_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;
            gl_buffer  buffer_vtx;
            gl_buffer  buffer_elmnt;
            gl_shader  shdr_vtx;
            gl_shader  shdr_frg;
        } gl;
        quad_vertex_buffer  buffer_vtx;
        quad_element_buffer buffer_elmnt;
    };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

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

    }

    IFB_INTERNAL u32
    renderer_quad_push(
        renderer_context* ctx,
        const quad*       q,
        const u32         count) {

        return(0);
    }

    IFB_INTERNAL u32
    renderer_quad_draw(
        renderer_context* ctx) {

        return(0);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

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
    }
};