#ifndef IFB_ENG_INTERNAL_HPP
#define IFB_ENG_INTERNAL_HPP

#include "ifb.hpp"
#include "files.hpp"
#include "renderer.hpp"

#define IFB_ENG_API_FUNC
#define IFB_ENG_INTERNAL static

namespace ifb {

    struct eng_context;
    struct eng_stack;


    struct eng_context {
        const eng_mem_map* mem_map;
        eng_stack*         stack;
        eng_system_info*   system;
        file_manager*      file_mngr;
        renderer_context*  renderer;
    } static * _eng_context;

    struct eng_stack {
        u32 size;
        u32 position;
    };

    eng_stack*        eng_stack_init                       (const eng_mem_map* mem_map);
    byte*             eng_stack_push_data                  (eng_stack* eng_stack, const u32 size);
    eng_context*      eng_stack_push_context               (eng_stack* eng_stack);
    eng_system_info*  eng_stack_push_system_info           (eng_stack* eng_stack);
    file_manager*     eng_stack_push_and_init_file_manager (eng_stack* eng_stack, const u32 file_count_max);
    renderer_context* eng_stack_push_and_init_renderer     (eng_stack* eng_stack);
};

#endif //IFB_ENG_INTERNAL_HPP