#pragma once

#include "entities.hpp"

namespace ifb {

    static void
    entity_tag_init(
        entity_tag*   tag,
        const cchar8* cstr) {

        assert(tag != NULL && cstr != NULL);

        // calculate the string length
        const u32 cstr_len = strnlen_s(cstr, ENTITY_TAG_SIZE);

        // zero the tag
        zero_memory((void*)tag, sizeof(entity_tag));

        // copy the string to the tag
        (void)memmove(
            (void*)tag->cstr, // dst
            (void*)cstr,      // src
            cstr_len          // size
        );

        //make sure the tag is null terminated
        tag->cstr[ENTITY_TAG_SIZE - 1] = 0;
    }   

    static u32
    entity_tag_hash(
        const entity_tag* tag) {

        const u32 hash = hash_u32(
            (void*)&tag->cstr[0],
            ENTITY_TAG_SIZE            
        );

        return(hash);
    }
};