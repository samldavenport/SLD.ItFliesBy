#ifndef IFB_QUAD_HPP
#define IFB_QUAD_HPP

#include "ifb.hpp"
#include "entity.hpp"
#include "component.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct quad_mngr;
    struct quad_entity;
    struct quad_vertex;
    struct quad_vertices;

    using quad_list = array_list<entity_id>;

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static const entity_archetype QUAD_ENTITY_ARCHETYPE = (
        cmpnt_type_e_position |
        cmpnt_type_e_color    |
        cmpnt_type_e_quad
    );

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static quad_mngr* _quad_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL quad_mngr* quad_mngr_create    (void);
    IFB_INTERNAL void       quad_mngr_validate  (void);
    IFB_INTERNAL void       quad_mngr_startup   (memory& mem);

    IFB_INTERNAL entity_id  quad_create         (const cchar*  tag_cstr);
    IFB_INTERNAL entity_id  quad_create         (const cchar*  tag_cstr, quad q);
    IFB_INTERNAL void       quad_create_batch   (const cchar** tag_cstr, const u32 count, entity_id* id);
    IFB_INTERNAL bool       quad_lookup_by_tag  (quad_entity& q, const cchar*    tag_cstr);
    IFB_INTERNAL bool       quad_lookup_by_id   (quad_entity& q, const entity_id id);
    IFB_INTERNAL bool       quad_does_exist     (const entity_id id);
    IFB_INTERNAL void       quad_update         (const quad_entity& q);
    IFB_INTERNAL void       quad_lookup_all     (quad_list& ql);
    IFB_INTERNAL bool       quad_get_vertices   (quad_vertices& qv, const entity_id id);
    IFB_INTERNAL bool       quad_list_init      (quad_list& ql, arena* a);

    IFB_INTERNAL void       quad_tests          (void);

    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_mngr {
        stack      stack_mem;
        quad_list  all;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        position_3d    pos;
    };

    struct quad_vertex {
        union {
            struct {
                f32 pos_x;
                f32 pos_y;
                f32 pos_z;
                f32 color_r;
                f32 color_g;
                f32 color_b;
                f32 color_a;
            };
            byte bytes[28];
        };
    };

    struct quad_vertices {
        union {
            struct {
                quad_vertex top_right;
                quad_vertex bottom_right;
                quad_vertex bottom_left;
                quad_vertex top_left;
            };
            byte data[112];
        };
    };
};

#endif //IFB_QUAD_HPP