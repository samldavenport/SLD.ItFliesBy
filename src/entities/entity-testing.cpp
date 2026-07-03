#pragma once

#include "entities.hpp"

namespace ifb {

    inline void create_entities (void);
    inline void test_lookups    (entity_list* list);


    IFB_INTERNAL void
    entity_test(
        void) {

        // allocate arena 
        arena* a = arena_alloc();
        assert(a);
        entity_list* id_list = entity_list_arena_create(a);


        create_entities ();
        test_lookups    (id_list);


        arena_free(a);
    }

    inline void
    create_entities(
        void) {

        // archetypes
        const entity_archetype atype_0 = entity_archetype(component_type_e_position);
        const entity_archetype atype_1 = entity_archetype(component_type_e_color);
        const entity_archetype atype_2 = entity_archetype(component_type_e_position | component_type_e_color );

        // entities
        const entity_id entity_0 = entity_create("TEST-0", atype_0);
        const entity_id entity_1 = entity_create("TEST-1", atype_1);
        const entity_id entity_2 = entity_create("TEST-2", atype_0);
        const entity_id entity_3 = entity_create("TEST-3", atype_1);
        const entity_id entity_4 = entity_create("TEST-4", atype_0);
        const entity_id entity_5 = entity_create("TEST-5", atype_1);
        const entity_id entity_6 = entity_create("TEST-6", atype_0);
        const entity_id entity_7 = entity_create("TEST-7", atype_1);
        const entity_id entity_8 = entity_create("TEST-8", atype_0);
        const entity_id entity_9 = entity_create("TEST-9", atype_2);
    }

    inline void
    test_lookups(
        entity_list* list) {

        const bool did_find_colored_entities =  entity_lookup_by_archetype(list, component_type_e_color);
        assert(did_find_colored_entities && list->count == 4);
    }
};