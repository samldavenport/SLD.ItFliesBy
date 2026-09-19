#ifndef PHYSICS_INTERNAL_HPP
#define PHYSICS_INTERNAL_HPP

#include "physics.hpp"
#include "ifb-types.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPES 
    //--------------------------------------------------------------------

    struct physics_memory;
    struct physics_world;
    struct physics_accumulator;
    struct physics_force_list;    
    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------

    static physics_mngr* _phys_mngr;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------
   
    IFB_INTERNAL void*                physics_mngr_res_alloc          (const u32 size_min);

    IFB_INTERNAL physics_accumulator* physics_accumulator_init        (reservation* res);
    IFB_INTERNAL void                 physics_accumulator_validate    (physics_accumulator* const accum);
    IFB_INTERNAL void                 physics_accumulator_add         (physics_accumulator* const accum, const entity_id id, const vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_lookup      (physics_accumulator* const accum, const entity_id id, vec3& v);
    IFB_INTERNAL bool                 physics_accumulator_remove      (physics_accumulator* const accum, const entity_id id);
    IFB_INTERNAL void                 physics_accumulator_reset       (physics_accumulator* const accum);

    IFB_INTERNAL physics_force_list*  physics_force_list_create    (void);
    IFB_INTERNAL bool                 physics_force_list_add       (physics_force_list* list, const entity_id eid, const vec3& v3_force);
    IFB_INTERNAL bool                 physics_force_list_lookup    (physics_force_list* list, const entity_id eid, vec3&       v3_force);
    IFB_INTERNAL bool                 physics_force_list_remove    (physics_force_list* list, const entity_id eid);
    IFB_INTERNAL void                 physics_force_list_reset     (physics_force_list* list);
    IFB_INTERNAL u32                  physics_force_list_index_of  (const physics_force_list* list, const u32 entity_id);
    IFB_INTERNAL entity_id            physics_force_list_get_id    (const physics_force_list* list, const u32 index);
    IFB_INTERNAL vec3&                physics_force_list_get_force (const physics_force_list* list, const u32 index);
    IFB_INTERNAL u32                  physics_force_list_get_count (const physics_force_list* list, const u32 index);

    IFB_INTERNAL void                 physics_integrate_forces        (const f32 dt, const hnd_arena arena_hnd);
    IFB_INTERNAL void                 physics_spring_calculate_forces (const hnd_arena arena_hnd);


    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct physics_memory {
        reservation*     res;
        block_allocator  world_allocator;
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

    struct physics_accumulator {
        physics_force_list* global_forces;
    };


};

#endif //PHYSICS_INTERNAL_HPP
