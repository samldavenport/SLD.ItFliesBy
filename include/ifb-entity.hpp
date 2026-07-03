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
    
    public:
        entity_id        id;
        entity_archetype archetype;
        u32              sparse_index;
        u32              dense_index;
    };
};


#endif //IFB_ENTITY_HPP