#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "sld.hpp"

namespace ifb {

    inline void game_pause_menu_dlg (game_context* ctx);

    IFB_GAME void
    game_pause_menu(
        game_context* ctx) {

        if (!ctx->is_paused) return;
        game_pause_menu_dlg(ctx); 
    }
    
    inline void
    game_pause_menu_dlg(
        game_context* ctx) {

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

        color_rgba_f32 pause_dlg_color(0x212121C8);

        bool did_draw;
        
        did_draw = eng_draw_rect_screen(pause_dlg_rect, pause_dlg_color);
        assert(did_draw); 
        
        const cchar pause_text_cstr[] = "GAME PAUSED";
        const u32   pause_text_size   = sizeof(pause_text_cstr);
        vec2 pause_text_pos;
        const f32 text_w = eng_font_measure_text(ctx->fnt, pause_text_cstr, pause_text_size);
        pause_text_pos.x = ((f32)screen_width  - text_w) * 0.5f;
        pause_text_pos.y = ((f32)screen_height - ctx->fnt->line_height) * 0.5f;


        const color_rgba_f32 pause_text_color(0xFFFFFFFF);

        eng_draw_text_screen(
            ctx->fnt,
            pause_text_cstr,
            pause_text_size,
            pause_text_pos,
            pause_text_color
        );
    }
};
