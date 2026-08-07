#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb-types.hpp"
#include "collections-internal.hpp"

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
    struct cmpnt_table_spring;

    using cmpnt_tbl_position      = component_table<position_3d>; 
    using cmpnt_tbl_color         = component_table<color_rgba_u32>; 
    using cmpnt_tbl_quad          = component_table<quad>; 
    using cmpnt_tbl_rigid_body    = component_table<rigid_body>; 
    using cmpnt_tbl_velocity      = component_table<velocity_3d>; 
    using cmpnt_tbl_acceleration  = component_table<acceleration_3d>; 
    using cmpnt_tbl_inv_mass      = component_table<inv_mass>; 
    using cmpnt_tbl_drag          = component_table<drag>; 
    using cmpnt_tbl_term_velocity = component_table<term_velocity_3d>; 
    using cmpnt_tbl_spring        = component_table<spring>; 

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static cmpnt_mngr* _cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr* cmpnt_mngr_create          (void);
    IFB_INTERNAL void        cmpnt_mngr_startup         (memory& mem_res);
    
    IFB_INTERNAL void        cmpnt_lookup_position      (const u32 sparse_index, position_3d&      pos);
    IFB_INTERNAL void        cmpnt_lookup_color         (const u32 sparse_index, color_rgba_u32&   clr);
    IFB_INTERNAL void        cmpnt_lookup_quad          (const u32 sparse_index, quad&    q);
    IFB_INTERNAL void        cmpnt_lookup_rigid_body    (const u32 sparse_index, rigid_body&       rb);
    IFB_INTERNAL void        cmpnt_lookup_velocity      (const u32 sparse_index, velocity_3d&      vel);
    IFB_INTERNAL void        cmpnt_lookup_acceleration  (const u32 sparse_index, acceleration_3d&  acc);
    IFB_INTERNAL void        cmpnt_lookup_inv_mass      (const u32 sparse_index, inv_mass&         im);
    IFB_INTERNAL void        cmpnt_lookup_drag          (const u32 sparse_index, drag&             d);
    IFB_INTERNAL void        cmpnt_lookup_term_velocity (const u32 sparse_index, term_velocity_3d& tv);
    IFB_INTERNAL void        cmpnt_lookup_spring        (const u32 sparse_index, spring&           s);

    IFB_INTERNAL void        cmpnt_update_position      (const u32 sparse_index, const position_3d&      pos);
    IFB_INTERNAL void        cmpnt_update_color         (const u32 sparse_index, const color_rgba_u32&   clr);
    IFB_INTERNAL void        cmpnt_update_quad          (const u32 sparse_index, const quad& q);
    IFB_INTERNAL void        cmpnt_update_rigid_body    (const u32 sparse_index, const rigid_body&       rb);
    IFB_INTERNAL void        cmpnt_update_velocity      (const u32 sparse_index, const velocity_3d&      vel);
    IFB_INTERNAL void        cmpnt_update_acceleration  (const u32 sparse_index, const acceleration_3d&  acc);
    IFB_INTERNAL void        cmpnt_update_inv_mass      (const u32 sparse_index, const inv_mass&         im);
    IFB_INTERNAL void        cmpnt_update_drag          (const u32 sparse_index, const drag&             d);
    IFB_INTERNAL void        cmpnt_update_term_velocity (const u32 sparse_index, const term_velocity_3d& tv);
    IFB_INTERNAL void        cmpnt_update_spring        (const u32 sparse_index, const spring&           s);

    //--------------------------------------------------------------------
    // COMPONENT MANAGER
    //--------------------------------------------------------------------

    struct cmpnt_mngr {
        stack mem;
        u32   capacity;
        struct {
            cmpnt_tbl_position      position;
            cmpnt_tbl_color         color;
            cmpnt_tbl_quad          quad;
            cmpnt_tbl_rigid_body    rigid_body;
            cmpnt_tbl_velocity      velocity;
            cmpnt_tbl_acceleration  acceleration;
            cmpnt_tbl_inv_mass      inv_mass;
            cmpnt_tbl_drag          drag;
            cmpnt_tbl_term_velocity term_velocity;
            cmpnt_tbl_spring        spring;
        } tbl;
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

    struct cmpnt_table_spring {
        entity_id* anchor;
        f32*       stiffness;
        f32*       damping;
        f32*       rest_length;
    };
};

#endif //COMPONENTS_HPP
