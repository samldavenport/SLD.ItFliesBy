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
    struct quad_list;
    struct quad_vertex;
    struct quad_vertex_buffer;

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

    IFB_INTERNAL quad_mngr*          quad_mngr_create            (void);
    IFB_INTERNAL void                quad_mngr_validate          (void);
    IFB_INTERNAL void                quad_mngr_startup           (memory& mem);

    // TODO(SAM): this doesn't actually create vertices, since its one
    // element per quad. It should be 4 vertices per quad, but thats something to resolve
    // between here and the renderer
    IFB_INTERNAL quad_vertex_buffer* quad_mngr_get_vertex_buffer (arena* a);
    IFB_INTERNAL void                quad_mngr_render_list_reset (void);

    IFB_INTERNAL entity_id           quad_create             (const cchar*  tag_cstr);
    IFB_INTERNAL void                quad_create_batch       (const cchar** tag_cstr, const u32 count, entity_id* id);
    IFB_INTERNAL bool                quad_lookup_by_tag      (quad_entity& q, const cchar*    tag_cstr);
    IFB_INTERNAL bool                quad_lookup_by_id       (quad_entity& q, const entity_id id);
    IFB_INTERNAL void                quad_update             (const quad_entity& q);
    IFB_INTERNAL void                quad_lookup_all         (quad_list* ql);
    IFB_INTERNAL void                quad_render             (const entity_id id);

    IFB_INTERNAL quad_list*          quad_list_create   (arena* a);
    IFB_INTERNAL void                quad_list_validate (const quad_list* ql);
    IFB_INTERNAL void                quad_list_reset    (quad_list* ql);
    IFB_INTERNAL bool                quad_list_add      (quad_list* ql, const entity_id* quad_id, const u32 count = 1);
    IFB_INTERNAL void                quad_list_remove   (quad_list* ql, const entity_id* quad_id, const u32 count = 1);

    IFB_INTERNAL void                quad_tests         (void);

    //--------------------------------------------------------------------
    // STRUCTURE DEFINITIONS
    //--------------------------------------------------------------------

    struct quad_list {
        entity_id* array;
        u32        count;
    };

    struct quad_mngr {
        stack      stack_mem;
        u32        capacity;
        quad_list  all;
        quad_list  to_render;
        quad_list  to_update;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        position_3d    pos;
    };

    struct quad_vertex {
        union {
            struct {
                f32 height;
                f32 width;
                f32 color_a;
                f32 color_b;
                f32 color_g;
                f32 color_r;
                f32 pos_z;
                f32 pos_y;
                f32 pos_x;
            };
            byte bytes[36];
        };
    };

    struct quad_vertex_buffer {
        union {
            quad_vertex* vertices;
            byte*        data;     
        };
        u32 vertex_count;
        u32 data_size;  
    };
};

#endif //IFB_QUAD_HPP