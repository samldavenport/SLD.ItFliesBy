#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "ifb-component.hpp"
#include "ifb-types.hpp"
#include "memory.hpp"
#include "ifb-collections.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------

    struct physics_mngr;
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
    IFB_INTERNAL physics_mngr*        physics_mngr_create             (void);
    IFB_INTERNAL void                 physics_mngr_validate           (void);
    IFB_INTERNAL void                 physics_mngr_startup            (memory& memory);
    IFB_INTERNAL void                 physics_mngr_shutdown           (void);
    IFB_INTERNAL void                 physics_mngr_simulate           (const f32 dt);

    IFB_INTERNAL physics_accumulator* physics_accumulator_init        (stack& s);
    IFB_INTERNAL void                 physics_accumulator_validate    (physics_accumulator* const accum);
    IFB_INTERNAL void                 physics_accumulator_add         (physics_accumulator* const accum, const entity_id id, const vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_lookup      (physics_accumulator* const accum, const entity_id id, vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_remove      (physics_accumulator* const accum, const entity_id id);
    IFB_INTERNAL void                 physics_accumulator_reset       (physics_accumulator* const accum);

    IFB_INTERNAL void                 physics_entity_add_force        (const entity_id id, const vec3& force);
    IFB_INTERNAL bool                 physics_entity_remove_all_force (const entity_id id);
    IFB_INTERNAL void                 physics_entity_set_mass         (const entity_id id, const f32 mass);
    IFB_INTERNAL void                 physics_entity_set_inv_mass     (const entity_id id, const f32 inv_mass);
    IFB_INTERNAL void                 physics_entity_set_rigid_body   (const entity_id id, const rigid_body& rb);

    IFB_INTERNAL void                 physics_integrate_forces        (const f32 dt, const arena_handle arena_hnd);

    IFB_INTERNAL void                 physics_spring_calculate_forces (const arena_handle arena_hnd);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct physics_memory {
        stack            stack;
        block_allocator  world_allocator;
        arena_handle simulation_arena;
    };

    struct physics_entity : entity {
        rigid_body      rb;
        position        pos;
        velocity_3d     vel;
        acceleration_3d acc;
        inv_mass        im;
        drag            drag;
    };

    struct physics_mngr {
        physics_memory*      memory;
        physics_accumulator* force_accumulator;
        entity_list          static_entities;
        entity_list          dynamic_entities;
        u32                  delta_time_ms;
    };
};

#endif //PHYSICS_HPP
