#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include "ifb-types.hpp"
#include "ifb-component.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENTITY ARCHETYPES 
    //--------------------------------------------------------------------

    static const entity_archetype ENTITY_ARCHETYPE_QUAD = (
        cmpnt_type_e_position |
        cmpnt_type_e_color    |
        cmpnt_type_e_quad
    );

    static const entity_archetype ENTITY_ARCHETYPE_PHYSICS_PARTICLE = (
        cmpnt_type_e_position     |
        cmpnt_type_e_rigid_body   |
        cmpnt_type_e_velocity     |
        cmpnt_type_e_acceleration |
        cmpnt_type_e_inv_mass     |
        cmpnt_type_e_drag         |     
        cmpnt_type_e_term_velocity 
    );

    static const entity_archetype ENTITY_ARCHETYPE_PHYSICS_QUAD = (
        ENTITY_ARCHETYPE_QUAD | 
        ENTITY_ARCHETYPE_PHYSICS_PARTICLE 
    );

    //--------------------------------------------------------------------
    // ENTITY TAG
    //--------------------------------------------------------------------

    struct entity_tag {

        cchar cstr[ENTITY_TAG_SIZE];

        entity_tag(const cchar* cstr) {
            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);
            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }

        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);

            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }

        inline entity_id
        to_id(void) const {

            const entity_id id = hash_u32((void*)this->cstr, ENTITY_TAG_SIZE);
            return(id);
        }
    };

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
