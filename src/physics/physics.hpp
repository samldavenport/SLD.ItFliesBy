#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "ifb-component.hpp"
#include "ifb-types.hpp"
#include "ifb.hpp"
#include "memory.hpp"
#include "entity.hpp"
#include "collections-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static const entity_archetype PHYSICS_ENTITY_ARCHETYPE = (
        cmpnt_type_e_position     |
        cmpnt_type_e_rigid_body   |
        cmpnt_type_e_velocity     |
        cmpnt_type_e_acceleration |
        cmpnt_type_e_inv_mass     |
        cmpnt_type_e_drag     
    );
 
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
    IFB_INTERNAL void                 physics_mngr_simulate           (const u32 dt_ms);

    IFB_INTERNAL physics_accumulator* physics_accumulator_init        (stack& s);
    IFB_INTERNAL void                 physics_accumulator_validate    (physics_accumulator* const accum);
    IFB_INTERNAL void                 physics_accumulator_add         (physics_accumulator* const accum, const entity_id id, const vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_lookup      (physics_accumulator* const accum, const entity_id id, vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_remove      (physics_accumulator* const accum, const entity_id id);
    IFB_INTERNAL void                 physics_accumulator_reset       (physics_accumulator* const accum);


    IFB_INTERNAL void                 physics_entity_add_force        (const entity_id id, const vec3& force);
    IFB_INTERNAL bool                 physics_entity_remove_all_force (const entity_id id);
    IFB_INTERNAL bool                 physics_entity_set_mass         (const entity_id id, const f32 mass);
    IFB_INTERNAL bool                 physics_entity_set_inv_mass     (const entity_id id, const f32 inv_mass);
    IFB_INTERNAL bool                 physics_entity_set_rigid_body   (const entity_id id, const rigid_body& rb);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct physics_memory {
        stack           stack;
        block_allocator world_allocator;
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
        entity_id_list       static_entity_ids;
        entity_id_list       dynamic_entity_ids;
        u32                  delta_time_ms;
    };
};

#endif //PHYSICS_HPP
