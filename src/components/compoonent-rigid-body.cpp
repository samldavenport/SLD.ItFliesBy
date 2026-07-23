#pragma once

#include "component.hpp"

namespace ifb {

    IFB_INTERNAL cmpnt_list_rigid_body*
    cmpnt_rigid_body_list_create(
        arena* a) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_list_validate(
        const cmpnt_list_rigid_body* list_rb) {

    }

    IFB_INTERNAL bool
    cmpnt_rigid_body_list_add(
        cmpnt_list_rigid_body*  list_rb,
        const cmpnt_rigid_body& rb) {

    }
     
    IFB_INTERNAL void
    cmpnt_rigid_body_table_validate(
        void) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_update(
        const cmpnt_rigid_body& rb) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_update(
        rigid_body& rb,
        const u32   sparse_index) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_update(
        const cmpnt_list_rigid_body* list_rb) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_lookup(
        cmpnt_list_rigid_body* list_rb,
        const entity_list*     list_entity) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_lookup(
        cmpnt_rigid_body& rb,
        const entity&     e) {

    }

    IFB_INTERNAL void
    cmpnt_rigid_body_table_lookup(
        rigid_body& rb,
        const u32   sparse_index) {
            
    }
};