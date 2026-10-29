#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "ifb-components.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------
    
    constexpr u32 ENTITY_ID_INVALID = INVALID_U32;
    constexpr u32 ENTITY_TAG_SIZE   = 16;

    //--------------------------------------------------------------------
    // ENTITY ARCHETYPES 
    //--------------------------------------------------------------------

    static const component_type ENTITY_ARCHETYPE_SPRITE = (
        cmpnt_type_e_position   |
        cmpnt_type_e_color      |
        cmpnt_type_e_quad       |
        cmpnt_type_e_map_coords |
        cmpnt_type_e_sprite
    );

    static const component_type ENTITY_ARCHETYPE_PHYSICS_PARTICLE = (
        cmpnt_type_e_position      |
        cmpnt_type_e_velocity      |
        cmpnt_type_e_acceleration  |
        cmpnt_type_e_inv_mass      |
        cmpnt_type_e_drag          |     
        cmpnt_type_e_term_velocity
    );

    static const component_type ENTITY_ARCHETYPE_PHYSICS_SPRITE = (
        ENTITY_ARCHETYPE_SPRITE | 
        ENTITY_ARCHETYPE_PHYSICS_PARTICLE 
    );

    //--------------------------------------------------------------------
    // ENTITY TAG
    //--------------------------------------------------------------------

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
        cmpnt_position pos;
        cmpnt_sprite   sprite;
    };

    struct entity_query {
        component_type has_all;
        component_type has_any;
        component_type has_none;
    };

    struct tag {
        cchar cstr[16];
        
        tag(const cchar* cstr) {
            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);
            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }
        inline void
        init(
            const cchar* tag_cstr)  {
            assert(cstr != NULL);
            (void)strncpy_s(cstr, 16, tag_cstr, 16);
        }

        inline u32
        hash(void) {
            return(hash_u32((void*)cstr, 16));
        }
    };

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct entity_list;

};

#endif //IFB_ENTITIES_HPP
