#pragma once

#include "entity.cpp"
#include "ifb-entity.hpp"
#include "quad.hpp"

namespace ifb {

    inline void create_100_quads (void);
    inline void update_quads     (void);

    IFB_INTERNAL void
    quad_tests(
        void) {

        create_100_quads();
        update_quads();
    }

    inline void
    create_100_quads(
        void) {

        entity_id array_id [100] = {0};
        char      tag_cstr [16]  = {0};

        for (
            u32 index = 0;
                index < 100;
              ++index
        ) {

            (void)memset((void*)tag_cstr, 0, sizeof(tag_cstr));
            (void)sprintf_s(tag_cstr, sizeof(tag_cstr), "ENTITY-%d",index);
       

            array_id[index] = entity_create(tag_cstr);

            assert(array_id[index] != ENTITY_ID_INVALID);

            entity_add_archetype(array_id[index], ENTITY_ARCHETYPE_QUAD);
        }
    }

    inline void
    update_quads(
        void) {

        const eng_arena_handle a = arena_alloc();

        entity_list quad_list;
        const bool did_init = quad_list.arena_init(a);
        assert(did_init);

        entity_query query = {0};
        query.has_all.val = ENTITY_ARCHETYPE_QUAD.val;
        entity_lookup_list(quad_list, query);
            

        for (
            u32 index = 0;
                index < quad_list.count();
              ++index
        ) {


            quad_entity q;
            const bool did_find = quad_lookup_by_id(q, quad_list[index]);
            assert(did_find);

            q.color.hex    =  0xFFFFFFFF;
            q.dims.width   =  0.1f;
            q.dims.height  =  0.1f;
            q.pos.x        =  0.0f;
            q.pos.y        =  0.0f;
            q.pos.z        = -1.0f;

            quad_update(q);
        }

        arena_free(a); 
    }
};
