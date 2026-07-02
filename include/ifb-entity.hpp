#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include "ifb-types.hpp"
#include "ifb-components.hpp"

namespace ifb {

    constexpr u32 ENTITY_ID_INVALID = 0xFFFFFFFF;
    constexpr u32 ENTITY_TAG_SIZE   = 16;

    struct entity_tag {
        cchar cstr[ENTITY_TAG_SIZE];

        inline entity_tag() = default;
        inline entity_tag(
            const cchar* tag_cstr) {

            assert(tag_cstr != NULL);

            const u32 len = strnlen_s(tag_cstr, ENTITY_TAG_SIZE);
            (void)strncpy(cstr, tag_cstr, len);
        }

        inline u32
        hash(void) const {
            
            void*       data = (void*)this->cstr;
            const u32   size = ENTITY_TAG_SIZE; 
            const u32   h    = hash_u32(data, size);

            return(h);
        }
    };

    class entity {
    
    private:
        entity_id        id;
        entity_archetype archetype;
        entity_tag       tag;

    public:

        inline entity() = default;

        inline 
        entity(
            const cchar*             tag_cstr,
            const entity_archetype = component_type_e_none) {
            
            tag = entity_tag (tag_cstr);
            id  = tag.hash();
        } 

        inline
        entity(
            const entity_id        id,
            const entity_tag&      tag,
            const entity_archetype archetype = 0) {

            this->id        = id;
            this->tag       = tag;
            this->archetype = archetype;
        }

        inline entity&
        add_component(
            const component_type type) {

            archetype |= type.val;
            return(*this);
        }

        inline const cchar*
        tag_cstr(void) {

            return(tag.cstr);
        }
    };
};


#endif //IFB_ENTITY_HPP