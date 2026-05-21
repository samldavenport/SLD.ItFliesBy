#ifndef IFB_ENG_INTERNAL_HPP
#define IFB_ENG_INTERNAL_HPP

#include "ifb.hpp"

#define IFB_ENG_API_FUNC
#define IFB_ENG_INTERNAL static

namespace ifb {

    struct eng_stack {
        byte* start;
        u32   size;
        u32   position;
    };

    struct eng_context {
        eng_mem_map*     mem_map;
        eng_stack*       stack;
        eng_system_info* system;
    };


    byte*
    eng_stack_push_data(const u32 size);
    
    template<typename t> t* 
    eng_stack_push_struct(const u32 count = 1);

};

#endif //IFB_ENG_INTERNAL_HPP