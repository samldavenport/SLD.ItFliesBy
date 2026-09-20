#ifndef PHYSICS_INTERNAL_HPP
#define PHYSICS_INTERNAL_HPP

#include "physics.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------
    
    struct physics_memory;
    struct physics_world;
    struct physics_accumulator;
    
    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------

    static physics_mngr* _phys_mngr;

    //--------------------------------------------------------------------
    // METHOD DECLARATIONS
    //--------------------------------------------------------------------
   
    IFB_INTERNAL void*                physics_mngr_res_alloc          (const u32 size_min);

    IFB_INTERNAL physics_accumulator* physics_accumulator_create      (reservation* res);
    IFB_INTERNAL void                 physics_accumulator_validate    (physics_accumulator* const accum);
    IFB_INTERNAL void                 physics_accumulator_add         (physics_accumulator* const accum, const entity_id id, const vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_lookup      (physics_accumulator* const accum, const entity_id id, vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_remove      (physics_accumulator* const accum, const entity_id id);
    IFB_INTERNAL void                 physics_accumulator_reset       (physics_accumulator* const accum);
    
    IFB_INTERNAL void                 physics_integrate_forces        (const f32 dt, const hnd_arena arena_hnd);

    IFB_INTERNAL void                 physics_spring_calculate_forces (const hnd_arena arena_hnd);
    
    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
    struct physics_memory {
        reservation*     res;
        hnd_arena        simulation_arena;
    };

    struct physics_entity : entity {
        cmpnt_rigid_body   rb;
        cmpnt_position     pos;
        cmpnt_velocity     vel;
        cmpnt_acceleration acc;
        cmpnt_inv_mass     im;
        cmpnt_drag         drag;
    };

    struct physics_mngr {
        physics_memory*      memory;
        physics_accumulator* force_accumulator;
        entity_list*         static_entities;
        entity_list*         dynamic_entities;
        u32                  delta_time_ms;
    };
};

#endif //PHYSICS_INTERNAL_HPP
