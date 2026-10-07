#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb-types.hpp"
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
    
    IFB_INTERNAL void        cmpnt_lookup_position         (const u32 sparse_index, cmpnt_position&         position);
    IFB_INTERNAL void        cmpnt_lookup_color            (const u32 sparse_index, cmpnt_color&            color);
    IFB_INTERNAL void        cmpnt_lookup_quad             (const u32 sparse_index, cmpnt_quad&             quad);
    IFB_INTERNAL void        cmpnt_lookup_rigid_body       (const u32 sparse_index, cmpnt_rigid_body&       rigid_body);
    IFB_INTERNAL void        cmpnt_lookup_velocity         (const u32 sparse_index, cmpnt_velocity&         velocity);
    IFB_INTERNAL void        cmpnt_lookup_acceleration     (const u32 sparse_index, cmpnt_acceleration&     acceleration);
    IFB_INTERNAL void        cmpnt_lookup_inv_mass         (const u32 sparse_index, cmpnt_inv_mass&         inv_mass);
    IFB_INTERNAL void        cmpnt_lookup_drag             (const u32 sparse_index, cmpnt_drag&             drag);
    IFB_INTERNAL void        cmpnt_lookup_term_velocity    (const u32 sparse_index, cmpnt_term_velocity&    term_velocity);
    IFB_INTERNAL void        cmpnt_lookup_spring           (const u32 sparse_index, cmpnt_spring&           spring);
    IFB_INTERNAL void        cmpnt_lookup_map_coords       (const u32 sparse_index, cmpnt_map_coords&       map_coords);    
    IFB_INTERNAL void        cmpnt_lookup_particle_emitter (const u32 sparse_index, cmpnt_particle_emitter& particle_emitter);    

    IFB_INTERNAL void        cmpnt_update_position         (const u32 sparse_index, const cmpnt_position&         position);
    IFB_INTERNAL void        cmpnt_update_color            (const u32 sparse_index, const cmpnt_color&            color);
    IFB_INTERNAL void        cmpnt_update_quad             (const u32 sparse_index, const cmpnt_quad&             quad);
    IFB_INTERNAL void        cmpnt_update_rigid_body       (const u32 sparse_index, const cmpnt_rigid_body&       rigid_body);
    IFB_INTERNAL void        cmpnt_update_velocity         (const u32 sparse_index, const cmpnt_velocity&         velocity);
    IFB_INTERNAL void        cmpnt_update_acceleration     (const u32 sparse_index, const cmpnt_acceleration&     acceleration);
    IFB_INTERNAL void        cmpnt_update_inv_mass         (const u32 sparse_index, const cmpnt_inv_mass&         inv_mass);
    IFB_INTERNAL void        cmpnt_update_drag             (const u32 sparse_index, const cmpnt_drag&             drag);
    IFB_INTERNAL void        cmpnt_update_term_velocity    (const u32 sparse_index, const cmpnt_term_velocity&    term_velocity);
    IFB_INTERNAL void        cmpnt_update_spring           (const u32 sparse_index, const cmpnt_spring&           spring);
    IFB_INTERNAL void        cmpnt_update_map_coords       (const u32 sparse_index, const cmpnt_map_coords&       map_coords);
    IFB_INTERNAL void        cmpnt_update_particle_emitter (const u32 sparse_index, const cmpnt_particle_emitter& particle_emitter);
};

#endif //COMPONENTS_HPP
