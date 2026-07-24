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
    struct cmpnt_table_rigid_body;
    struct cmpnt_table_velocity;
    struct cmpnt_table_acceleration;

    struct cmpnt_list_position;
    struct cmpnt_list_color;
    struct cmpnt_list_quad;
    struct cmpnt_list_rigid_body;
    struct cmpnt_list_velocity;
    struct cmpnt_list_acceleration;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static cmpnt_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr*              cmpnt_mngr_create               (void);
    IFB_INTERNAL void                     cmpnt_mngr_startup              (memory& mem_res);

    IFB_INTERNAL cmpnt_list_position*     cmpnt_position_list_create      (arena* arena);
    IFB_INTERNAL void                     cmpnt_position_list_validate    (const cmpnt_list_position* list_position);
    IFB_INTERNAL bool                     cmpnt_position_list_add         (cmpnt_list_position*       list_position, const cmpnt_position& position);
    IFB_INTERNAL void                     cmpnt_position_table_validate   (void);
    IFB_INTERNAL void                     cmpnt_position_table_update     (const cmpnt_list_position* list_position);
    IFB_INTERNAL void                     cmpnt_position_table_update     (const cmpnt_position&      position);
    IFB_INTERNAL void                     cmpnt_position_table_update     (const position_3d&         position,      const u32 sparse_index);
    IFB_INTERNAL void                     cmpnt_position_table_lookup     (cmpnt_list_position*       list_position, const entity_list* list_entity);
    IFB_INTERNAL void                     cmpnt_position_table_lookup     (cmpnt_position&            position,      const entity& e);
    IFB_INTERNAL void                     cmpnt_position_table_lookup     (position_3d&               position,      const u32 sparse_index);

    IFB_INTERNAL cmpnt_list_color*        cmpnt_color_list_create         (arena* a);
    IFB_INTERNAL void                     cmpnt_color_list_validate       (const cmpnt_list_color* list_color);
    IFB_INTERNAL bool                     cmpnt_color_list_add            (cmpnt_list_color*       list_color, const cmpnt_color& color);
    IFB_INTERNAL void                     cmpnt_color_table_validate      (void);
    IFB_INTERNAL void                     cmpnt_color_table_update        (const cmpnt_color&      color);
    IFB_INTERNAL void                     cmpnt_color_table_update        (const color_rgba_u32&   color, const u32 sparse_index);
    IFB_INTERNAL void                     cmpnt_color_table_update        (const cmpnt_list_color* list_color);
    IFB_INTERNAL void                     cmpnt_color_table_lookup        (cmpnt_list_color*       list_color, const entity_list* list_entity);
    IFB_INTERNAL void                     cmpnt_color_table_lookup        (cmpnt_color&            color,      const entity&      e);
    IFB_INTERNAL void                     cmpnt_color_table_lookup        (color_rgba_u32&         color,      const u32          sparse_index);

    IFB_INTERNAL cmpnt_list_quad*         cmpnt_quad_list_create          (arena* a);
    IFB_INTERNAL void                     cmpnt_quad_list_validate        (const cmpnt_list_quad* list_quad);
    IFB_INTERNAL bool                     cmpnt_quad_list_add             (cmpnt_list_quad*       list_quad, const cmpnt_quad& quad);
    IFB_INTERNAL void                     cmpnt_quad_table_validate       (void);
    IFB_INTERNAL void                     cmpnt_quad_table_update         (const cmpnt_quad&    quad);
    IFB_INTERNAL void                     cmpnt_quad_table_update         (const dimensions_2d& dims, const u32 sparse_index);
    IFB_INTERNAL void                     cmpnt_quad_table_update         (const cmpnt_list_quad* list_quad);
    IFB_INTERNAL void                     cmpnt_quad_table_lookup         (cmpnt_list_quad*       list_quad, const entity_list* list_entity);
    IFB_INTERNAL void                     cmpnt_quad_table_lookup         (cmpnt_quad*            quad,      const entity&      e);
    IFB_INTERNAL void                     cmpnt_quad_table_lookup         (dimensions_2d&         dims,      const u32          sparse_index);

    IFB_INTERNAL cmpnt_list_rigid_body*   cmpnt_rigid_body_list_create    (arena* a);
    IFB_INTERNAL void                     cmpnt_rigid_body_list_validate  (const cmpnt_list_rigid_body* list_rb);
    IFB_INTERNAL bool                     cmpnt_rigid_body_list_add       (cmpnt_list_rigid_body*       list_rb, const cmpnt_rigid_body& rb);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_validate (void);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_update   (const cmpnt_rigid_body&    rb);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_update   (rigid_body& rb, const u32 sparse_index);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_update   (const cmpnt_list_rigid_body* list_rb);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_lookup   (cmpnt_list_rigid_body*       list_rb, const entity_list* list_entity);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_lookup   (cmpnt_rigid_body& rb, const entity& e);
    IFB_INTERNAL void                     cmpnt_rigid_body_table_lookup   (rigid_body&       rb, const u32     sparse_index);

    IFB_INTERNAL cmpnt_list_velocity*     cmpnt_velocity_list_create      (arena* a);
    IFB_INTERNAL void                     cmpnt_velocity_list_validate    (const cmpnt_list_velocity* list_vel);
    IFB_INTERNAL bool                     cmpnt_velocity_list_add         (cmpnt_list_velocity*       list_vel, const cmpnt_velocity& vel);
    IFB_INTERNAL void                     cmpnt_velocity_table_validate   (void);
    IFB_INTERNAL void                     cmpnt_velocity_table_update     (const cmpnt_velocity& vel);
    IFB_INTERNAL void                     cmpnt_velocity_table_update     (const velocity_3d& vel, const u32 sparse_index);
    IFB_INTERNAL void                     cmpnt_velocity_table_update     (const cmpnt_list_velocity* list_vel);
    IFB_INTERNAL void                     cmpnt_velocity_table_lookup     (cmpnt_list_velocity*       list_vel, const entity_list* list_entity);
    IFB_INTERNAL void                     cmpnt_velocity_table_lookup     (cmpnt_velocity* vel,      const entity&      e);
    IFB_INTERNAL void                     cmpnt_velocity_table_lookup     (velocity_3d& vel,      const u32          sparse_index);

    IFB_INTERNAL cmpnt_list_acceleration* cmpnt_acceleration_list_create    (arena* a);
    IFB_INTERNAL void                     cmpnt_acceleration_list_validate  (const cmpnt_list_acceleration* list_accel);
    IFB_INTERNAL bool                     cmpnt_acceleration_list_add       (cmpnt_list_acceleration*       list_accel, const cmpnt_acceleration& accel);
    IFB_INTERNAL void                     cmpnt_acceleration_table_validate (void);
    IFB_INTERNAL void                     cmpnt_acceleration_table_update   (const cmpnt_acceleration& accel);
    IFB_INTERNAL void                     cmpnt_acceleration_table_update   (const acceleration_3d& accel, const u32 sparse_index);
    IFB_INTERNAL void                     cmpnt_acceleration_table_update   (const cmpnt_list_acceleration* list_accel);
    IFB_INTERNAL void                     cmpnt_acceleration_table_lookup   (cmpnt_list_acceleration*       list_accel, const entity_list* list_entity);
    IFB_INTERNAL void                     cmpnt_acceleration_table_lookup   (cmpnt_acceleration& accel, const entity&      e);
    IFB_INTERNAL void                     cmpnt_acceleration_table_lookup   (acceleration_3d& accel,    const u32          sparse_index);


    //--------------------------------------------------------------------
    // COMPONENT MANAGER
    //--------------------------------------------------------------------

    struct cmpnt_mngr {
        stack mem;
        u32   capacity;
        struct  {
            cmpnt_table_position*     position;
            cmpnt_table_color*        color;
            cmpnt_table_quad*         quad;
            cmpnt_table_rigid_body*   rigid_body;
            cmpnt_table_velocity*     velocity;
            cmpnt_table_acceleration* acceleration;
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

    struct cmpnt_table_rigid_body {
        f32* origin_x;
        f32* origin_y;
        f32* origin_z;
        f32* width;
        f32* height;
    };

    struct cmpnt_table_velocity {
        f32* x;
        f32* y;
        f32* z;
    };

    struct cmpnt_table_acceleration {
        f32* x;
        f32* y;
        f32* z;
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

    struct cmpnt_list_rigid_body {
        struct {
            entity_id* id;
            u32*       sparse_index;
            f32*       origin_x;
            f32*       origin_y;
            f32*       origin_z;
            f32*       width;
            f32*       height;
        } data;
        u32 count;
    };

    struct cmpnt_list_velocity {
        struct {
            entity_id* id;
            u32*       sparse_index;
            f32*       x;
            f32*       y;
            f32*       z;            
        } data;
        u32 count;
    };

    struct cmpnt_list_acceleration {
        struct {
            entity_id* id;
            u32*       sparse_index;
            f32*       x;
            f32*       y;
            f32*       z;            
        } data;
        u32 count;
    };
};

#endif //COMPONENTS_HPP