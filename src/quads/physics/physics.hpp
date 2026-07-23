#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "ifb.hpp"
#include "memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static const entity_archetype PHYSICS_ENTITY_ARCHETYPE_STATIC = (
        cmpnt_type_e_position   |
        cmpnt_type_e_rigid_body
    );

    static const entity_archetype PHYSICS_ENTITY_ARCHETYPE_DYNAMIC = (
        cmpnt_type_e_position     |
        cmpnt_type_e_rigid_body   |
        cmpnt_type_e_velocity     |
        cmpnt_type_e_acceleration
    );
 
    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------

    struct physics_manager;
    struct physics_memory;
    struct physics_world;
    struct physics_dynamic_entity;
    struct physics_static_entitiy;

    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------

    static physics_manager* _phys_mngr;

    //--------------------------------------------------------------------
    // METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL physics_manager* physics_manager_create   (void);
    IFB_INTERNAL void             physics_manager_startup  (memory& memory);
    IFB_INTERNAL void             physics_manager_shutdown ();

    IFB_INTERNAL physics_world*   physics_world_create             (void);
    IFB_INTERNAL void             physics_world_destroy            (physics_world* world);
    IFB_INTERNAL void             physics_world_simulate           (physics_world* world, const u32 dt_ms);
    IFB_INTERNAL entity_id        physics_world_add_entity_dynamic (physics_world* world);
    IFB_INTERNAL entity_id        physics_world_add_entity_static  (physics_world* world);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------

    struct physics_memory {
        stack           stack;
        block_allocator world_allocator;
    };

    struct physics_manager {
        physics_memory* memory;
        physics_world*  world_list;
    };

    struct physics_world {
        physics_world* next;
        physics_world* prev;
    };

    struct physics_dynamic_entity : entity {
        rigid_body      rb;
        position        pos;
        velocity_3d     vel;
        acceleration_3d acc;
    };

    struct physics_static_entitiy : entity {
        rigid_body rb;
        position   pos;
    }

};

#endif //PHYSICS_HPP