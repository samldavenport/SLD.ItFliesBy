#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_GAME void
    game_pause_menu(
        bool& is_paused) {

        if (!is_paused) return;
        
        u32 screen_width;
        u32 screen_height;
        eng_draw_get_screen_size(
            screen_width,
            screen_height
        );
    
        rect pause_dlg_rect;
        pause_dlg_rect.width  = (f32)screen_width;
        pause_dlg_rect.height = (f32)screen_height;
        pause_dlg_rect.x      = 0.0f;
        pause_dlg_rect.y      = 0.0f;

        color_rgba_f32 pause_dlg_color(0x212121FF);

        bool did_draw;
        
        did_draw = eng_draw_rect_screen(pause_dlg_rect, pause_dlg_color);
        assert(did_draw); 
    }
};
