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

    IFB_INLINE void entity_tag_init (entity_tag& tag, const cchar* cstr);
    IFB_INLINE u32  entity_tag_hash (const entity_tag& tag);

    IFB_INLINE void
    entity_tag_init (
        entity_tag&  tag,
        const cchar* cstr) {

        assert(cstr != NULL);

        memset((void*)tag.cstr, 0, ENTITY_TAG_SIZE);

        const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
        (void)strncpy(tag.cstr, cstr, len);
    }

    IFB_INLINE u32
    entity_tag_hash(
        const entity_tag& tag) {

        const u32 hash = hash_u32((void*)tag.cstr, ENTITY_TAG_SIZE);
        return(hash);
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