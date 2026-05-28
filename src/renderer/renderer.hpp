#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb.hpp"


namespace ifb {

    struct renderer {
        struct {
            void* ptr;
            u32   size;            
        } reserved_memory;
        struct {
            gl_program quad;
        } shader;
    };

    struct quad {
        u32 x;
        u32 y;
    };


    struct shader_pipeline {
        gl_shader vertex;
        gl_shader tess_ctrl;
        gl_shader tess_eval;
        gl_shader fragment;
        gl_shader geometry;
        gl_shader compute;
    };

    u32       renderer_memory_requirement (void);
    renderer* renderer_init_from_memory   (const u32 mem_size, void* mem_ptr);
    void      renderer_startup            (renderer* rndr, const u32 reserved_mem_size, void*       reserved_mem_ptr);
    void      renderer_add_quad_shader    (renderer* rndr, const u32 shader_src_size,   const byte* shader_src_ptr);
    u32       renderer_push_quad          (renderer* rndr, const u32 quad_count,        const quad* quad_ptr);
    u32       renderer_draw_quads         (renderer* rndr);
    void      renderer_shutdown           (renderer* rndr);

};

#endif //RENDERER_HPP