#pragma once

#include "ifb.hpp"
#include "eng-entity.hpp"
#include "eng-component.hpp"
#include "eng-events.hpp"
#include "eng-physics.hpp"

namespace ifb {

    // calls the collider proc of the contact's self, if it has one
    // the entity is looked up again here, a proc that ran before this one may have destroyed it
    inline void
    phys_cntct_call_proc(
        const phys_cllsn_contact& contact) {

        entity e;
        if (!entity_lookup_by_id(e, contact.self))               return;
        if (!e.archetype.has_all(cmpnt_type_e_collider))         return;

        cmpnt_collider clldr;
        cmpnt_lookup_collider(e.index_sparse, clldr);
        if (clldr.proc == NULL) return;

        clldr.proc(contact);
    }

    // runs after the physics step, once nothing in physics is mid-loop
    // so a proc can safely destroy entities, or add forces for the next step
    IFB_INTERNAL void
    phys_cntct_dispatch(
        const phys_cntct&          cntct,
        const phys_cllsn_phase_u32 phase) {

        // the global event, the listeners run after the physics step
        event_payload payload;
        payload.data.physics_collision.a      = cntct.a;
        payload.data.physics_collision.b      = cntct.b;
        payload.data.physics_collision.normal = cntct.normal;
        payload.data.physics_collision.depth  = cntct.depth;
        payload.data.physics_collision.phase  = phase;

        const bool did_publish = event_mngr_publish(event_type_e_physics_collision, payload);
        assert(did_publish);
        (void)did_publish;

        // a's proc, the normal already points from a to b
        phys_cllsn_contact contact;
        contact.self   = cntct.a;
        contact.other  = cntct.b;
        contact.normal = cntct.normal;
        contact.depth  = cntct.depth;
        contact.phase  = phase;
        phys_cntct_call_proc(contact);

        // b's proc, the normal is flipped so it points from b to a
        contact.self     = cntct.b;
        contact.other    = cntct.a;
        contact.normal.x = -cntct.normal.x;
        contact.normal.y = -cntct.normal.y;
        contact.normal.z = -cntct.normal.z;
        phys_cntct_call_proc(contact);
    }
};
