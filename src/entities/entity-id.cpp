#pragma once

#include "entities.hpp"

namespace ifb {

    static entity_id
    entity_id_init(
        const entity_tag* tag) {

        assert(tag);

        entity_id id;
        id.hash = entity_tag_hash(tag);

        return(id);
    }
};