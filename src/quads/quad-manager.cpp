#pragma once

#include "quad.hpp"
#include "eng-internal.hpp"

namespace ifb {


    IFB_INTERNAL quad_mngr*
    quad_mngr_create(
        void) {

        _quad_mngr = global_alloc<quad_mngr>();
        assert(_quad_mngr);

        return(_quad_mngr);
    }

    IFB_INTERNAL void
    quad_mngr_validate(
        void) {

        assert(_quad_mngr);
    }

    IFB_INTERNAL void
    quad_mngr_startup(
        memory& mem) {

        const auto& cfg = config_instance();

        // initialize the stack
        memory stack_mem;
        stack_mem.ptr  = pfm_memory_commit(mem.ptr, 0, mem.size);
        stack_mem.size = mem.size;
        _quad_mngr->stack_mem.init(stack_mem);
        _quad_mngr->capacity = cfg.quad_capacity;

        // allocate list memory
        auto quads_all       = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        auto quads_to_render = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        auto quads_to_update = _quad_mngr->stack_mem.push_struct<entity_id>(cfg.quad_capacity);
        assert(
            _quad_mngr->capacity != 0    &&            
            quads_all            != NULL &&
            quads_to_render      != NULL &&
            quads_to_update      != NULL            
        );

        // initialize lists
        _quad_mngr->all.array       = quads_all;
        _quad_mngr->all.count       = 0;
        _quad_mngr->to_render.array = quads_to_render;
        _quad_mngr->to_render.count = 0;
        _quad_mngr->to_update.array = quads_to_update;
        _quad_mngr->to_update.count = 0;
    }

    IFB_INTERNAL quad_vertex_buffer*
    quad_mngr_get_vertex_buffer(
        arena* a) {

        if (_quad_mngr->to_render.count == 0) {
            return(NULL);
        }

        // 4 vertices per quad
        const u32 vtx_count = 4 * _quad_mngr->to_render.count;

        // allocate memory
        auto buf_struct    = arena_push<quad_vertex_buffer> (a);
        auto quad_vertices = arena_push<quad_vertex>        (a, vtx_count); 
        assert(
            buf_struct    != NULL &&
            quad_vertices != NULL            
        );

        // initialize list
        buf_struct->vertices     = quad_vertices;
        buf_struct->vertex_count = _quad_mngr->to_render.count;
        buf_struct->data_size    = sizeof(quad_vertex) * buf_struct->vertex_count;

        quad_entity curr_quad = {0};
        u32         vtx_index = 0;
        for (
            u32 quad_index = 0;
                quad_index < _quad_mngr->to_render.count;
              ++quad_index
        ) {
            
            // get the quad info
            const entity_id id       = _quad_mngr->to_render.array[quad_index]; 
            const bool      did_find = quad_lookup_by_id(curr_quad, id);
            assert(did_find);

            // normalize the color
            const color_rgba_f32 color(curr_quad.color.hex);

            // calculate x and y offsets
            const f32 offset_x = (curr_quad.dims.width  / 2);
            const f32 offset_y = (curr_quad.dims.height / 2);

            // get the vertices
            quad_vertex& vtx_top_right    = quad_vertices[vtx_index++];
            quad_vertex& vtx_bottom_right = quad_vertices[vtx_index++];
            quad_vertex& vtx_bottom_left  = quad_vertices[vtx_index++];
            quad_vertex& vtx_top_left     = quad_vertices[vtx_index++];

            // top right
            vtx_top_right.pos_x   = curr_quad.pos.x + offset_x; 
            vtx_top_right.pos_y   = curr_quad.pos.y + offset_y; 
            vtx_top_right.pos_z   = curr_quad.pos.z; 
            vtx_top_right.color_r = color.r;
            vtx_top_right.color_g = color.g;
            vtx_top_right.color_b = color.b;
            vtx_top_right.color_a = color.a;
            
            // bottom right
            vtx_top_right.pos_x   = curr_quad.pos.x + offset_x; 
            vtx_top_right.pos_y   = curr_quad.pos.y - offset_y; 
            vtx_top_right.pos_z   = curr_quad.pos.z; 
            vtx_top_right.color_r = color.r;
            vtx_top_right.color_g = color.g;
            vtx_top_right.color_b = color.b;
            vtx_top_right.color_a = color.a;
            
            // bottom left
            vtx_top_right.pos_x   = curr_quad.pos.x - offset_x; 
            vtx_top_right.pos_y   = curr_quad.pos.y - offset_y; 
            vtx_top_right.pos_z   = curr_quad.pos.z; 
            vtx_top_right.color_r = color.r;
            vtx_top_right.color_g = color.g;
            vtx_top_right.color_b = color.b;
            vtx_top_right.color_a = color.a;
            
            // top left
            vtx_top_right.pos_x   = curr_quad.pos.x - offset_x; 
            vtx_top_right.pos_y   = curr_quad.pos.y + offset_y; 
            vtx_top_right.pos_z   = curr_quad.pos.z; 
            vtx_top_right.color_r = color.r;
            vtx_top_right.color_g = color.g;
            vtx_top_right.color_b = color.b;
            vtx_top_right.color_a = color.a;
        }

        return(buf_struct);
    }

    IFB_INTERNAL void
    quad_mngr_render_list_reset(
        void) {

        quad_mngr_validate();

        _quad_mngr->to_render.count = 0;
    }
};