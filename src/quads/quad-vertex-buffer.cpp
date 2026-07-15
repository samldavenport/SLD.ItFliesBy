#pragma once

#include "quad.hpp"
#include "eng-internal.hpp"

namespace ifb {

    IFB_INTERNAL u32
    quad_vertex_buffer_size(
        void) {

        const u32 size_struct = sizeof(quad_vertex_buffer);
        const u32 size_data = (
            4                   *
            sizeof(quad_vertex) *
            _quad_mngr->all.count
        );

        const u32 size_total = size_struct + size_data;
        return(size_total);
    }

    IFB_INTERNAL quad_vertex_buffer*
    quad_vertex_buffer_create(
        memory& mem) {

        const u32 size_requred = quad_vertex_buffer_size();
        assert(
            mem.size    != 0            &&
            mem.size    == size_requred &&
            mem.address != 0
        );

        auto vtx_buf = (quad_vertex_buffer*)mem.ptr;
        vtx_buf->data.bytes = (byte*)(mem.address + sizeof(quad_vertex_buffer));
        vtx_buf->data_size  =        (mem.size    - sizeof(quad_vertex_buffer));

        return(vtx_buf);
    }

    IFB_INTERNAL void
    quad_vertex_buffer_render(
        quad_vertex_buffer* vtx_buffer) {

        quad_vertex_buffer_validate(vtx_buffer);

        // 4 vertices per quad
        vtx_buffer->vertex_count = (4 * _quad_mngr->to_render.count);

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
            quad_vertex& vtx_top_right    = vtx_buffer->data.vertices[vtx_index++];
            quad_vertex& vtx_bottom_right = vtx_buffer->data.vertices[vtx_index++];
            quad_vertex& vtx_bottom_left  = vtx_buffer->data.vertices[vtx_index++];
            quad_vertex& vtx_top_left     = vtx_buffer->data.vertices[vtx_index++];

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
    }

    IFB_INTERNAL void
    quad_vertex_buffer_validate(
        const quad_vertex_buffer* vtx_buffer) {

        assert(
            vtx_buffer                  != NULL                 &&
            vtx_buffer->data.bytes      != NULL                 &&
            vtx_buffer->vertex_count    <= _quad_mngr->capacity
        );
    }

};