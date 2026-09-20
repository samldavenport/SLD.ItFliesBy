#ifndef PHYSICS_INTERNAL_HPP
#define PHYSICS_INTERNAL_HPP

#include "physics.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------
    
    struct physics_memory;
    struct physics_world;
    struct physics_force_accumulator;
    
    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------

    static physics_mngr* _phys_mngr;

    //--------------------------------------------------------------------
    // METHOD DECLARATIONS
    //--------------------------------------------------------------------
   
    IFB_INTERNAL void*                      physics_mngr_res_alloc             (const u32 size_min);
    IFB_INTERNAL physics_force_accumulator* physics_mngr_get_force_accumulator (void);

    IFB_INTERNAL physics_force_accumulator* physics_force_accumulator_create      (reservation* res);
    IFB_INTERNAL void                 physics_force_accumulator_validate    (physics_force_accumulator* const accum);
    IFB_INTERNAL void                 physics_force_accumulator_add         (physics_force_accumulator* const accum, const entity_id id, const vec3& v);
    IFB_INTERNAL bool                 physics_force_accumulator_lookup      (physics_force_accumulator* const accum, const entity_id id, vec3& v);
    IFB_INTERNAL bool                 physics_force_accumulator_remove      (physics_force_accumulator* const accum, const entity_id id);
    IFB_INTERNAL void                 physics_force_accumulator_reset       (physics_force_accumulator* const accum);
    
    IFB_INTERNAL void                 physics_integrate_forces        (physics_force_accumulator* const accum, const f32 dt, const hnd_arena arena_hnd);

    IFB_INTERNAL void                 physics_spring_calculate_forces (const hnd_arena arena_hnd);
    
    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
};

#endif //PHYSICS_INTERNAL_HPP
