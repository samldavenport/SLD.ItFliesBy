#pragma once

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
        
            array_id[index] = quad_create(tag_cstr);
            assert(array_id[index] != ENTITY_ID_INVALID);
        }
    }

    inline void
    update_quads(
        void) {

        arena* a = arena_alloc();
        assert(a);

        quad_list* ql = quad_list_create(a);
        assert(ql);

        quad_lookup_all(ql);

        for (
            u32 index = 0;
                index < ql->count;
              ++index
        ) {

            quad_entity q;
            const bool did_find = quad_lookup_by_id(q, ql->array[index]);
            assert(did_find);

            q.color.hex    =  0xFFFFFFFF;
            q.dims.width   =  0.1f;
            q.dims.height  =  0.1f;
            q.pos.x        =  0.0f;
            q.pos.y        =  0.0f;
            q.pos.z        = -1.0f;

            quad_update(q);
        }
    }

};