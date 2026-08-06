#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb-component.hpp"
#include "ifb-types.hpp"
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
    struct cmpnt_table_inv_mass;
    struct cmpnt_table_drag;
    struct cmpnt_table_term_velocity;

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static cmpnt_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr* cmpnt_mngr_create               (void);
    IFB_INTERNAL void        cmpnt_mngr_startup              (memory& mem_res);

    IFB_INTERNAL void        cmpnt_table_position_validate      (void);
    IFB_INTERNAL void        cmpnt_table_position_update        (const cmpnt_position& position);
    IFB_INTERNAL void        cmpnt_table_position_update        (const position_3d&    position, const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_table_position_lookup        (cmpnt_position&       position, const entity& e);
    IFB_INTERNAL void        cmpnt_table_position_lookup        (position_3d&          position, const u32 sparse_index);

    IFB_INTERNAL void        cmpnt_table_color_validate         (void);
    IFB_INTERNAL void        cmpnt_table_color_update           (const cmpnt_color&      color);
    IFB_INTERNAL void        cmpnt_table_color_update           (const color_rgba_u32&   color, const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_table_color_lookup           (cmpnt_color&            color, const entity&      e);
    IFB_INTERNAL void        cmpnt_table_color_lookup           (color_rgba_u32&         color, const u32          sparse_index);

    IFB_INTERNAL void        cmpnt_table_quad_validate          (void);
    IFB_INTERNAL void        cmpnt_table_quad_update            (const cmpnt_quad&    quad);
    IFB_INTERNAL void        cmpnt_table_quad_update            (const dimensions_2d& dims, const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_table_quad_lookup            (cmpnt_quad*          quad, const entity&      e);
    IFB_INTERNAL void        cmpnt_table_quad_lookup            (dimensions_2d&       dims, const u32          sparse_index);

    IFB_INTERNAL void        cmpnt_table_rigid_body_validate    (void);
    IFB_INTERNAL void        cmpnt_table_rigid_body_update      (const cmpnt_rigid_body&    rb);
    IFB_INTERNAL void        cmpnt_table_rigid_body_update      (const rigid_body& rb, const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_table_rigid_body_lookup      (cmpnt_rigid_body& rb, const entity& e);
    IFB_INTERNAL void        cmpnt_table_rigid_body_lookup      (rigid_body&       rb, const u32     sparse_index);

    IFB_INTERNAL void        cmpnt_table_velocity_validate      (void);
    IFB_INTERNAL void        cmpnt_table_velocity_update        (const cmpnt_velocity& vel);
    IFB_INTERNAL void        cmpnt_table_velocity_update        (const velocity_3d& vel, const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_table_velocity_lookup        (cmpnt_velocity*    vel, const entity&      e);
    IFB_INTERNAL void        cmpnt_table_velocity_lookup        (velocity_3d&       vel, const u32          sparse_index);

    IFB_INTERNAL void        cmpnt_table_acceleration_validate  (void);
    IFB_INTERNAL void        cmpnt_table_acceleration_update    (const cmpnt_acceleration& accel);
    IFB_INTERNAL void        cmpnt_table_acceleration_update    (const acceleration_3d&    accel, const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_table_acceleration_lookup    (cmpnt_acceleration&       accel, const entity&      e);
    IFB_INTERNAL void        cmpnt_table_acceleration_lookup    (acceleration_3d&          accel, const u32          sparse_index);

    //TODO(SAM): inconsistent naming needs to be fixed

    IFB_INTERNAL void        cmpnt_table_inv_mass_validate      (void);
    IFB_INTERNAL void        cmpnt_table_inv_mass_update        (const cmpnt_inv_mass& inv_mass);
    IFB_INTERNAL void        cmpnt_table_inv_mass_update        (const u32 sparse_index, const inv_mass& inv_mass);
    IFB_INTERNAL void        cmpnt_table_inv_mass_lookup        (const u32 sparse_index, inv_mass&       inv_mass);
    IFB_INTERNAL void        cmpnt_table_inv_mass_lookup        (const u32 sparse_index, cmpnt_inv_mass& inv_mass);

    IFB_INTERNAL void        cmpnt_table_drag_validate          (void);
    IFB_INTERNAL void        cmpnt_table_drag_update            (const cmpnt_drag& drag);
    IFB_INTERNAL void        cmpnt_table_drag_update            (const u32 sparse_index, const drag& d);
    IFB_INTERNAL void        cmpnt_table_drag_lookup            (const u32 sparse_index, drag&       d);
    IFB_INTERNAL void        cmpnt_table_drag_lookup            (const u32 sparse_index, cmpnt_drag& d);

    IFB_INTERNAL void        cmpnt_table_term_velocity_validate (void);
    IFB_INTERNAL void        cmpnt_table_term_velocity_update   (const cmpnt_term_velocity& tv);
    IFB_INTERNAL void        cmpnt_table_term_velocity_update   (const u32 sparse_index, const term_velocity_3d& tv);
    IFB_INTERNAL void        cmpnt_table_term_velocity_lookup   (const u32 sparse_index, term_velocity_3d&       tv);
    IFB_INTERNAL void        cmpnt_table_term_velocity_lookup   (const u32 sparse_index, cmpnt_term_velocity& tv);

    //--------------------------------------------------------------------
    // COMPONENT MANAGER
    //--------------------------------------------------------------------

    struct cmpnt_mngr {
        stack mem;
        u32   capacity;
        struct  {
            cmpnt_table_position*      position;
            cmpnt_table_color*         color;
            cmpnt_table_quad*          quad;
            cmpnt_table_rigid_body*    rigid_body;
            cmpnt_table_velocity*      velocity;
            cmpnt_table_acceleration*  acceleration;
            cmpnt_table_inv_mass*      inv_mass;
            cmpnt_table_drag*          drag;
            cmpnt_table_term_velocity* term_velocity;
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

    struct cmpnt_table_inv_mass {
        f32* normal_val;
    };

    struct cmpnt_table_drag {
        f32* normal_val;
    };

    struct cmpnt_table_term_velocity {
        f32* x;
        f32* y;
        f32* z;
    };
};

#endif //COMPONENTS_HPP
