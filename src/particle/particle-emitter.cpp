#pragma once

#include "ifb-types.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "particle.hpp"
#include "sld-strings.hpp"
#include "sld.hpp"
#include <cassert>

namespace ifb {


    IFB_INTERNAL hnd_particle_emitter
    particle_emitter_create(
        const cchar*           name,
        const u32              count,
        const f32              lifespan_ms,
        const f32              width,
        const f32              height,
        const color_rgba_u32&  color_primary,
        const color_rgba_u32&  color_secondary) {

        assert(name         != NULL);
        assert(count        != 0);
        assert(lifespan_ms  != 0);
        assert(width        != 0);
        assert(height       != 0);

        // initialize the name string
        cstr_c16 name_cstr;
        cstr_c16_init(&name_cstr, name);
   
        // check for collisions and find the next available handle
        const hnd_particle_emitter h_emitter_new     = cstr_c16_hash(&name_cstr); 
        const u32                  emitter_capacity  = particle_mngr_get_emitter_capacity();
        const u32                  emitter_index_new = particle_mngr_find_emitter_index_new(h_emitter_new);

        // if we didn't find a handle, we're done
        if (emitter_index_new == INVALID_INDEX) {
            return(INVALID_HANDLE);
        }

        // get the emitter
        particle_emitter& emitter = particle_mngr_get_emitter(emitter_index_new);

        // set the emitter properties
        emitter.hnd             = h_emitter_new;
        emitter.count           = count;
        emitter.lifespan_ms     = lifespan_ms;
        emitter.height          = height;
        emitter.width           = width;
        emitter.color_primary   = color_primary;
        emitter.color_secondary = color_secondary;
        emitter.name            = name_cstr; 
  
        // clear the particle arrays
        const auto& cfg = config_instance();
        zero_memory((void*)emitter.array_life_ms,  cfg.particle_bufffer_count * sizeof(f32)); 
        zero_memory((void*)emitter.array_colors,   cfg.particle_bufffer_count * sizeof(color_rgba_u32)); 
        zero_memory((void*)emitter.array_position, cfg.particle_bufffer_count * sizeof(vec3)); 
    
        return(h_emitter_new);
    }

    IFB_INTERNAL void
    particle_emitter_destroy(
        const hnd_particle_emitter h_emitter) {

    }

    IFB_INTERNAL particle_buffer*
    particle_emitter_render_buffer(
        const hnd_particle_emitter h_emitter,
        const hnd_arena            h_arena,
        const vec3&                offset) {

    }
    
    IFB_INTERNAL bool
    particle_emitter_get_info(
        const hnd_particle_emitter h_emitter,
        particle_emitter_info&     info) {

        assert(h_emitter != INVALID_HANDLE);
    
        const u32 index = particle_mngr_find_emitter_index_existing(h_emitter); 
        if (index == INVALID_INDEX) return(false);
    
        const particle_emitter& emitter = particle_mngr_get_emitter(index);
    
        info.name            = emitter.name.chars;
        info.count           = emitter.count;
        info.lifespan_ms     = emitter.lifespan_ms;
        info.height          = emitter.height;
        info.width           = emitter.width;
        info.color_primary   = emitter.color_primary;
        info.color_secondary = emitter.color_secondary;
        return(true);
    }
};
