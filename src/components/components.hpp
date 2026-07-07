#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"
#include "memory.hpp"
#include "entities.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct component_mngr;
    struct component_table_position;
    struct component_table_color;
    struct component_table_quad;
    struct component_list_position;
    struct component_list_color;
    struct component_list_quad;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static component_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL component_mngr*          component_mngr_create            (void);
    IFB_INTERNAL void                     component_mngr_startup           (memory& mem_res);

    IFB_INTERNAL component_list_position* component_position_list_create   (arena* arena);
    IFB_INTERNAL void                     component_position_list_validate (const component_list_position* list_position);
    IFB_INTERNAL void                     component_position_table_update  (const component_list_position* list_position);
    IFB_INTERNAL void                     component_position_table_lookup  (component_list_position*       list_position, const entity_list* list_entity);

    IFB_INTERNAL component_list_color*    component_color_list_create      (arena* a);
    IFB_INTERNAL void                     component_color_list_validate    (const component_list_color* list_color);
    IFB_INTERNAL void                     component_color_table_update     (const component_list_color* list_color);
    IFB_INTERNAL void                     component_color_table_lookup     (component_list_color*       list_color, const entity_list* list_entity);

    IFB_INTERNAL component_list_quad*     component_quad_list_create       (arena* a);
    IFB_INTERNAL void                     component_quad_list_validate     (const component_list_quad* list_quad);
    IFB_INTERNAL void                     component_quad_table_update      (const component_list_quad* list_quad);
    IFB_INTERNAL void                     component_quad_table_lookup      (component_list_quad*       list_quad, const entity_list* list_entity);

    //--------------------------------------------------------------------
    // COMPONENT MANAGER
    //--------------------------------------------------------------------

    struct component_mngr {
        stack mem;
        u32   capacity;
        struct  {
            component_table_position* position;
            component_table_color*    color;
            component_table_quad*     quad;
        } tables;
    };

    //--------------------------------------------------------------------
    // TABLE DEFINITIONS
    //--------------------------------------------------------------------

    struct component_table_position {
        f32* x;
        f32* y;
        f32* z;
    };

    struct component_table_color {
        u32* rgba_hex;
    };

    struct component_table_quad {
        u32* width;
        u32* height;
    };

    //--------------------------------------------------------------------
    // LIST DEFINITIONS
    //--------------------------------------------------------------------

    struct component_list_position {
        struct {
            entity_id* id;
            u32*       sparse_index;
            f32*       x;
            f32*       y;
            f32*       z;            
        } data;
        u32 count;
    };

    struct component_list_color {
        struct {
            entity_id*      id;
            u32*            sparse_index;
            color_rgba_u32* color;
        } data;
        u32 count;
    };

    struct component_list_quad {
        struct {
            entity_id* id;
            u32*       width;
            u32*       height;
        } data;
        u32 count;
    };
};

#endif //COMPONENTS_HPP