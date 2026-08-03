#pragma once

#include "quad.hpp"
#include "entity-lookup.cpp"
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

        cmpnt_position_table_lookup (q.pos,   q.index_sparse);
        cmpnt_color_table_lookup    (q.color, q.index_sparse);
        cmpnt_quad_table_lookup     (q.dims,  q.index_sparse);

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

        cmpnt_position_table_update (q.pos,   q.index_sparse);
        cmpnt_color_table_update    (q.color, q.index_sparse);
        cmpnt_quad_table_update     (q.dims,  q.index_sparse);
    }

    IFB_INTERNAL void
    quad_update(
        const entity_id id,
        const quad&     q) {

        assert(id != ENTITY_ID_INVALID);

        const u32 sparse_index = entity_lookup_sparse_index(id);
        assert(sparse_index != INVALID_INDEX);
        
        cmpnt_position_table_update (q.position,   sparse_index);
        cmpnt_color_table_update    (q.color,      sparse_index);
        cmpnt_quad_table_update     (q.dimensions, sparse_index);
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
