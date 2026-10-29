#ifndef IFB_ENG_INTERNAL_HPP
#define IFB_ENG_INTERNAL_HPP

#include "ifb-engine.hpp"
#include "ifb-input.hpp"
#include "ifb-files.hpp"
#include "renderer-internal.hpp"
#include "ifb-memory.hpp"
#include "component.hpp"
#include "physics-internal.hpp"
#include "ifb-map.hpp"
#include "ifb.hpp"
#include "random-internal.hpp"
#include "sprites.hpp"
#include "entity-internal.hpp"
#include "file-internal.hpp"
#include "eng-input.hpp"
#include "map-internal.hpp"
#include "memory-internal.hpp"
#include "particle-internal.hpp"

#define IFB_ENG_API_FUNC
#define IFB_ENG_INTERNAL static

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS 
    //--------------------------------------------------------------------
    
    struct keyboard_input;
    struct asset_mngr;
    struct eng_context;
    struct global_stack;

    struct eng_context {
        f32                seconds_per_frame;
        u64                frame_count;
        const eng_mem_map* mem_map;
        fptr_eng_game      game_callback;
        fptr_eng_render    render_callback;
        eng_game_context*  game_ctx;
        eng_system_info*   system;
        keyboard_input*    keyboard;
        renderer*          renderer;
        file_mngr*         file_mngr;
        entity_mngr*       entity_mngr;
        memory_mngr*       memory_mngr;
        cmpnt_mngr*        cmpnt_mngr;
        phys_mngr*         phys_mngr;
        map_mngr*          map_mngr;
        particle_mngr*     particle_mngr;
        random_table*      random_table;
        asset_mngr*        asset_mngr;
        sprite_mngr*       sprite_mngr;
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

    IFB_ENG_INTERNAL u64   eng_context_get_frame_count  (void);

    IFB_ENG_INTERNAL void  global_stack_create_and_init (const eng_mem_map* mem_map);
    IFB_ENG_INTERNAL void  global_stack_validate        (void);
    IFB_ENG_INTERNAL void* global_alloc                 (const u32 size);
    template<typename t>
    IFB_ENG_INTERNAL t*    global_alloc                 (const u32 count = 1);

    IFB_ENG_INTERNAL void system_refresh_info                (void);
    IFB_ENG_INTERNAL void system_update_time                 (void);
    IFB_ENG_INTERNAL f32  system_get_delta_time_s            (void);
    IFB_ENG_INTERNAL u32  system_get_memory_page_size        (void);
    IFB_ENG_INTERNAL u32  system_get_memory_granularity      (void);
    IFB_ENG_INTERNAL u32  system_align_to_memory_page_size   (const u32 size);
    IFB_ENG_INTERNAL u32  system_align_to_memory_granularity (const u32 size);
    IFB_ENG_INTERNAL u32  system_get_primary_monitor_width   (void);
    IFB_ENG_INTERNAL u32  system_get_primary_monitor_height  (void);
};

#endif //IFB_ENG_INTERNAL_HPP
