#pragma once

#include "entity.hpp"
namespace ifb {

    inline void test_create         (void);
    inline void test_lookups        (entity_list* list);
    inline void test_identical_tags (void);
    inline void test_destroy        (void);
    inline void test_destroy_all    (void);


    IFB_INTERNAL void
    entity_test(
        void) {

        // allocate arena 
        arena* a = memory_mngr_arena_alloc(); 
        assert(a);
        entity_list* id_list = entity_list_create(a);


        test_create         ();
        test_identical_tags ();
        test_lookups        (id_list);
        
        memory_mngr_arena_free(&a);
    }

    inline void
    test_create(
        void) {

        // archetypes
        const entity_archetype atype_0 = (component_type_e_position);
        const entity_archetype atype_1 = (component_type_e_color);
        const entity_archetype atype_2 = (component_type_e_position | component_type_e_color );

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

        assert(
            entity_0 != ENTITY_ID_INVALID &&
            entity_1 != ENTITY_ID_INVALID &&
            entity_2 != ENTITY_ID_INVALID &&
            entity_3 != ENTITY_ID_INVALID &&
            entity_4 != ENTITY_ID_INVALID &&
            entity_5 != ENTITY_ID_INVALID &&
            entity_6 != ENTITY_ID_INVALID &&
            entity_7 != ENTITY_ID_INVALID &&
            entity_8 != ENTITY_ID_INVALID &&
            entity_9 != ENTITY_ID_INVALID &&
            "ENTITY CREATE RETURN INVALID ID"          
        );
    }

    inline void
    test_lookups(
        entity_list* list) {

        entity_query query = {0};
        query.has_any = component_type_e_color;

        const bool did_find_colored_entities =  entity_lookup_by_archetype(list, query);
        assert(did_find_colored_entities && list->count == 4);

        entity entity_8;
        const bool did_find_entity_8 = entity_lookup_by_tag(entity_8, "TEST-8");
        assert(did_find_entity_8 && "ENTITY NOT FOUND AFTER CREATE");
    }

    inline void
    test_identical_tags(
        void) {

        cchar test[] = "TEST";

        const u32 hash_0 = hash_u32(test, sizeof(test));
        const u32 hash_1 = hash_u32(test, sizeof(test));

        assert(hash_0 == hash_1 && "IDENTICAL HASHES ARE DIFFERENT");
    }

    inline void
    test_destroy(
        void) {

        entity       entity_5_before;
        entity       entity_5_after;
        entity       entity_9;
        const cchar* tag_entity_5 = "TEST-5";
        const cchar* tag_entity_9 = "TEST-9";

        const bool does_exist = entity_lookup_by_tag(entity_5_before, tag_entity_5);
        assert(does_exist);

        const bool did_destroy          =  entity_destroy       (tag_entity_5);
        const bool did_destroy_for_real = !entity_lookup_by_tag (entity_5_after, tag_entity_5); 
        assert(
            did_destroy          &&
            did_destroy_for_real &&
            "ENTITY DESTROY FAILED ON VALID ENTITY"
        );

        // we know that entity 9 is the last one
        // so its info should be equal to what entity 5 used to be

        //TODO(SAM): this is the process that is still failing
        const bool swapped_entity_still_exists = entity_lookup_by_tag(entity_9, tag_entity_9);
        bool did_swap = true;
        did_swap &= (strncmp(entity_9.tag, tag_entity_9, ENTITY_TAG_SIZE) == 0);
        did_swap &= (entity_9.index_dense  == entity_5_before.index_dense); 
        assert(did_swap);

    }

    inline void
    test_destroy_all(
        void) {

        const bool did_destroy_entity_0 = entity_destroy("TEST-0");
        const bool did_destroy_entity_1 = entity_destroy("TEST-1");
        const bool did_destroy_entity_2 = entity_destroy("TEST-2");
        const bool did_destroy_entity_3 = entity_destroy("TEST-3");
        const bool did_destroy_entity_4 = entity_destroy("TEST-4");
        const bool did_destroy_entity_6 = entity_destroy("TEST-6");
        const bool did_destroy_entity_7 = entity_destroy("TEST-7");
        const bool did_destroy_entity_8 = entity_destroy("TEST-8");
        const bool did_destroy_entity_9 = entity_destroy("TEST-9");

        assert(
            did_destroy_entity_0 &&
            did_destroy_entity_1 &&
            did_destroy_entity_2 &&
            did_destroy_entity_3 &&
            did_destroy_entity_4 &&
            did_destroy_entity_6 &&
            did_destroy_entity_7 &&
            did_destroy_entity_8 &&
            did_destroy_entity_9            
        );

        assert(_entity_mngr->count == 0);

        entity e;
        const bool did_destroy_entity_0_for_real = entity_lookup_by_tag(e, "TEST-0");
        const bool did_destroy_entity_1_for_real = entity_lookup_by_tag(e, "TEST-1");
        const bool did_destroy_entity_2_for_real = entity_lookup_by_tag(e, "TEST-2");
        const bool did_destroy_entity_3_for_real = entity_lookup_by_tag(e, "TEST-3");
        const bool did_destroy_entity_4_for_real = entity_lookup_by_tag(e, "TEST-4");
        const bool did_destroy_entity_6_for_real = entity_lookup_by_tag(e, "TEST-6");
        const bool did_destroy_entity_7_for_real = entity_lookup_by_tag(e, "TEST-7");
        const bool did_destroy_entity_8_for_real = entity_lookup_by_tag(e, "TEST-8");
        const bool did_destroy_entity_9_for_real = entity_lookup_by_tag(e, "TEST-9");

        assert(
            did_destroy_entity_0_for_real &&
            did_destroy_entity_1_for_real &&
            did_destroy_entity_2_for_real &&
            did_destroy_entity_3_for_real &&
            did_destroy_entity_4_for_real &&
            did_destroy_entity_6_for_real &&
            did_destroy_entity_7_for_real &&
            did_destroy_entity_8_for_real &&
            did_destroy_entity_9_for_real
        );
    }

};