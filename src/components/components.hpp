#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "ifb.hpp"

namespace ifb {

    struct component_manager;

    u32                component_manager_memory_size (void);
    component_manager* component_manager_create      (memory& mem_stack);
    void               component_manager_startup     (memory& mem_res);

};

#endif //COMPONENTS_HPP