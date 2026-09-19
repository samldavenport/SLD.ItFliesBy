#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "ifb-types.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------

    struct physics_mngr;

    //--------------------------------------------------------------------
    // METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL physics_mngr*        physics_mngr_create             (void);
    IFB_INTERNAL void                 physics_mngr_validate           (void);
    IFB_INTERNAL void                 physics_mngr_startup            (reservation* res);
    IFB_INTERNAL void                 physics_mngr_shutdown           (void);
    IFB_INTERNAL void                 physics_mngr_simulate           (const f32 dt);

    IFB_INTERNAL void                 physics_entity_add_force        (const entity_id id, const vec3& force);
    IFB_INTERNAL bool                 physics_entity_remove_all_force (const entity_id id);
    IFB_INTERNAL void                 physics_entity_set_mass         (const entity_id id, const f32 mass);
    IFB_INTERNAL void                 physics_entity_set_inv_mass     (const entity_id id, const f32 inv_mass);
    IFB_INTERNAL void                 physics_entity_set_rigid_body   (const entity_id id, const cmpnt_rigid_body& rb);
};

#endif //PHYSICS_HPP
