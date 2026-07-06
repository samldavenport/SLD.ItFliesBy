#ifndef IFB_ENG_INTERNAL_HPP
#define IFB_ENG_INTERNAL_HPP

#include "ifb.hpp"
#include "files.hpp"
#include "renderer.hpp"
#include "entities.hpp"
#include "gui.hpp"
#include "memory.hpp"

#define IFB_ENG_API_FUNC
#define IFB_ENG_INTERNAL static

namespace ifb {

    struct eng_context;

    struct eng_context {
        const eng_mem_map* mem_map;
        eng_system_info*   system;
        renderer_context*  renderer;
        file_mngr*         file_mngr;
        entity_mngr*       entity_mngr;
        memory_mngr*       memory_mngr;
    } static * _eng_context;

    struct global_stack {
        u32 size;
        u32 position;
    } static * _global_stack;

    IFB_ENG_INTERNAL void  global_stack_create_and_init (const eng_mem_map* mem_map);
    IFB_ENG_INTERNAL void  global_stack_validate        (void);
    IFB_ENG_INTERNAL void* global_alloc                 (const u32 size);
    
    template<typename t>
    IFB_ENG_INTERNAL t* global_alloc(const u32 count = 1);
};

#endif //IFB_ENG_INTERNAL_HPP