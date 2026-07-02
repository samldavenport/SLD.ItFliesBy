#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"
#include "entities.hpp"
namespace ifb {

    struct component_manager;
    struct component_table_base;
    struct component_table_position;
    struct component_table_color;

    u32                component_manager_memory_size (void);
    component_manager* component_manager_create      (memory& mem_stack);
    void               component_manager_startup     (memory& mem_res);


    struct component_manager {
        struct  {
            component_table_position* position;
            component_table_color*    color; 
        } tables;
    };

    struct component_table_base {
        entity_sparse_set* entity_id;
    };

    struct component_table_position : component_table_base {
        f32* x;
        f32* y;
        f32* z;
    };

    struct component_table_color : component_table_base{
        u32* rgba_hex;
    };
};

#endif //COMPONENTS_HPP