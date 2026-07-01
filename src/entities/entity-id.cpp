#pragma once

#include "entities.hpp"
#include "ifb-entity.hpp"

namespace ifb {

    IFB_INTERNAL entity_id
    entity_id_init(
        const entity_tag* tag) {

        assert(tag);

        entity_id id;
        id.hash = entity_tag_hash(tag);

        return(id);
    }

    IFB_INTERNAL entity_id
    entity_id_init(
        const cchar* tag_cstr) {

        assert(tag_cstr);

        const entity_id id = hash_u32((void*)tag_cstr, ENTITY_TAG_SIZE);
        return(id);
    }
};