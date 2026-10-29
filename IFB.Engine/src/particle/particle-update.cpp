#pragma once

#include <cassert>
#include <cmath>
#include "random-internal.hpp"

#include "ifb.hpp"
#include "particle-emitter.cpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "ifb-particle.hpp"
#include "random-table.cpp"
#include "sld-math-globals.hpp"
#include "sld-math-vec3.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // t is 0.0 at the primary color and 1.0 at the secondary
    inline color_rgba_u32
    particle_color_blend(
        const color_rgba_u32& color_primary,
        const color_rgba_u32& color_secondary,
        const f32             t) {

        color_rgba_u32 color;
        color.r = (byte)((f32)color_primary.r + (((f32)color_secondary.r - (f32)color_primary.r) * t));
        color.g = (byte)((f32)color_primary.g + (((f32)color_secondary.g - (f32)color_primary.g) * t));
        color.b = (byte)((f32)color_primary.b + (((f32)color_secondary.b - (f32)color_primary.b) * t));
        color.a = (byte)((f32)color_primary.a + (((f32)color_secondary.a - (f32)color_primary.a) * t));
        return(color);
    }

    // ages the live particles and blends their colors
    // returns the count of particles that are still alive
    inline u32
    particle_emitter_age(
        particle_emitter& emitter,
        const f32         dt_ms) {

        u32 count_live = 0;
        for (
            u32 index = 0;
                index < emitter.count;
              ++index) {

            // a particle with no life left is dead
            f32& life_ms = emitter.array_life_ms[index];
            if (life_ms <= 0.0f) continue;

            life_ms -= dt_ms;
            if (life_ms <= 0.0f) {
                life_ms = 0.0f;
                continue;
            }

            const f32 t = 1.0f - (life_ms / emitter.lifespan_ms);
            emitter.array_colors[index] = particle_color_blend(
                emitter.color_primary,
                emitter.color_secondary,
                t
            );
            ++count_live;
        }
        return(count_live);
    }

    // a random point within the spawn radius of the emitter position
    // the cube root spreads the points evenly through the sphere, instead of bunching in the middle
    inline vec3
    particle_emitter_spawn_position(
        const particle_emitter& emitter) {

        if (emitter.spawn_radius <= 0.0f) return(emitter.position);

        const f32 height   = (random_f32() * 2.0f) - 1.0f;
        const f32 ring     = sqrtf(1.0f - (height * height));
        const f32 rotation = random_f32() * 2.0f * PI_32;
        const f32 distance = emitter.spawn_radius * cbrtf(random_f32());

        const vec3 offset = {
            cosf(rotation) * ring * distance,
            height                * distance,
            sinf(rotation) * ring * distance
        };
        return(vec3_add(emitter.position, offset));
    }

    // spawns the next particle in the ring within the spawn radius, if one is due
    // returns the particle index so the caller can set the velocity, or INVALID_INDEX
    inline u32
    particle_emitter_spawn_next(
        particle_emitter& emitter) {

        if (!emitter.is_spawning) return(INVALID_INDEX);

        // the emitter is done once it has spawned its limit
        if (emitter.spawn_remaining == 0) {
            emitter.is_spawning = false;
            return(INVALID_INDEX);
        }

        if (emitter.spawn_timer_ms < emitter.spawn_delay_ms) return(INVALID_INDEX);

        // every particle has the same lifespan, so they die in the order they spawn
        // if the next one in the ring is alive, they all are, and we wait
        const u32 index = emitter.spawn_index;
        if (emitter.array_life_ms[index] > 0.0f) {
            emitter.spawn_timer_ms = emitter.spawn_delay_ms;
            return(INVALID_INDEX);
        }

        emitter.array_life_ms  [index] = emitter.lifespan_ms;
        emitter.array_colors   [index] = emitter.color_primary;
        emitter.array_position [index] = particle_emitter_spawn_position(emitter);

        emitter.spawn_index     = (index + 1) % emitter.count;
        emitter.spawn_timer_ms -= emitter.spawn_delay_ms;
        if (emitter.spawn_remaining != PARTICLE_SPAWN_UNLIMITED) {
            --emitter.spawn_remaining;
        }
        return(index);
    }

    // a random direction within a cone around the axis
    // the cosine of the cone's half angle is 1.0 for a line and -1.0 for a full sphere
    inline vec3
    particle_random_direction(
        const vec3& axis,
        const vec3& tangent,
        const vec3& bitangent,
        const f32   cone_cos) {

        const f32 angle_cos = 1.0f - (random_f32() * (1.0f - cone_cos));
        const f32 angle_sin = sqrtf(1.0f - (angle_cos * angle_cos));
        const f32 rotation  = random_f32() * 2.0f * PI_32;

        const vec3 radial = vec3_add(
            vec3_scalar_multiply(tangent,   cosf(rotation) * angle_sin),
            vec3_scalar_multiply(bitangent, sinf(rotation) * angle_sin)
        );
        const vec3 direction = vec3_add(
            vec3_scalar_multiply(axis, angle_cos),
            radial
        );
        return(direction);
    }

    // fountains
    inline void
    particle_mngr_update_emitters_parabolic(
        const f32 dt_ms) {

        const f32              dt_s = dt_ms * 0.001f;
        particle_emitter_list& list = particle_mngr_get_list(particle_emitter_type_e_parabolic);

        for (u32 list_index = 0; list_index < list.count;) {

            const u32                       emitter_index = list.array_index[list_index];
            particle_emitter&               emitter       = particle_mngr_get_emitter(emitter_index);
            const particle_parabolic_props& props         = particle_mngr_get_props_parabolic(emitter_index);

            u32 count_live = particle_emitter_age(emitter, dt_ms);

            // gravity bends the path of every live particle
            const vec3 velocity_delta = vec3_scalar_multiply(props.gravity, dt_s);
            for (
                u32 index = 0;
                    index < emitter.count;
                  ++index) {

                if (emitter.array_life_ms[index] <= 0.0f) continue;

                vec3& velocity = emitter.array_velocity [index];
                vec3& position = emitter.array_position [index];
                velocity = vec3_add(velocity, velocity_delta);
                position = vec3_add(position, vec3_scalar_multiply(velocity, dt_s));
            }

            // new particles launch within a cone around the launch velocity
            const f32  speed     = vec3_magnitude(props.velocity);
            const vec3 axis      = vec3_scalar_divide(props.velocity, speed);
            const vec3 reference = (fabsf(axis.x) < 0.9f) ? vec3{1.0f, 0.0f, 0.0f} : vec3{0.0f, 1.0f, 0.0f};
            const vec3 tangent   = vec3_normalize(vec3_cross(axis, reference));
            const vec3 bitangent = vec3_cross(axis, tangent);
            const f32  cone_cos  = cosf(props.spread_degrees * RADIANS_FACTOR);

            emitter.spawn_timer_ms += dt_ms;
            for (
                u32 index = particle_emitter_spawn_next(emitter);
                    index != INVALID_INDEX;
                    index = particle_emitter_spawn_next(emitter)) {

                const vec3 direction = particle_random_direction(axis, tangent, bitangent, cone_cos);
                emitter.array_velocity[index] = vec3_scalar_multiply(direction, speed);
                ++count_live;
            }

            // a stopped emitter goes idle when its last particle dies
            // the last emitter in the list takes this slot, so we process the slot again
            if (!emitter.is_spawning && count_live == 0) {
                particle_mngr_deactivate_emitter(emitter_index);
                continue;
            }
            ++list_index;
        }
    }

    // explosions
    inline void
    particle_mngr_update_emitters_radial(
        const f32 dt_ms) {

        const f32              dt_s = dt_ms * 0.001f;
        particle_emitter_list& list = particle_mngr_get_list(particle_emitter_type_e_radial);

        // new particles launch in any direction
        const vec3 axis      = {0.0f, 1.0f, 0.0f};
        const vec3 tangent   = {1.0f, 0.0f, 0.0f};
        const vec3 bitangent = {0.0f, 0.0f, 1.0f};
        const f32  cone_cos  = -1.0f;

        for (u32 list_index = 0; list_index < list.count;) {

            const u32                    emitter_index = list.array_index[list_index];
            particle_emitter&            emitter       = particle_mngr_get_emitter(emitter_index);
            const particle_radial_props& props         = particle_mngr_get_props_radial(emitter_index);

            u32 count_live = particle_emitter_age(emitter, dt_ms);

            // every live particle moves in a straight line
            for (
                u32 index = 0;
                    index < emitter.count;
                  ++index) {

                if (emitter.array_life_ms[index] <= 0.0f) continue;

                vec3& position = emitter.array_position[index];
                position = vec3_add(position, vec3_scalar_multiply(emitter.array_velocity[index], dt_s));
            }

            emitter.spawn_timer_ms += dt_ms;
            for (
                u32 index = particle_emitter_spawn_next(emitter);
                    index != INVALID_INDEX;
                    index = particle_emitter_spawn_next(emitter)) {

                // a particle that spawned away from the center keeps going that way,
                // one that spawned on the center has no way out, so it gets a random one
                const vec3 offset    = vec3_subtract(emitter.array_position[index], emitter.position);
                const f32  distance  = vec3_magnitude(offset);
                const vec3 direction = (distance > 0.0f)
                    ? vec3_scalar_divide(offset, distance)
                    : particle_random_direction(axis, tangent, bitangent, cone_cos);

                emitter.array_velocity[index] = vec3_scalar_multiply(direction, props.speed);
                ++count_live;
            }

            // a finished emitter goes idle when its last particle dies
            // the last emitter in the list takes this slot, so we process the slot again
            if (!emitter.is_spawning && count_live == 0) {
                particle_mngr_deactivate_emitter(emitter_index);
                continue;
            }
            ++list_index;
        }
    }

    IFB_INTERNAL void
    particle_mngr_update_emitters(
        const f32 dt_ms) {

        assert(_particle_mngr);
        assert(dt_ms >= 0.0f);

        // each list only has the active emitters of one type
        particle_mngr_update_emitters_parabolic (dt_ms);
        particle_mngr_update_emitters_radial    (dt_ms);
    }
};
