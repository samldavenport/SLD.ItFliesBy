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
    IFB_INTERNAL void        cmpnt_lookup_color         (const u32 sparse_index, cmpnt_color&         clr);
    IFB_INTERNAL void        cmpnt_lookup_quad          (const u32 sparse_index, cmpnt_quad&          q);
    IFB_INTERNAL void        cmpnt_lookup_rigid_body    (const u32 sparse_index, cmpnt_rigid_body&    rb);
    IFB_INTERNAL void        cmpnt_lookup_velocity      (const u32 sparse_index, cmpnt_velocity&      vel);
    IFB_INTERNAL void        cmpnt_lookup_acceleration  (const u32 sparse_index, cmpnt_acceleration&  acc);
    IFB_INTERNAL void        cmpnt_lookup_inv_mass      (const u32 sparse_index, cmpnt_inv_mass&      im);
    IFB_INTERNAL void        cmpnt_lookup_drag          (const u32 sparse_index, cmpnt_drag&          d);
    IFB_INTERNAL void        cmpnt_lookup_term_velocity (const u32 sparse_index, cmpnt_term_velocity& tv);
    IFB_INTERNAL void        cmpnt_lookup_spring        (const u32 sparse_index, cmpnt_spring&        s);
    IFB_INTERNAL void        cmpnt_lookup_map_coords    (const u32 sparse_index, cmpnt_map_coords&    mc);    

    IFB_INTERNAL void        cmpnt_update_position      (const u32 sparse_index, const cmpnt_position&      pos);
    IFB_INTERNAL void        cmpnt_update_color         (const u32 sparse_index, const cmpnt_color&         clr);
    IFB_INTERNAL void        cmpnt_update_quad          (const u32 sparse_index, const cmpnt_quad&          q);
    IFB_INTERNAL void        cmpnt_update_rigid_body    (const u32 sparse_index, const cmpnt_rigid_body&    rb);
    IFB_INTERNAL void        cmpnt_update_velocity      (const u32 sparse_index, const cmpnt_velocity&      vel);
    IFB_INTERNAL void        cmpnt_update_acceleration  (const u32 sparse_index, const cmpnt_acceleration&  acc);
    IFB_INTERNAL void        cmpnt_update_inv_mass      (const u32 sparse_index, const cmpnt_inv_mass&      im);
    IFB_INTERNAL void        cmpnt_update_drag          (const u32 sparse_index, const cmpnt_drag&          d);
    IFB_INTERNAL void        cmpnt_update_term_velocity (const u32 sparse_index, const cmpnt_term_velocity& tv);
    IFB_INTERNAL void        cmpnt_update_spring        (const u32 sparse_index, const cmpnt_spring&        s);
    IFB_INTERNAL void        cmpnt_update_map_coords    (const u32 sparse_index, const cmpnt_map_coords&    mc);
};

#endif //COMPONENTS_HPP
