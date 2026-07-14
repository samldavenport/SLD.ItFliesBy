#pragma once

#include "quad.hpp"

namespace ifb {

    IFB_INTERNAL entity_id
    quad_create(
        const cchar* tag_cstr) {

        assert(tag_cstr != NULL);

        const entity_id id = entity_create(tag_cstr, QUAD_ENTITY_ARCHETYPE);
        assert(id != ENTITY_ID_INVALID);

        const bool did_add = quad_list_add(&_quad_mngr->all, &id, 1);
        if (!did_add) {
            entity_destroy(tag_cstr);
            return(ENTITY_ID_INVALID);
        }

        return(id);
    }

    IFB_INTERNAL void
    quad_create_batch(
        const cchar** tag_cstr,
        const u32     count,
        entity_id*    id) {

        assert(
            tag_cstr != NULL &&
            count    != 0    &&
            id       != NULL          
        );

        (void)memset((void*)id, 0xFF, sizeof(entity_id) * count);

        for (
            u32 index = 0;
                index < count;
              ++index
        ) {
            entity_id&   curr_id  = id       [index];
            const cchar* curr_tag = tag_cstr [index]; 
            assert(curr_tag != NULL);

            curr_id = entity_create(curr_tag, QUAD_ENTITY_ARCHETYPE);
            assert(curr_id != ENTITY_ID_INVALID);

            const bool did_add = quad_list_add(&_quad_mngr->all, &curr_id, 1);
            if (!did_add) {
                curr_id = ENTITY_ID_INVALID;
                break;
            }
        }
    }

    IFB_INTERNAL bool
    quad_lookup_by_tag(
        quad_entity& q,
        const cchar* tag_cstr) {

        const entity_id id = entity_tag_cstr_to_id(tag_cstr);
        bool did_find = false;

        for (
            u32 index = 0;
                index < _quad_mngr->all.count;
              ++index
        ) {

            if (id == _quad_mngr->all.array[index]) {
                did_find = true;
                break;
            }
        }

        if (!did_find) {
            return(FALSE);
        }
    }

    IFB_INTERNAL bool 
    quad_lookup_by_id(
        quad_entity&    q,
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);

        bool found_entity = false;
        entity e;
        for (
            u32 index = 0;
            (
                index < _quad_mngr->all.count &&
                !found_entity
            );
            ++index
        ) {

            found_entity = (
                id == _quad_mngr->all.array[index] &&
                entity_lookup_by_id(e, id) 
            );
        }

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
            q.archetype & QUAD_ENTITY_ARCHETYPE == QUAD_ENTITY_ARCHETYPE &&
            q.id                                != ENTITY_ID_INVALID     &&
            q.tag                               != NULL 
        );

        cmpnt_position_table_update (q.pos,   q.index_sparse);
        cmpnt_color_table_update    (q.color, q.index_sparse);
        cmpnt_quad_table_update     (q.dims,  q.index_sparse);
    }

    IFB_INTERNAL void
    quad_lookup_all(
        quad_list* ql) {

        quad_list_validate(ql);

        for (
            u32 index = 0;
                index < _quad_mngr->all.count;
              ++index) {

            ql->array[index] = _quad_mngr->all.array[index];
        }
        ql->count = _quad_mngr->all.count;

    }

    IFB_INTERNAL void
    quad_render(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);
        quad_mngr_validate();

        for (
            u32 index = 0;
                index < _quad_mngr->to_render.count;
              ++index
        ) {
            if (id == _quad_mngr->to_render.array[index]) {
                return;
            }
        }

        (void)quad_list_add(
            &_quad_mngr->to_render,
            &id
        );
    }
};
