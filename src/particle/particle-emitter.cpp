#pragma once

#include "ifb-types.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "particle.hpp"
#include "sld-strings.hpp"
#include "sld-math-vec3.hpp"
#include "sld.hpp"
#include <cassert>

namespace ifb {


    // claims a slot and sets everything the emitter types have in common
    // returns the index of the new emitter, or INVALID_INDEX if there is no slot
    inline u32
    particle_emitter_create_common(
        const cchar*           name,
        const u32              count,
        const f32              lifespan_ms,
        const f32              spawn_delay_ms,
        const f32              width,
        const f32              height,
        const color_rgba_u32&  color_primary,
        const color_rgba_u32&  color_secondary,
        const u32              type,
        const u32              spawn_limit) {

        const auto& cfg = config_instance();

        assert(name           != NULL);
        assert(count          != 0);
        assert(count          <= cfg.particle_bufffer_count);
        assert(lifespan_ms    >  0);
        assert(spawn_delay_ms >= 0);
        assert(width          >  0);
        assert(height         >  0);
        assert(type           <  particle_emitter_type_e_count);

        // initialize the name string
        cstr_c16 name_cstr;
        cstr_c16_init(&name_cstr, name);

        // check for collisions and find the next available handle
        const hnd_particle_emitter h_emitter_new     = cstr_c16_hash(&name_cstr);
        const u32                  emitter_index_new = particle_mngr_find_emitter_index_new(h_emitter_new);

        // if we didn't find a handle, we're done
        if (emitter_index_new == INVALID_INDEX) {
            return(INVALID_INDEX);
        }

        // claim the slot and get the emitter
        particle_emitter& emitter = particle_mngr_set_handle(emitter_index_new, h_emitter_new);

        // set the emitter properties
        emitter.type            = type;
        emitter.count           = count;
        emitter.lifespan_ms     = lifespan_ms;
        emitter.spawn_delay_ms  = spawn_delay_ms;
        emitter.height          = height;
        emitter.width           = width;
        emitter.color_primary   = color_primary;
        emitter.color_secondary = color_secondary;
        emitter.name            = name_cstr;

        // the emitter is idle until it is started
        emitter.list_index      = INVALID_INDEX;
        emitter.is_spawning     = false;
        emitter.spawn_index     = 0;
        emitter.spawn_limit     = spawn_limit;
        emitter.spawn_remaining = 0;
        emitter.spawn_timer_ms  = 0.0f;
        emitter.position        = {0.0f, 0.0f, 0.0f};

        // clear the particle arrays
        zero_memory((void*)emitter.array_life_ms,  cfg.particle_bufffer_count * sizeof(f32));
        zero_memory((void*)emitter.array_colors,   cfg.particle_bufffer_count * sizeof(color_rgba_u32));
        zero_memory((void*)emitter.array_position, cfg.particle_bufffer_count * sizeof(vec3));
        zero_memory((void*)emitter.array_velocity, cfg.particle_bufffer_count * sizeof(vec3));

        return(emitter_index_new);
    }

    IFB_INTERNAL hnd_particle_emitter
    particle_emitter_create_parabolic(
        const cchar*                    name,
        const u32                       count,
        const f32                       lifespan_ms,
        const f32                       spawn_delay_ms,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_parabolic_props& props) {

        assert(vec3_magnitude(props.velocity) >  0.0f);
        assert(props.spread_degrees           >= 0.0f);
        assert(props.spread_degrees           <= 180.0f);

        // fountains spawn until they are stopped
        const u32 emitter_index = particle_emitter_create_common(
            name,
            count,
            lifespan_ms,
            spawn_delay_ms,
            width,
            height,
            color_primary,
            color_secondary,
            particle_emitter_type_e_parabolic,
            PARTICLE_SPAWN_UNLIMITED
        );
        if (emitter_index == INVALID_INDEX) return(INVALID_HANDLE);

        particle_mngr_get_props_parabolic(emitter_index) = props;

        return(particle_mngr_get_emitter_hnd(emitter_index));
    }

    IFB_INTERNAL hnd_particle_emitter
    particle_emitter_create_radial(
        const cchar*                    name,
        const u32                       count,
        const f32                       lifespan_ms,
        const f32                       spawn_delay_ms,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_radial_props&    props) {

        assert(props.speed > 0.0f);

        // explosions spawn every particle once per start
        const u32 emitter_index = particle_emitter_create_common(
            name,
            count,
            lifespan_ms,
            spawn_delay_ms,
            width,
            height,
            color_primary,
            color_secondary,
            particle_emitter_type_e_radial,
            count
        );
        if (emitter_index == INVALID_INDEX) return(INVALID_HANDLE);

        particle_mngr_get_props_radial(emitter_index) = props;

        return(particle_mngr_get_emitter_hnd(emitter_index));
    }

    IFB_INTERNAL bool
    particle_emitter_destroy(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);

        particle_mngr_deactivate_emitter(index);
        particle_mngr_set_handle(index, INVALID_HANDLE);
        return(true);
    }

    IFB_INTERNAL bool
    particle_emitter_start(
        const hnd_particle_emitter h_emitter,
        const vec3&                position) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);

        // the first particle spawns on the next update
        particle_emitter& emitter = particle_mngr_get_emitter(index);
        emitter.position        = position;
        emitter.is_spawning     = true;
        emitter.spawn_remaining = emitter.spawn_limit;
        emitter.spawn_timer_ms  = emitter.spawn_delay_ms;

        particle_mngr_activate_emitter(index);
        return(true);
    }

    IFB_INTERNAL bool
    particle_emitter_stop(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);

        // the live particles finish, then the update makes the emitter idle
        particle_emitter& emitter = particle_mngr_get_emitter(index);
        emitter.is_spawning = false;
        return(true);
    }

    IFB_INTERNAL particle_buffer*
    particle_emitter_render_buffer(
        const hnd_particle_emitter h_emitter,
        const hnd_arena            h_arena,
        const vec3&                offset) {

        //TODO(SLD): implement
        return(NULL);
    }
    
    IFB_INTERNAL bool
    particle_emitter_get_info(
        const hnd_particle_emitter h_emitter,
        particle_emitter_info&     info) {

        assert(h_emitter != INVALID_HANDLE);
    
        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter); 
        if (index == INVALID_INDEX) return(false);
    
        const particle_emitter& emitter = particle_mngr_get_emitter(index);
    
        // count the particles that are still alive
        u32 count_live = 0;
        for (
            u32 index = 0;
                index < emitter.count;
              ++index) {

            if (emitter.array_life_ms[index] > 0.0f) ++count_live;
        }

        info.name            = emitter.name.chars;
        info.type            = emitter.type;
        info.is_active       = (emitter.list_index != INVALID_INDEX);
        info.is_spawning     = emitter.is_spawning;
        info.count_live      = count_live;
        info.position        = emitter.position;
        info.count           = emitter.count;
        info.lifespan_ms     = emitter.lifespan_ms;
        info.spawn_delay_ms  = emitter.spawn_delay_ms;
        info.height          = emitter.height;
        info.width           = emitter.width;
        info.color_primary   = emitter.color_primary;
        info.color_secondary = emitter.color_secondary;
        return(true);
    }
};
