#pragma once

#include "quad.hpp"

namespace ifb {

    inline void create_100_quads (void);

    IFB_INTERNAL void
    quad_tests(
        void) {

        create_100_quads();
    }

    inline void
    create_100_quads(
        void) {

        entity_id  array_id  [100] = {0};
        char       tag_cstr  [16]  = {0};

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
};