#ifndef IFB_ENTITIES_HPP
#define IFB_ENTITIES_HPP

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "ifb-components.hpp"
#include "ifb-memory.hpp"

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

    struct entity_mngr;
    struct entity_list;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static entity_mngr* _entity_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // entity manager
    IFB_INTERNAL entity_mngr*     entity_mngr_create               (void);
    IFB_INTERNAL void             entity_mngr_validate             (void);
    IFB_INTERNAL void             entity_mngr_startup              (reservation* res);
    IFB_INTERNAL void             entity_mngr_shutdown             (void);
    IFB_INTERNAL u32              entity_mngr_capacity_dense       (void);
    IFB_INTERNAL u32              entity_mngr_capacity_sparse      (void);
    IFB_INTERNAL u32              entity_mngr_get_count            (void);

    // entity
    IFB_INTERNAL entity_id        entity_create                    (const cchar* tag_cstr);
    IFB_INTERNAL bool             entity_add_archetype             (const entity_id id, const entity_archetype atype);
    IFB_INTERNAL bool             entity_remove_archetype          (const entity_id id, const entity_archetype atype);
    IFB_INTERNAL bool             entity_destroy                   (const entity_id id);
    IFB_INTERNAL bool             entity_has_component             (const entity& e, const component_type type);

    // lookup
    IFB_INTERNAL bool             entity_lookup_list               (entity_list* list, const entity_query& query);
    IFB_INTERNAL bool             entity_lookup_by_tag             (entity& e, const cchar* tag_cstr);
    IFB_INTERNAL bool             entity_lookup_by_index_dense     (entity& e, const u32    index);
    IFB_INTERNAL bool             entity_lookup_by_id              (entity& e, const entity_id id);
    IFB_INTERNAL u32              entity_lookup_sparse_index       (const entity_id id);
    IFB_INTERNAL entity_archetype entity_lookup_archetype          (const entity_id id);
    IFB_INTERNAL const cchar*     entity_lookup_tag                (const entity_id id);

    // components
    IFB_INTERNAL bool             entity_component_add             (const entity_id id, const component_type types);
    IFB_INTERNAL bool             entity_component_remove          (const entity_id id, const component_type types);

    // testing
    IFB_INTERNAL void             entity_test (void);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct entity_mngr {
        struct {
            struct {
                entity_id*        id;
                entity_tag*       tag;
                entity_archetype* archetype;
                u32*              sparse_index;
            } dense;
            struct {
                u32* dense_index;
            } sparse;
        } data;
        struct {
            u32 dense;
            u32 sparse;
        } capacity;
        reservation* res;
        u32          count;
    };
};

#endif //IFB_ENTITIES_HPP
