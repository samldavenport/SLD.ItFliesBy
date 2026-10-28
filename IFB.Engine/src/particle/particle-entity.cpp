#pragma once

#include <cassert>

#include "component.hpp"
#include "ifb-entities.hpp"
#include "ifb-types.hpp"
#include "particle-internal.hpp"
#include "particle-mngr.cpp"
#include "ifb-particle.hpp"
#include "sld.hpp"

namespace ifb {

    // moves the emitter of every entity that has one to where the entity is
    // only new particles spawn there, so the live ones trail behind a moving entity
    IFB_INTERNAL void
    particle_mngr_follow_entities(
        void) {

        assert(_particle_mngr);

        static const component_type cmpnt_types = (
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

            // the component has no emitter until one is set
            cmpnt_particle_emitter emitter;
            cmpnt_lookup_particle_emitter(e.index_sparse, emitter);
            if (emitter.hnd == INVALID_HANDLE) continue;

            cmpnt_position position;
            cmpnt_lookup_position(e.index_sparse, position);
            particle_emitter_set_position(emitter.hnd, position);
        }
    }
};
