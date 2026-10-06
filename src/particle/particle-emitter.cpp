#pragma once

#include "ifb-types.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "particle.hpp"
#include "sld-strings.hpp"

namespace ifb {


    IFB_INTERNAL hnd_particle_emitter
    particle_emitter_create(
        const cchar* name,
        const u32    count,
        const f32    life,
        const f32    width,
        const f32    height,
        const vec3&  color_primary,
        const vec3&  color_secondary) {

        assert(name   != NULL);
        assert(count  != 0);
        assert(life   != 0);
        assert(width  != 0);
        assert(height != 0);

        // initialize the name string
        cstr_c16 name_cstr;
        cstr_c16_init(&name_cstr, name);
   
        // check for collisions and find the next available handle
        const hnd_particle_emitter h_emitter_new     = cstr_c16_hash(&name_cstr); 
        const u32                  emitter_capacity  = particle_mngr_get_emitter_capacity();
        u32                        emitter_index_new = INVALID_INDEX;
        for (
            u32 emitter_index_curr = 0;
                emitter_index_curr < emitter_capacity;
              ++emitter_index_curr) {

            // make sure there's no collision
            const hnd_particle_emitter h_emitter_curr = particle_mngr_get_emitter_hnd(emitter_index_curr);
            assert(h_emitter_curr != h_emitter_new);
        
            // if we found an empty handle, store it
            const bool is_free = (
                emitter_index_new == INVALID_INDEX &&
                h_emitter_curr    == INVALID_HANDLE
            );

            if (is_free) {
                emitter_index_new = emitter_index_curr;
            }
        } 

        // if we didn't find a handle, we're done
        if (emitter_index_new == INVALID_INDEX) {
            return(INVALID_HANDLE);
        }


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
};
