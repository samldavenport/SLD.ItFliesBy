#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "sld.hpp"

namespace ifb {

    static constexpr f32 GAME_PAUSE_MENU_ROW_HEIGHT            = 32.0f;
    static constexpr f32 GAME_PAUSE_MENU_ROW_GAP               = 8.0f;
    static constexpr f32 GAME_PAUSE_MENU_COLUMN_GAP            = 32.0f;
    static constexpr f32 GAME_PAUSE_MENU_COLUMN_WIDTH          = 360.0f;
    static constexpr u32 GAME_PAUSE_MENU_TEXT_COLOR_RGBA       = 0xFFFFFFFF;
    static constexpr u32 GAME_PAUSE_MENU_BACKGROUND_COLOR_RGBA = 0x212121C8;
    inline void game_pause_menu_dlg (game_context* ctx);


    IFB_GAME void
    game_pause_menu(
        game_context* ctx) {

        if (!ctx->is_paused) return;
        game_pause_menu_dlg(ctx); 
        
        const color_rgba_u32 prev_color = game_ui_get_text_color();

        game_ui_begin(ctx->fnt, ctx->is_paused);
        game_ui_set_text_color(GAME_PAUSE_MENU_TEXT_COLOR_RGBA);

        game_pause_menu_dlg(ctx);

        game_ui_end();
        game_ui_set_text_color(prev_color);
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


        const color_rgba_f32 pause_text_color(GAME_PAUSE_MENU_BACKGROUND_COLOR_RGBA);

        eng_draw_text_screen(
            ctx->fnt,
            pause_text_cstr,
            pause_text_size,
            pause_text_pos,
            pause_text_color
        );
    }
};
