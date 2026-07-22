#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "ifb.hpp"

namespace ifb {

    struct physics_manager;
    struct physics_world;
    struct physics_dynamic_entity;
    struct physics_static_entitiy;

    IFB_INTERNAL physics_manager* physics_manager_create   ();
    IFB_INTERNAL void             physics_manager_startup  (memory& memory);
    IFB_INTERNAL void             physics_manager_shutdown ();

    IFB_INTERNAL physics_world*  physics_world_create             ();
    IFB_INTERNAL void            physics_world_destroy            (physics_world* world);
    IFB_INTERNAL void            physics_world_simulate           (physics_world* world, const u32 dt_ms);
    IFB_INTERNAL entity_id       physics_world_add_entity_dynamic (physics_world* world);
    IFB_INTERNAL entity_id       physics_world_add_entity_static  (physics_world* world);

    struct physics_manager {

        physics_world* world_list;
    };

    struct physics_world {
        physics_world* next;
        physics_world* prev;
    };

    struct physics_dynamic_entity : entity {
        
    };

    struct physics_static_entitiy : entity {

    }

};

#endif //PHYSICS_HPP