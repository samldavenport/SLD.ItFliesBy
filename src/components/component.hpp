#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"
#include "memory.hpp"
#include "entity.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct cmpnt_mngr;
    struct cmpnt_table_position;
    struct cmpnt_table_color;
    struct cmpnt_table_quad;
    struct cmpnt_list_position;
    struct cmpnt_list_color;
    struct cmpnt_list_quad;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static cmpnt_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr*          cmpnt_mngr_create            (void);
    IFB_INTERNAL void                 cmpnt_mngr_startup           (memory& mem_res);


    IFB_INTERNAL cmpnt_list_position* cmpnt_position_list_create    (arena* arena);
    IFB_INTERNAL void                 cmpnt_position_list_validate  (const cmpnt_list_position* list_position);
    IFB_INTERNAL bool                 cmpnt_position_list_add       (cmpnt_list_position*       list_position, const cmpnt_position& position);
    IFB_INTERNAL void                 cmpnt_position_table_validate (void);
    IFB_INTERNAL void                 cmpnt_position_table_update   (const cmpnt_list_position* list_position);
    IFB_INTERNAL void                 cmpnt_position_table_update   (const cmpnt_position&      position);
    IFB_INTERNAL void                 cmpnt_position_table_update   (const position_3d&         position,      const u32 sparse_index);
    IFB_INTERNAL void                 cmpnt_position_table_lookup   (cmpnt_list_position*       list_position, const entity_list* list_entity);
    IFB_INTERNAL void                 cmpnt_position_table_lookup   (cmpnt_position&            position,      const entity& e);
    IFB_INTERNAL void                 cmpnt_position_table_lookup   (position_3d&               position,      const u32 sparse_index);

    IFB_INTERNAL cmpnt_list_color*    cmpnt_color_list_create      (arena* a);
    IFB_INTERNAL void                 cmpnt_color_list_validate    (const cmpnt_list_color* list_color);
    IFB_INTERNAL bool                 cmpnt_color_list_add         (cmpnt_list_color*       list_color, const cmpnt_color& color);
    IFB_INTERNAL void                 cmpnt_color_table_validate   (void);
    IFB_INTERNAL void                 cmpnt_color_table_update     (const cmpnt_color&      color);
    IFB_INTERNAL void                 cmpnt_color_table_update     (const color_rgba_u32&   color, const u32 sparse_index);
    IFB_INTERNAL void                 cmpnt_color_table_update     (const cmpnt_list_color* list_color);
    IFB_INTERNAL void                 cmpnt_color_table_lookup     (cmpnt_list_color*       list_color, const entity_list* list_entity);
    IFB_INTERNAL void                 cmpnt_color_table_lookup     (cmpnt_color&            color,      const entity&      e);
    IFB_INTERNAL void                 cmpnt_color_table_lookup     (color_rgba_u32&         color,      const u32          sparse_index);

    IFB_INTERNAL cmpnt_list_quad*     cmpnt_quad_list_create       (arena* a);
    IFB_INTERNAL void                 cmpnt_quad_list_validate     (const cmpnt_list_quad* list_quad);
    IFB_INTERNAL bool                 cmpnt_quad_list_add          (cmpnt_list_quad*       list_quad, const cmpnt_quad& quad);
    IFB_INTERNAL void                 cmpnt_quad_table_validate    (void);
    IFB_INTERNAL void                 cmpnt_quad_table_update      (const cmpnt_quad&    quad);
    IFB_INTERNAL void                 cmpnt_quad_table_update      (const dimensions_2d& dims, const u32 sparse_index);
    IFB_INTERNAL void                 cmpnt_quad_table_update      (const cmpnt_list_quad* list_quad);
    IFB_INTERNAL void                 cmpnt_quad_table_lookup      (cmpnt_list_quad*       list_quad, const entity_list* list_entity);
    IFB_INTERNAL void                 cmpnt_quad_table_lookup      (cmpnt_quad*            quad,      const entity&      e);
    IFB_INTERNAL void                 cmpnt_quad_table_lookup      (dimensions_2d&         dims,      const u32          sparse_index);

    //--------------------------------------------------------------------
    // COMPONENT MANAGER
    //--------------------------------------------------------------------

    struct cmpnt_mngr {
        stack mem;
        u32   capacity;
        struct  {
            cmpnt_table_position* position;
            cmpnt_table_color*    color;
            cmpnt_table_quad*     quad;
        } tables;
    };

    //--------------------------------------------------------------------
    // TABLE DEFINITIONS
    //--------------------------------------------------------------------

    struct cmpnt_table_position {
        f32* x;
        f32* y;
        f32* z;
    };

    struct cmpnt_table_color {
        u32* rgba_hex;
    };

    struct cmpnt_table_quad {
        f32* width;
        f32* height;
    };

    //--------------------------------------------------------------------
    // LIST DEFINITIONS
    //--------------------------------------------------------------------

    struct cmpnt_list_position {
        struct {
            entity_id* id;
            u32*       sparse_index;
            f32*       x;
            f32*       y;
            f32*       z;            
        } data;
        u32 count;
    };

    struct cmpnt_list_color {
        struct {
            entity_id*      id;
            u32*            sparse_index;
            color_rgba_u32* color;
        } data;
        u32 count;
    };

    struct cmpnt_list_quad {
        struct {
            entity_id* id;
            u32*       sparse_index;
            f32*       width;
            f32*       height;
        } data;
        u32 count;
    };
};

#endif //COMPONENTS_HPP