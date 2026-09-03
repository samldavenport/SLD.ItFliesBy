#ifndef IFB_ENTITY_HPP
#define IFB_ENTITY_HPP

#include "ifb-types.hpp"
#include "ifb-component.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENTITY ARCHETYPES 
    //--------------------------------------------------------------------

    static const component_type ENTITY_ARCHETYPE_QUAD = (
        cmpnt_type_e_position |
        cmpnt_type_e_color    |
        cmpnt_type_e_quad
    );

    static const component_type ENTITY_ARCHETYPE_PHYSICS_PARTICLE = (
        cmpnt_type_e_position     |
        cmpnt_type_e_rigid_body   |
        cmpnt_type_e_velocity     |
        cmpnt_type_e_acceleration |
        cmpnt_type_e_inv_mass     |
        cmpnt_type_e_drag         |     
        cmpnt_type_e_term_velocity 
    );

    static const component_type ENTITY_ARCHETYPE_PHYSICS_QUAD = (
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

    struct entity_archetype {

        component_type cmpnt_mask;

        entity_archetype(void) = default;
        entity_archetype(const component_type val) : cmpnt_mask(val) {}

        inline void reset(void) {
            cmpnt_mask = 0;
        }

        inline bool has_any  (const component_type types) const { return((cmpnt_mask & types) !=  0);    } 
        inline bool has_all  (const component_type types) const { return((cmpnt_mask & types) == types); } 
        inline bool has_none (const component_type types) const { return((cmpnt_mask & types) == 0);     } 
    
        inline void 
        add(const component_type types) {

            cmpnt_mask |= types;
        }

        inline void
        remove(const component_type types) {
            cmpnt_mask &= ~types.val;
        }
    };

    struct entity {
        const char*      tag;             
        entity_id        id;
        entity_archetype archetype;
        u32              index_sparse;
        u32              index_dense;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        position_3d    pos;
    };

    struct entity_query {
        component_type has_all;
        component_type has_any;
        component_type has_none;
    };
};


#endif //IFB_ENTITY_HPP
