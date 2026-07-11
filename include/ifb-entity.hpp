#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include "ifb-types.hpp"
#include "ifb-component.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENTITY TAG
    //--------------------------------------------------------------------

    struct entity_tag {
        cchar cstr[ENTITY_TAG_SIZE];
    };

    IFB_INLINE void      entity_tag_init       (entity_tag& tag, const cchar* cstr);
    IFB_INLINE entity_id entity_tag_to_id      (const entity_tag& tag);
    IFB_INLINE entity_id entity_tag_cstr_to_id (const cchar* tag_cstr);

    IFB_INLINE void
    entity_tag_init (
        entity_tag&  tag,
        const cchar* cstr) {

        assert(cstr != NULL);

        memset((void*)tag.cstr, 0, ENTITY_TAG_SIZE);

        const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
        (void)strncpy(tag.cstr, cstr, len);
    }

    IFB_INLINE entity_id
    entity_tag_to_id(
        const entity_tag& tag) {

        const entity_id id = hash_u32((void*)tag.cstr, ENTITY_TAG_SIZE);
        return(id);
    }

    IFB_INLINE entity_id
    entity_tag_cstr_to_id(
        const cchar* tag_cstr) {
        
        assert(tag_cstr);
        const entity_id id = hash_u32((void*)tag_cstr, ENTITY_TAG_SIZE);
        return(id);
    }

    //--------------------------------------------------------------------
    // ENTITY
    //--------------------------------------------------------------------

    struct entity {
        const char*      tag;             
        entity_id        id;
        entity_archetype archetype;
        u32              index_sparse;
        u32              index_dense;
    };
};


#endif //IFB_ENTITY_HPP