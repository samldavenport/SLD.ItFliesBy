#pragma once

#include <cassert>

#include "ifb-types.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "particle.hpp"
#include "quad.hpp"
#include "sld-math-vec3.hpp"
#include "sld.hpp"

namespace ifb {

    // renders the live particles of every active emitter as quads that face the camera
    // returns the size of the vertex data that was written to the buffer
    IFB_INTERNAL u32
    particle_mngr_render_buffer(
        byte*              buffer_data,
        const u32          buffer_size,
        const orientation& camera_orientation) {

        assert(_particle_mngr);
        assert(buffer_data != NULL);
        assert(buffer_size != 0);

        auto*     vertex_buffer  = (quad_vertices*)buffer_data;
        const u32 quad_capacity  = buffer_size / sizeof(quad_vertices);
        u32       quad_count     = 0;

        for (
            u32 type = 0;
                type < particle_emitter_type_e_count;
              ++type) {

            const particle_emitter_list& list = particle_mngr_get_list(type);
            for (
                u32 list_index = 0;
                    list_index < list.count;
                  ++list_index) {

                const particle_emitter& emitter = particle_mngr_get_emitter(list.array_index[list_index]);

                // every particle in the emitter is the same size
                const vec3 right = vec3_scalar_multiply(camera_orientation.right, emitter.width  * 0.5f);
                const vec3 up    = vec3_scalar_multiply(camera_orientation.up,    emitter.height * 0.5f);

                for (
                    u32 index = 0;
                        index < emitter.count;
                      ++index) {

                    if (emitter.array_life_ms[index] <= 0.0f) continue;

                    // if the buffer is full, the rest of the particles aren't rendered
                    if (quad_count == quad_capacity) {
                        return(quad_count * sizeof(quad_vertices));
                    }

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
                    ++quad_count;
                }
            }
        }

        return(quad_count * sizeof(quad_vertices));
    }
};
