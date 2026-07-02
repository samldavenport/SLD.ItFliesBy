#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"
#include "entities.hpp"
#include "memory.hpp"
namespace ifb {

    struct component_manager;
    struct component_table_position;
    struct component_table_color;
    struct component_table_memory;

    u32                component_manager_memory_size (void);
    component_manager* component_manager_create      (memory& mem_stack);
    void               component_manager_startup     (memory& mem_res);


    struct component_manager {
        struct  {
            component_table_position* position;
            component_table_color*    color; 
        } tables;
        component_table_memory* memory;
    };

    struct component_table_memory {
        block_allocator* block_alctr;
    };

    struct component_table_position {
        f32* x;
        f32* y;
        f32* z;
    };

    struct component_table_color {
        u32* rgba_hex;
    };
};

#endif //COMPONENTS_HPP