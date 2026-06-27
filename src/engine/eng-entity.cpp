#pragma once

#include "ifb.hpp"
#include "eng-internal.hpp"

namespace ifb {
    
    IFB_ENGINE_API eng_entity_id
    eng_entity_create(
        const cchar8* tag) {

        assert(tag != NULL);

        eng_managers*   mngrs       = _eng_context->mngrs; 
        entity_manager* entity_mngr = mngrs->entity;

        const entity_id id = entity_create(tag);

        return(id.hash);
    }

    IFB_ENGINE_API bool
    eng_entity_destroy_by_id(
        const eng_entity_id id) {

        assert(id != ENTITY_ID_INVALID);
        const entity_id eid = {id};

        eng_managers*   mngrs       = _eng_context->mngrs; 
        entity_manager* entity_mngr = mngrs->entity;

        const bool did_destroy = entity_destroy_by_id(eid);
        return(did_destroy);
    }

    IFB_ENGINE_API bool
    eng_entity_destroy_by_tag (
        const cchar8* tag) {

        assert(tag);
        
        eng_managers*   mngrs       = _eng_context->mngrs; 
        entity_manager* entity_mngr = mngrs->entity;

        const bool did_destroy = entity_destroy_by_tag(tag);
        return(did_destroy);
    }

    IFB_ENGINE_API const cchar8*
    eng_entity_get_tag(
        const eng_entity_id id) {

        assert(id != ENTITY_ID_INVALID);
        const entity_id eid = { id };

        eng_managers*   mngrs       = _eng_context->mngrs; 
        entity_manager* entity_mngr = mngrs->entity;

        const u32 index = entity_lookup_index_by_id(eid);

        entity e;
        entity_get(index, &e);

        return(e.tag->cstr);
    }
};