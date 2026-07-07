#pragma once

#include "components.hpp"

namespace ifb {

    IFB_INTERNAL component_list_quad*     component_quad_list_create       (arena* a);
    IFB_INTERNAL void                     component_quad_list_validate     (const component_list_quad* list_quad);
    IFB_INTERNAL void                     component_quad_table_update      (const component_list_quad* list_quad);
    IFB_INTERNAL void                     component_quad_table_lookup      (component_list_quad*       list_quad, const entity_list* list_entity);

};