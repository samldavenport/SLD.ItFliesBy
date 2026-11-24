#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"
#include "sld.hpp"

namespace ifb {

    static constexpr f32 GAME_PAUSE_MENU_ROW_HEIGHT            = 32.0f;
    static constexpr f32 GAME_PAUSE_MENU_ROW_GAP               = 8.0f;
    static constexpr f32 GAME_PAUSE_MENU_COLUMN_GAP            = 32.0f;
    static constexpr f32 GAME_PAUSE_MENU_ROW_WIDTH             = 360.0f;
    static constexpr u32 GAME_PAUSE_MENU_ROW_COUNT             = 4;      // the header and the three sliders
    static constexpr u32 GAME_PAUSE_MENU_TEXT_COLOR_RGBA       = 0xFFFFFFFF;
    static constexpr u32 GAME_PAUSE_MENU_BACKGROUND_COLOR_RGBA = 0x212121C8;
   
    struct screen_dims {
        u32 width;
        u32 height;
    };

    inline f32  game_pause_menu_row_y           (const screen_dims& dims, const u32 row);
    inline void game_pause_menu_get_screen_dims (screen_dims& dims);
    inline void game_pause_menu_set_background  (screen_dims& dims);
    inline void game_pause_menu_header          (screen_dims& dims, const font* fnt);
    inline void game_pause_menu_master_volume   (screen_dims& dims);
    inline void game_pause_menu_music_volume    (screen_dims& dims);
    inline void game_pause_menu_sfx_volume      (screen_dims& dims);

    IFB_GAME void
    game_pause_menu(
        game_context* ctx) {

        if (!ctx->is_paused) return;
        
        const color_rgba_u32 prev_color = game_ui_get_text_color();
        screen_dims scr_dims;


        game_ui_begin(ctx->fnt, ctx->is_paused);
        game_ui_set_text_color(GAME_PAUSE_MENU_TEXT_COLOR_RGBA);

        game_pause_menu_get_screen_dims (scr_dims);
        game_pause_menu_set_background  (scr_dims);
        game_pause_menu_header          (scr_dims, ctx->fnt);
        game_pause_menu_master_volume   (scr_dims);
        game_pause_menu_music_volume    (scr_dims);
        game_pause_menu_sfx_volume      (scr_dims);

        game_ui_end();
        game_ui_set_text_color(prev_color);
    }
    
    // the top of a row, every row is ROW_HEIGHT tall and ROW_GAP from the next,
    // and the rows together are centered vertically on the screen
    inline f32
    game_pause_menu_row_y(
        const screen_dims& dims,
        const u32          row) {

        const f32 row_step      = GAME_PAUSE_MENU_ROW_HEIGHT + GAME_PAUSE_MENU_ROW_GAP;
        const f32 column_height = (row_step * (f32)GAME_PAUSE_MENU_ROW_COUNT) - GAME_PAUSE_MENU_ROW_GAP;
        const f32 column_top    = ((f32)dims.height - column_height) * 0.5f;
        return(column_top + (row_step * (f32)row));
    }

    inline void
    game_pause_menu_get_screen_dims(
        screen_dims& dims) {

        eng_draw_get_screen_size(
            dims.width,
            dims.height
        );
    }
    
    inline void
    game_pause_menu_set_background(
        screen_dims& dims) {
    
        rect background_rect;
        background_rect.width  = dims.width;
        background_rect.height = dims.height;
        background_rect.x      = 0.0f;
        background_rect.y      = 0.0f;

        color_rgba_f32 pause_dlg_color(GAME_PAUSE_MENU_BACKGROUND_COLOR_RGBA);

        bool did_draw;
        
        did_draw = eng_draw_rect_screen(background_rect, pause_dlg_color);
        assert(did_draw); 
    }
    
    inline void
    game_pause_menu_header(
        screen_dims& dims,
        const font*  fnt) {
        
        const cchar pause_text_cstr[] = "GAME PAUSED";
        const u32   pause_text_size   = sizeof(pause_text_cstr) - 1; // not the null terminator
        const f32   pause_text_width  = eng_font_measure_text(fnt, pause_text_cstr, pause_text_size);

        // game_ui_label starts the text GAME_UI_PADDING in from the rect's left edge,
        // so the rect is the text plus the padding on each side, centered
        rect header_rect;
        header_rect.width  = pause_text_width + (GAME_UI_PADDING * 2.0f);
        header_rect.height = GAME_PAUSE_MENU_ROW_HEIGHT;
        header_rect.x      = ((f32)dims.width  - (f32)header_rect.width) * 0.5f;
        header_rect.y      = game_pause_menu_row_y(dims, 0);

        game_ui_label(header_rect, pause_text_cstr);
    }
    
    inline void
    game_pause_menu_master_volume(
        screen_dims& dims) {

        rect slider_rect;
        slider_rect.width  = GAME_PAUSE_MENU_ROW_WIDTH; 
        slider_rect.height = GAME_PAUSE_MENU_ROW_HEIGHT;
        slider_rect.x      = ((f32)dims.width  - (f32)slider_rect.width) * 0.5f; 
        slider_rect.y      = game_pause_menu_row_y(dims, 1);

        game_ui_set_text_color(0x000000FF);

        // the engine keeps the volume, the slider shows it and sets it when it's moved
        f32 volume = eng_audio_get_master_volume();
        if (game_ui_slider(slider_rect, "MASTER VOLUME", volume, 0.0f, 1.0f)) {
            eng_audio_set_master_volume(volume);
        }
    }
    
    inline void
    game_pause_menu_music_volume(
        screen_dims& dims) {

        rect slider_rect;
        slider_rect.width  = GAME_PAUSE_MENU_ROW_WIDTH; 
        slider_rect.height = GAME_PAUSE_MENU_ROW_HEIGHT;
        slider_rect.x      = ((f32)dims.width  - (f32)slider_rect.width) * 0.5f; 
        slider_rect.y      = game_pause_menu_row_y(dims, 2);
        
        game_ui_set_text_color(0x000000FF);

        f32 volume = eng_audio_group_get_volume(audio_group_e_music);
        if (game_ui_slider(slider_rect, "MUSIC VOLUME", volume, 0.0f, 1.0f)) {
            eng_audio_group_set_volume(audio_group_e_music, volume);
        }
    }

    inline void
    game_pause_menu_sfx_volume(
        screen_dims& dims) {

        rect slider_rect;
        slider_rect.width  = GAME_PAUSE_MENU_ROW_WIDTH; 
        slider_rect.height = GAME_PAUSE_MENU_ROW_HEIGHT;
        slider_rect.x      = ((f32)dims.width  - (f32)slider_rect.width) * 0.5f; 
        slider_rect.y      = game_pause_menu_row_y(dims, 3);
        game_ui_set_text_color(0x000000FF);

        f32 volume = eng_audio_group_get_volume(audio_group_e_sfx);
        if (game_ui_slider(slider_rect, "SFX VOLUME", volume, 0.0f, 1.0f)) {
            eng_audio_group_set_volume(audio_group_e_sfx, volume);
        }
    }
}
