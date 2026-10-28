#ifndef PARTICLE_INTERNAL_HPP
#define PARTICLE_INTERNAL_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "memory-reservation.cpp"
#include "ifb-particle.hpp"
#include "sld.hpp"
#include <sld-strings.hpp>

namespace ifb {

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // particle manager
    IFB_INTERNAL particle_mngr*       particle_mngr_create               (void);
    IFB_INTERNAL void                 particle_mngr_startup              (reservation* res); 
    IFB_INTERNAL void                 particle_mngr_update_emitters      (const f32 dt_ms);
    IFB_INTERNAL u32                  particle_mngr_get_emitter_capacity (void);
    IFB_INTERNAL hnd_particle_emitter particle_mngr_get_emitter_hnd      (const u32 index);
    IFB_INTERNAL u32                  particle_mngr_render_buffer        (byte* buffer_data, const u32 buffer_size, const orientation& camera_orientation);
    IFB_INTERNAL void                 particle_mngr_follow_entities      (void);

    // particle emitter
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
        const particle_parabolic_props& props
    );
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
        const particle_radial_props&    props
    );
    IFB_INTERNAL bool             particle_emitter_destroy       (const hnd_particle_emitter h_emitter);
    IFB_INTERNAL bool             particle_emitter_start         (const hnd_particle_emitter h_emitter, const vec3& position);
    IFB_INTERNAL bool             particle_emitter_stop          (const hnd_particle_emitter h_emitter);
    IFB_INTERNAL particle_buffer* particle_emitter_render_buffer (const hnd_particle_emitter h_emitter, const hnd_arena h_arena, const vec3& offset);
    IFB_INTERNAL bool             particle_emitter_get_info      (const hnd_particle_emitter h_emitter, particle_emitter_info& info);
    IFB_INTERNAL bool             particle_emitter_set_position  (const hnd_particle_emitter h_emitter, const vec3& position);
    IFB_INTERNAL u32              particle_emitter_render_quads  (const hnd_particle_emitter h_emitter, byte* buffer_data, const u32 buffer_size, const orientation& camera_orientation);
    IFB_INTERNAL bool             particle_emitter_update        (const hnd_particle_emitter h_emitter, const particle_emitter_info& info);
    IFB_INTERNAL bool             particle_emitter_get_props_parabolic (const hnd_particle_emitter h_emitter, particle_parabolic_props& props);
    IFB_INTERNAL bool             particle_emitter_set_props_parabolic (const hnd_particle_emitter h_emitter, const particle_parabolic_props& props);
    IFB_INTERNAL bool             particle_emitter_get_props_radial    (const hnd_particle_emitter h_emitter, particle_radial_props& props);
    IFB_INTERNAL bool             particle_emitter_set_props_radial    (const hnd_particle_emitter h_emitter, const particle_radial_props& props);

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
