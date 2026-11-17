#pragma once

#include <cassert>
#include "eng-entity.hpp"

#include "eng-component.hpp"
#include "ifb-profile.hpp"
#include "ifb.hpp"
#include "eng-particle.hpp"
#include "particle-mngr.cpp"
#include "particle-render.cpp"
#include "sld.hpp"

namespace ifb {

    // moves the emitter of every entity that has one to where the entity is
    // only new particles spawn there, so the live ones trail behind a moving entity
    IFB_INTERNAL void
    particle_mngr_follow_entities(
        void) {

        IFB_PROFILE_FUNCTION();

        assert(_particle_mngr);

        static const cmpnt_type_u32 cmpnt_types = (
            cmpnt_type_e_position |
            cmpnt_type_e_particle_emitter
        );

        const u32 entity_count = entity_mngr_get_count();
        for (
            u32 entity_index = 0;
                entity_index < entity_count;
              ++entity_index) {

            entity e;
            if (!entity_lookup_by_index_dense(e, entity_index)) continue;
            if (!e.archetype.has_all(cmpnt_types))              continue;
            if (!entity_is_updating(e))                         continue;

            // the component has no emitter until one is set
            cmpnt_particle_emitter emitter;
            cmpnt_lookup_particle_emitter(e.index_sparse, emitter);
            if (emitter.hnd == INVALID_HANDLE) continue;

            cmpnt_position position;
            cmpnt_lookup_position(e.index_sparse, position);
            particle_emitter_set_position(emitter.hnd, position);
        }
    }

    // renders the particles of every entity marked to be drawn this frame
    // returns the size of the vertex data that was written to the buffer
    // if the buffer is full, the rest of the particles aren't rendered
    IFB_INTERNAL u32
    particle_mngr_render_entities(
        byte*              buffer_data,
        const u32          buffer_size,
        const orientation& camera_orientation) {

        IFB_PROFILE_FUNCTION();

        assert(_particle_mngr);
        assert(buffer_data != NULL);

        static const cmpnt_type_u32 cmpnt_types = (
            cmpnt_type_e_position |
            cmpnt_type_e_particle_emitter
        );

        u32       size         = 0;
        const u32 entity_count = entity_mngr_get_count();
        for (
            u32 entity_index = 0;
                entity_index < entity_count && size < buffer_size;
              ++entity_index) {

            entity e;
            if (!entity_lookup_by_index_dense(e, entity_index)) continue;
            if (!e.archetype.has_all(cmpnt_types))              continue;
            if (!entity_is_drawn(e))                            continue;

            cmpnt_particle_emitter emitter;
            cmpnt_lookup_particle_emitter(e.index_sparse, emitter);
            if (emitter.hnd == INVALID_HANDLE) continue;

            size += particle_emitter_render_quads(
                emitter.hnd,
                &buffer_data[size],
                buffer_size - size,
                camera_orientation
            );
        }
        return(size);
    }
};
