#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {
    
    IFB_ENGINE_API entity_id
    eng_entity_create(
        const cchar* tag) {

        assert(tag != NULL);

        eng_managers*   mngrs       = _eng_context->mngrs; 
        entity_manager* entity_mngr = mngrs->entity;

        const entity_id id = entity_create(tag);

        return(id);
    }
};