#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "ifb-config.hpp"
#include "memory.hpp"

namespace ifb {
     
    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------

    struct particle_mngr;
    struct particle_buffer;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    // particle manager
    IFB_INTERNAL particle_mngr*       particle_mngr_create               (void);
    IFB_INTERNAL void                 particle_mngr_startup              (reservation* res); 
    IFB_INTERNAL void                 particle_mngr_update_emitters      (const u32 dt_ms);
    IFB_INTERNAL u32                  particle_mngr_get_emitter_capacity (void);
    IFB_INTERNAL hnd_particle_emitter particle_mngr_get_emitter_hnd      (const u32 index);

    // particle emitter
    IFB_INTERNAL hnd_particle_emitter
    particle_emitter_create(
        const cchar*           name,
        const u32              count,
        const f32              lifespan_ms,
        const f32              width,
        const f32              height,
        const color_rgba_u32&  color_primary,
        const color_rgba_u32&  color_secondary
    );
    IFB_INTERNAL bool             particle_emitter_destroy       (const hnd_particle_emitter h_emitter);
    IFB_INTERNAL particle_buffer* particle_emitter_render_buffer (const hnd_particle_emitter h_emitter, const hnd_arena h_arena, const vec3& offset);
    IFB_INTERNAL bool             particle_emitter_get_info      (const hnd_particle_emitter h_emitter, particle_emitter_info& info);

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct particle_buffer {
        u32       count;
        particle* particles;
    };
};

#endif //PARTICLE_HPP
