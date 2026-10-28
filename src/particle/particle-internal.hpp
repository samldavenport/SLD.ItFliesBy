#ifndef PARTICLE_INTERNAL_HPP
#define PARTICLE_INTERNAL_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "ifb-particle.hpp"
#include "sld.hpp"
#include <sld-strings.hpp>

namespace ifb {

    // spawn limit of an emitter that spawns until it is stopped
    constexpr u32 PARTICLE_SPAWN_UNLIMITED = INVALID_U32;

    struct particle_emitter {
        hnd_particle_emitter hnd;
        u32                  type;
        u32                  list_index;      // INVALID_INDEX when idle
        bool                 is_spawning;
        u32                  spawn_index;     // next particle in the ring
        u32                  spawn_limit;     // particles spawned per start
        u32                  spawn_remaining;
        f32                  spawn_timer_ms;
        u32                  count;
        f32                  lifespan_ms;
        f32                  spawn_delay_ms;
        f32                  spawn_radius;    // particles spawn within this distance of the position
        f32                  height;
        f32                  width;
        vec3                 position;
        color_rgba_u32       color_primary;
        color_rgba_u32       color_secondary;
        f32*                 array_life_ms;
        color_rgba_u32*      array_colors;
        vec3*                array_position;
        vec3*                array_velocity;
        cstr_c16             name;
    };  

    // the active emitters of one type
    struct particle_emitter_list {
        u32  count;
        u32* array_index;
    };

    IFB_INTERNAL particle_emitter_list&    particle_mngr_get_list            (const u32 type);
    IFB_INTERNAL particle_parabolic_props& particle_mngr_get_props_parabolic (const u32 index);
    IFB_INTERNAL particle_radial_props&    particle_mngr_get_props_radial    (const u32 index);
    IFB_INTERNAL void                      particle_mngr_activate_emitter    (const u32 index);
    IFB_INTERNAL void                      particle_mngr_deactivate_emitter  (const u32 index);

    IFB_INTERNAL particle_emitter& particle_mngr_get_emitter  (const u32 index);
    IFB_INTERNAL particle_emitter& particle_mngr_set_handle   (const u32 index, const hnd_particle_emitter);
    IFB_INTERNAL u32               particle_mngr_find_emitter_index_existing (const hnd_particle_emitter h_emitter);
    IFB_INTERNAL u32               particle_mngr_find_emitter_index_new      (const hnd_particle_emitter h_emitter);
};

#endif //PARTICLE_INTERNAL_HPP
