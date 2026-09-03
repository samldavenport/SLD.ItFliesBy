#ifndef IFB_ENG_INTERNAL_HPP
#define IFB_ENG_INTERNAL_HPP

#include "ifb-engine.hpp"
#include "ifb-input.hpp"
#include "ifb.hpp"
#include "files.hpp"
#include "renderer.hpp"
#include "entity.hpp"
#include "memory.hpp"
#include "component.hpp"
#include "quad.hpp"
#include "physics.hpp"
#include "map.hpp"

#define IFB_ENG_API_FUNC
#define IFB_ENG_INTERNAL static

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct keyboard_input;
    struct eng_context;
    struct global_stack;

    struct eng_context {
        f32                seconds_per_frame;
        const eng_mem_map* mem_map;
        eng_game_proc      game_callback;
        eng_render_proc    render_callback;
        eng_game_context*  game_ctx;
        eng_system_info*   system;
        keyboard_input*    keyboard;
        renderer_context*  renderer;
        file_mngr*         file_mngr;
        entity_mngr*       entity_mngr;
        memory_mngr*       memory_mngr;
        cmpnt_mngr*        cmpnt_mngr;
        quad_mngr*         quad_mngr;
        physics_mngr*      phys_mngr;
        map_mngr*         map_mngr;
    } static * _eng_context;

    struct global_stack {
        u32 size;
        u32 position;
    } static * _global_stack;

    IFB_ENG_INTERNAL keyboard_input* keyboard_input_create   (void);
    IFB_ENG_INTERNAL void            keyboard_input_validate (void);
    IFB_ENG_INTERNAL void            keyboard_input_reset    (void);
    IFB_ENG_INTERNAL void            keyboard_set_key_up     (const input_keycode kc);
    IFB_ENG_INTERNAL void            keyboard_set_key_down   (const input_keycode kc);
    IFB_ENG_INTERNAL bool            keyboard_is_key_down    (const input_keycode kc);

    IFB_ENG_INTERNAL void  global_stack_create_and_init (const eng_mem_map* mem_map);
    IFB_ENG_INTERNAL void  global_stack_validate        (void);
    IFB_ENG_INTERNAL void* global_alloc                 (const u32 size);
    template<typename t>
    IFB_ENG_INTERNAL t*    global_alloc                 (const u32 count = 1);

    IFB_ENG_INTERNAL void eng_system_update_time      (void);
    IFB_ENG_INTERNAL f32  eng_system_get_delta_time_s (void);
};

#endif //IFB_ENG_INTERNAL_HPP
