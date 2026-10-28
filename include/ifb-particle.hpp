#ifndef IFB_PARTICLE_HPP
#define IFB_PARTICLE_HPP

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "ifb-config.hpp"
#include "ifb-memory.hpp"
#include "sld.hpp"

namespace ifb {
     
    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
    
    enum particle_emitter_type_e {
        particle_emitter_type_e_parabolic = 0,
        particle_emitter_type_e_radial    = 1,
        particle_emitter_type_e_count     = 2
    };

    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------

    struct particle_mngr;
    struct particle_buffer;
    struct particle_vertex;

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

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct particle_buffer {
        u32       count;
        particle* particles;
    };

    struct particle_vertex {
        vec3           position;
        color_rgba_u32 color;
    };
};

#endif //IFB_PARTICLE_HPP
