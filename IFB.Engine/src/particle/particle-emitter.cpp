#pragma once

#include "ifb.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
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
        const f32              spawn_radius,
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
        assert(spawn_radius   >= 0);
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
        emitter.spawn_radius    = spawn_radius;
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
        const f32                       spawn_radius,
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
            spawn_radius,
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
        const f32                       spawn_radius,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_radial_props&    props) {

        assert(props.speed > 0.0f);

        // explosions spawn every particle once per start, unless they are continuous
        const u32 spawn_limit   = props.is_continuous ? PARTICLE_SPAWN_UNLIMITED : count;
        const u32 emitter_index = particle_emitter_create_common(
            name,
            count,
            lifespan_ms,
            spawn_delay_ms,
            spawn_radius,
            width,
            height,
            color_primary,
            color_secondary,
            particle_emitter_type_e_radial,
            spawn_limit
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
        info.spawn_radius    = emitter.spawn_radius;
        info.height          = emitter.height;
        info.width           = emitter.width;
        info.color_primary   = emitter.color_primary;
        info.color_secondary = emitter.color_secondary;
        return(true);
    }

    // applies the properties in the info that can change while the emitter is running
    // the name, type, and state are ignored
    IFB_INTERNAL bool
    particle_emitter_update(
        const hnd_particle_emitter   h_emitter,
        const particle_emitter_info& info) {

        const auto& cfg = config_instance();

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);

        // nothing changes unless everything is valid
        const bool is_valid = (
            info.count          != 0                          &&
            info.count          <= cfg.particle_bufffer_count &&
            info.lifespan_ms    >  0.0f                       &&
            info.spawn_delay_ms >= 0.0f                       &&
            info.spawn_radius   >= 0.0f                       &&
            info.width          >  0.0f                       &&
            info.height         >  0.0f
        );
        if (!is_valid) return(false);

        particle_emitter& emitter = particle_mngr_get_emitter(index);

        // the ring and the live particles are only good for the old count,
        // so a new count starts the emitter over with no particles
        if (info.count != emitter.count) {

            zero_memory((void*)emitter.array_life_ms, cfg.particle_bufffer_count * sizeof(f32));
            emitter.count       = info.count;
            emitter.spawn_index = 0;

            // an emitter with a limit spawns every particle once per start
            if (emitter.spawn_limit != PARTICLE_SPAWN_UNLIMITED) {
                emitter.spawn_limit     = info.count;
                emitter.spawn_remaining = emitter.is_spawning ? info.count : 0;
            }
        }

        // no particle can have more life left than the new lifespan
        if (info.lifespan_ms < emitter.lifespan_ms) {
            for (
                u32 particle_index = 0;
                    particle_index < emitter.count;
                  ++particle_index) {

                f32& life_ms = emitter.array_life_ms[particle_index];
                if (life_ms > info.lifespan_ms) life_ms = info.lifespan_ms;
            }
        }

        emitter.lifespan_ms     = info.lifespan_ms;
        emitter.spawn_delay_ms  = info.spawn_delay_ms;
        emitter.spawn_radius    = info.spawn_radius;

        // the time saved up for a longer delay would spawn a burst with a shorter one
        if (emitter.spawn_timer_ms > emitter.spawn_delay_ms) {
            emitter.spawn_timer_ms = emitter.spawn_delay_ms;
        }
        emitter.height          = info.height;
        emitter.width           = info.width;
        emitter.position        = info.position;
        emitter.color_primary   = info.color_primary;
        emitter.color_secondary = info.color_secondary;
        return(true);
    }

    IFB_INTERNAL bool
    particle_emitter_get_props_parabolic(
        const hnd_particle_emitter h_emitter,
        particle_parabolic_props&  props) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);
        if (particle_mngr_get_emitter(index).type != particle_emitter_type_e_parabolic) return(false);

        props = particle_mngr_get_props_parabolic(index);
        return(true);
    }

    IFB_INTERNAL bool
    particle_emitter_set_props_parabolic(
        const hnd_particle_emitter      h_emitter,
        const particle_parabolic_props& props) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);
        if (particle_mngr_get_emitter(index).type != particle_emitter_type_e_parabolic) return(false);

        // the launch direction comes from the velocity, so it can't be zero
        const bool is_valid = (
            vec3_magnitude(props.velocity) >  0.0f &&
            props.spread_degrees           >= 0.0f &&
            props.spread_degrees           <= 180.0f
        );
        if (!is_valid) return(false);

        particle_mngr_get_props_parabolic(index) = props;
        return(true);
    }

    IFB_INTERNAL bool
    particle_emitter_get_props_radial(
        const hnd_particle_emitter h_emitter,
        particle_radial_props&     props) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);
        if (particle_mngr_get_emitter(index).type != particle_emitter_type_e_radial) return(false);

        props = particle_mngr_get_props_radial(index);
        return(true);
    }

    IFB_INTERNAL bool
    particle_emitter_set_props_radial(
        const hnd_particle_emitter   h_emitter,
        const particle_radial_props& props) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);
        if (particle_mngr_get_emitter(index).type != particle_emitter_type_e_radial) return(false);

        if (props.speed <= 0.0f) return(false);

        // a spawning emitter picks up the new limit right away,
        // an emitter that is draining or idle gets it on the next start
        particle_emitter& emitter = particle_mngr_get_emitter(index);
        emitter.spawn_limit = props.is_continuous ? PARTICLE_SPAWN_UNLIMITED : emitter.count;
        if (emitter.is_spawning) {
            emitter.spawn_remaining = emitter.spawn_limit;
        }

        particle_mngr_get_props_radial(index) = props;
        return(true);
    }

    // moves where new particles spawn, the live particles stay on their paths
    IFB_INTERNAL bool
    particle_emitter_set_position(
        const hnd_particle_emitter h_emitter,
        const vec3&                position) {

        assert(h_emitter != INVALID_HANDLE);

        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter);
        if (index == INVALID_INDEX) return(false);

        particle_mngr_get_emitter(index).position = position;
        return(true);
    }
};
