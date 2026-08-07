#pragma once

#include "quad.hpp"
#include "entity.cpp"
#include "ifb-entity.hpp"
#include "component.hpp"
#include "ifb-types.hpp"

namespace ifb {

    IFB_INTERNAL bool 
    quad_lookup_by_id(
        quad_entity&    q,
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);


        entity e;
        const bool found_entity = (
            entity_lookup_by_id(e, id) &&
            e.archetype & ENTITY_ARCHETYPE_QUAD == ENTITY_ARCHETYPE_QUAD 
        );

        if (!found_entity) {
            return(false);
        }

        q.tag          = e.tag;
        q.id           = e.id;
        q.archetype    = e.archetype;
        q.index_sparse = e.index_sparse;
        q.index_dense  = e.index_dense;

        quad q_;
        cmpnt_lookup_position (q.index_sparse, q.pos);
        cmpnt_lookup_color    (q.index_sparse, q.color);
        cmpnt_lookup_quad     (q.index_sparse, q_);

        q.dims.width  = q_.width;
        q.dims.height = q_.height;

        return(true);
    }

    IFB_INTERNAL void
    quad_update(
	const quad_entity& q) {

        assert(
            (q.archetype & ENTITY_ARCHETYPE_QUAD) == ENTITY_ARCHETYPE_QUAD &&
            q.id                                  != ENTITY_ID_INVALID     &&
            q.tag                                 != NULL 
        );

        quad q_;
        q_.width  = q.dims.width;
        q_.height = q.dims.height;
        cmpnt_update_position (q.index_sparse, q.pos);
        cmpnt_update_color    (q.index_sparse, q.color);
        cmpnt_update_quad     (q.index_sparse, q_);
    }

    IFB_INTERNAL void
    quad_update(
        const entity_id id,
        const quad&     q) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);
        
        cmpnt_update_quad     (sparse_index, q);
    }

    IFB_INTERNAL bool
    quad_does_exist(
        const entity_id id) {

        entity e;
        const bool does_exist = (
            entity_lookup_by_id(e, id) &&
            e.archetype & ENTITY_ARCHETYPE_QUAD == ENTITY_ARCHETYPE_QUAD 
        );

        return(does_exist);
    }
};
