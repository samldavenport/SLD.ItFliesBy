#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "particle.hpp"

namespace ifb {
  
    IFB_ENGINE_API hnd_particle_emitter
    eng_particle_emitter_create(
        const cchar*           name,
        const u32              count,
        const f32              lifespan_ms,
        const f32              width,
        const f32              height,
        const color_rgba_u32&  color_primary,
        const color_rgba_u32&  color_secondary) {

        assert(name        != NULL);
        assert(count       > 0);
        assert(lifespan_ms > 0);
        assert(width       > 0);
        assert(height      > 0);

        const hnd_particle_emitter h_emitter = particle_emitter_create(
            name,
            count,
            lifespan_ms,
            width,
            height,
            color_primary,
            color_secondary
        );
    
        return(h_emitter);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_destroy(
        const hnd_particle_emitter h_emitter) {

        assert(h_emitter != INVALID_HANDLE);
    
        const bool did_destroy = particle_emitter_destroy(h_emitter);
        return(did_destroy);
    }

    IFB_ENGINE_API bool
    eng_particle_emitter_render(
        const hnd_particle_emitter h_emitter) {

        //TODO(SLD): implement
        return(false);
    }
};
