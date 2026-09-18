#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb-types.hpp"
#include "ifb-collections.hpp"
#include "memory.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct cmpnt_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr* cmpnt_mngr_create          (void);
    IFB_INTERNAL void        cmpnt_mngr_startup         (reservation* res);
    
    IFB_INTERNAL void        cmpnt_lookup_position      (const u32 sparse_index, cmpnt_position&      pos);
    IFB_INTERNAL void        cmpnt_lookup_color         (const u32 sparse_index, color_rgba_u32&   clr);
    IFB_INTERNAL void        cmpnt_lookup_quad          (const u32 sparse_index, quad&             q);
    IFB_INTERNAL void        cmpnt_lookup_rigid_body    (const u32 sparse_index, rigid_body&       rb);
    IFB_INTERNAL void        cmpnt_lookup_velocity      (const u32 sparse_index, cmpnt_velocity&      vel);
    IFB_INTERNAL void        cmpnt_lookup_acceleration  (const u32 sparse_index, cmpnt_acceleration&  acc);
    IFB_INTERNAL void        cmpnt_lookup_inv_mass      (const u32 sparse_index, inv_mass&         im);
    IFB_INTERNAL void        cmpnt_lookup_drag          (const u32 sparse_index, drag&             d);
    IFB_INTERNAL void        cmpnt_lookup_term_velocity (const u32 sparse_index, term_cmpnt_velocity& tv);
    IFB_INTERNAL void        cmpnt_lookup_spring        (const u32 sparse_index, spring&           s);
    IFB_INTERNAL void        cmpnt_lookup_map_coords    (const u32 sparse_index, map_coords&       mc);    

    IFB_INTERNAL void        cmpnt_update_position      (const u32 sparse_index, const cmpnt_position&      pos);
    IFB_INTERNAL void        cmpnt_update_color         (const u32 sparse_index, const color_rgba_u32&   clr);
    IFB_INTERNAL void        cmpnt_update_quad          (const u32 sparse_index, const quad&             q);
    IFB_INTERNAL void        cmpnt_update_rigid_body    (const u32 sparse_index, const rigid_body&       rb);
    IFB_INTERNAL void        cmpnt_update_velocity      (const u32 sparse_index, const cmpnt_velocity&      vel);
    IFB_INTERNAL void        cmpnt_update_acceleration  (const u32 sparse_index, const cmpnt_acceleration&  acc);
    IFB_INTERNAL void        cmpnt_update_inv_mass      (const u32 sparse_index, const inv_mass&         im);
    IFB_INTERNAL void        cmpnt_update_drag          (const u32 sparse_index, const drag&             d);
    IFB_INTERNAL void        cmpnt_update_term_velocity (const u32 sparse_index, const term_cmpnt_velocity& tv);
    IFB_INTERNAL void        cmpnt_update_spring        (const u32 sparse_index, const spring&           s);
    IFB_INTERNAL void        cmpnt_update_map_coords    (const u32 sparse_index, const map_coords&       mc);

};

#endif //COMPONENTS_HPP
