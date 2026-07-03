#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include "ifb-types.hpp"
#include "ifb-components.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENTITY TAG
    //--------------------------------------------------------------------

    class entity_tag {
    
    private:
        cchar _cstr[ENTITY_TAG_SIZE];

    public:
        inline entity_tag() = default;
        inline entity_tag(
            const cchar* tag_cstr) {

            assert(tag_cstr != NULL);

            const u32 len = strnlen_s(tag_cstr, ENTITY_TAG_SIZE);
            (void)strncpy(_cstr, tag_cstr, len);
        }

        inline u32
        hash(void) const {
            
            void*      data = (void*)_cstr;
            const u32  size = ENTITY_TAG_SIZE; 
            const u32  h    = hash_u32(data, size);

            return(h);
        }

        inline const cchar*
        cstr(void) {
            return(_cstr);
        }
    };

    //--------------------------------------------------------------------
    // ENTITY
    //--------------------------------------------------------------------

    class entity {
    
    private:
    
        entity_id        _id;
        entity_archetype _archetype;
        entity_tag       _tag;

    public:

        inline entity() = default;

        inline 
        entity(
            const cchar*             tag_cstr,
            const entity_archetype = component_type_e_none) {
            
            _tag = entity_tag (tag_cstr);
            _id  = _tag.hash();
        } 

        inline
        entity(
            const entity_id        id,
            const entity_tag&      tag,
            const entity_archetype archetype = 0) {

            _id        = id;
            _tag       = tag;
            _archetype = archetype;

            assert(
                _tag.hash() == _id
            );
        }

        inline entity&
        add_component(
            const component_type type) {

            _archetype |= type.val;
            return(*this);
        }

        inline const cchar*     tag_cstr (void) { return (_tag.cstr()); }
        inline entity_id        id       (void) { return (_id);         }
        inline entity_archetype archetype(void) { return (_archetype);  }
    };
};


#endif //IFB_ENTITY_HPP