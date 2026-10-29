#pragma once

#include <cassert>
#include "eng-types.hpp"

#include "ifb.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "ifb-particle.hpp"
#include "quad-internal.hpp"
#include "sld-math-vec3.hpp"
#include "sld.hpp"

namespace ifb {

    // renders the live particles of an emitter as quads that face the camera
    // returns the count of quads that were written to the buffer
    inline u32
    particle_emitter_render_quad_vertices(
        const particle_emitter& emitter,
        quad_vertices*          vertex_buffer,
        const u32               quad_capacity,
        const orientation&      camera_orientation) {

        // every particle in the emitter is the same size
        const vec3 right = vec3_scalar_multiply(camera_orientation.right, emitter.width  * 0.5f);
        const vec3 up    = vec3_scalar_multiply(camera_orientation.up,    emitter.height * 0.5f);

        u32 quad_count = 0;
        for (
            u32 index = 0;
                index < emitter.count;
              ++index) {

            if (emitter.array_life_ms[index] <= 0.0f) continue;

            // if the buffer is full, the rest of the particles aren't rendered
            if (quad_count == quad_capacity) break;

            const vec3&          position = emitter.array_position[index];
            const color_rgba_f32 color    = color_rgba_f32(emitter.array_colors[index].hex);

            quad_vertices& vertices = vertex_buffer[quad_count];
            vertices.top_right.position    = vec3_add      (vec3_add      (position, right), up);
            vertices.bottom_right.position = vec3_subtract (vec3_add      (position, right), up);
            vertices.bottom_left.position  = vec3_subtract (vec3_subtract (position, right), up);
            vertices.top_left.position     = vec3_add      (vec3_subtract (position, right), up);
            vertices.top_right.color       = color;
            vertices.bottom_right.color    = color;
            vertices.bottom_left.color     = color;
            vertices.top_left.color        = color;

            // particles don't have sprites, they're a solid color
            vertices.top_right.text_coords    = { 0.0f, 0.0f };
            vertices.bottom_right.text_coords = { 0.0f, 0.0f };
            vertices.bottom_left.text_coords  = { 0.0f, 0.0f };
            vertices.top_left.text_coords     = { 0.0f, 0.0f };
            vertices.top_right.textured       = 0.0f;
            vertices.bottom_right.textured    = 0.0f;
            vertices.bottom_left.textured     = 0.0f;
            vertices.top_left.textured        = 0.0f;
            ++quad_count;
        }
        return(quad_count);
    }

    // renders one emitter
    // returns the size of the vertex data that was written to the buffer
    IFB_INTERNAL u32
    particle_emitter_render_quads(
        const hnd_particle_emitter h_emitter,
        byte*                      buffer_data,
        const u32                  buffer_size,
        const orientation&         camera_orientation) {

        assert(h_emitter   != INVALID_HANDLE);
        assert(buffer_data != NULL);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(0);

        const u32 quad_count = particle_emitter_render_quad_vertices(
            particle_mngr_get_emitter(index),
            (quad_vertices*)buffer_data,
            buffer_size / sizeof(quad_vertices),
            camera_orientation
        );
        return(quad_count * sizeof(quad_vertices));
    }

    // renders every active emitter
    // returns the size of the vertex data that was written to the buffer
    IFB_INTERNAL u32
    particle_mngr_render_buffer(
        byte*              buffer_data,
        const u32          buffer_size,
        const orientation& camera_orientation) {

        assert(_particle_mngr);
        assert(buffer_data != NULL);
        assert(buffer_size != 0);

        auto*     vertex_buffer = (quad_vertices*)buffer_data;
        const u32 quad_capacity = buffer_size / sizeof(quad_vertices);
        u32       quad_count    = 0;

        for (
            u32 type = 0;
                type < particle_emitter_type_e_count;
              ++type) {

            const particle_emitter_list& list = particle_mngr_get_list(type);
            for (
                u32 list_index = 0;
                    list_index < list.count;
                  ++list_index) {

                quad_count += particle_emitter_render_quad_vertices(
                    particle_mngr_get_emitter(list.array_index[list_index]),
                    &vertex_buffer[quad_count],
                    quad_capacity - quad_count,
                    camera_orientation
                );
            }
        }

        return(quad_count * sizeof(quad_vertices));
    }
};
