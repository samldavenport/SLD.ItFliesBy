#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITION 
    //--------------------------------------------------------------------
    
    struct game_dialog {
        const cchar*          text;        // not copied, it has to outlive the dialog
        const font*           fnt;
        u32                   text_len;
        font_line             lines[GAME_DIALOG_LINE_CAPACITY];
        u32                   line_count;
        u32                   page_line;   // the first line on the page
        f32                   reveal;      // characters shown on the page, the fraction is the next one coming in
        f32                   blink_s;
        game_dialog_state_u32 state;
    } static * _dlg;
    
    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    // the lines on the current page, the last page can be short
    inline u32
    game_dialog_page_line_count(
        void) {

        const u32 lines_left = _dlg->line_count - _dlg->page_line;
        return((lines_left < GAME_DIALOG_LINES_PER_PAGE) ? lines_left : GAME_DIALOG_LINES_PER_PAGE);
    }

    // the characters on the current page, the spaces and newlines the lines broke on aren't counted
    inline u32
    game_dialog_page_char_count(
        void) {

        u32 char_count = 0;
        const u32 page_line_count = game_dialog_page_line_count();
        for (
              u32 i_line = 0;
                  i_line < page_line_count;
                ++i_line) {

            char_count += _dlg->lines[_dlg->page_line + i_line].length;
        }
        return(char_count);
    }

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL game_dialog*
    game_dialog_create(
        const hnd_arena h_arena,
        const font*     fnt) {

        _dlg = (game_dialog*)eng_arena_push(h_arena, sizeof(game_dialog));
        assert(_dlg);
        
        _dlg->text       = NULL;
        _dlg->fnt        = fnt;
        _dlg->text_len   = 0;
        _dlg->line_count = 0;
        _dlg->page_line  = 0;
        _dlg->reveal     = 0.0f;
        _dlg->blink_s    = 0.0f;
        _dlg->state      = game_dialog_state_e_closed;
    
        return(_dlg);
    }

    IFB_INTERNAL void
    game_dialog_reset(
        void) {

        _dlg->text       = NULL;
        _dlg->text_len   = 0;
        _dlg->line_count = 0;
        _dlg->page_line  = 0;
        _dlg->reveal     = 0.0f;
        _dlg->blink_s    = 0.0f;
        _dlg->state      = game_dialog_state_e_closed;
    }
    
    // the box along the bottom of the screen, and the area inside it the text goes in
    // the box is as tall as a page of lines, so it changes with the font
    IFB_INTERNAL void
    game_dialog_get_layout(
        rect&  box,
        rect&  text_area) {

        assert(_dlg);
        assert(_dlg->fnt);

        auto fnt = _dlg->fnt;

        u32 screen_width  = 0;
        u32 screen_height = 0;
        eng_draw_get_screen_size(screen_width, screen_height);

        const f32 line_step   = fnt->line_height + GAME_DIALOG_LINE_SPACING;
        const f32 text_height = (line_step * (f32)GAME_DIALOG_LINES_PER_PAGE) - GAME_DIALOG_LINE_SPACING;
        const f32 inset       = GAME_DIALOG_BORDER + GAME_DIALOG_PADDING;

        box.width  = (f32)screen_width - (GAME_DIALOG_MARGIN * 2.0f);
        box.height = text_height + (inset * 2.0f);
        box.x      = GAME_DIALOG_MARGIN;
        box.y      = (f32)screen_height - GAME_DIALOG_MARGIN - box.height;

        text_area.x      = box.x + inset;
        text_area.y      = box.y + inset;
        text_area.width  = box.width - (inset * 2.0f);
        text_area.height = text_height;
    }

    // wraps the whole text now, so a word doesn't jump to the next line partway through typing it
    // the text isn't copied, it has to stay around until the dialog closes
    // false if there's nothing to show, the dialog stays closed
    IFB_INTERNAL bool
    game_dialog_open(
        const cchar* text,
        const u32    text_len) {

        assert(_dlg);
        assert(_dlg->fnt);
        assert(text != NULL || text_len == 0);

        auto fnt = _dlg->fnt;

        rect box;
        rect text_area;
        game_dialog_get_layout(box, text_area);

        u32 line_count = eng_font_wrap_text(
            fnt,
            text,
            text_len,
            text_area.width,
            _dlg->lines,
            GAME_DIALOG_LINE_CAPACITY
        );
        if (line_count > GAME_DIALOG_LINE_CAPACITY) line_count = GAME_DIALOG_LINE_CAPACITY;
        if (line_count == 0) return(false);

        _dlg->text       = text;
        _dlg->text_len   = text_len;
        _dlg->line_count = line_count;
        _dlg->page_line  = 0;
        _dlg->reveal     = 0.0f;
        _dlg->blink_s    = 0.0f;
        _dlg->state      = game_dialog_state_e_typing;
        return(true);
    }

    IFB_INTERNAL bool
    game_dialog_is_open(
        void) {

        assert(_dlg);
        return(_dlg->state != game_dialog_state_e_closed);
    }

    // the player pressed the button
    // while typing, the rest of the page shows at once
    // once it's all shown, the next page starts, or the dialog closes after the last one
    IFB_INTERNAL void
    game_dialog_advance(
        void) {

        assert(_dlg);

        switch (_dlg->state.val) {

            case (game_dialog_state_e_typing): {
                _dlg->reveal  = (f32)game_dialog_page_char_count();
                _dlg->blink_s = 0.0f;
                _dlg->state   = game_dialog_state_e_paused;
            } break;

            case (game_dialog_state_e_paused): {
                const u32 next_page_line = _dlg->page_line + GAME_DIALOG_LINES_PER_PAGE;
                if (next_page_line >= _dlg->line_count) {
                    game_dialog_reset();
                    break;
                }
                _dlg->page_line = next_page_line;
                _dlg->reveal    = 0.0f;
                _dlg->state     = game_dialog_state_e_typing;
            } break;

            default: break;
        }
    }

    // e is the action button, an open dialog uses it first to advance
    // otherwise it's left in is_action_pressed for the collision procs, which run in the physics step after this
    // so whatever connor is touching can react to it, like rose opening her dialog
    inline void
    game_dialog_read_action(
        game_context* ctx) {

        // a press nothing used last pass doesn't carry over
        ctx->is_action_pressed = false;

        const bool is_action_down  = eng_input_is_key_down(input_keycode_e_e);
        const bool is_action_press = is_action_down && !ctx->talk_was_down;
        ctx->talk_was_down         = is_action_down;
        if (!is_action_press) return;

        if (game_dialog_is_open()) {
            game_dialog_advance();
        }
        else {
            ctx->is_action_pressed = true;
        }
    }

    // reveals the page, or blinks the marker once it's all shown
    inline void
    game_dialog_tick(
        const f32 dt) {

        assert(_dlg);

        switch (_dlg->state.val) {

            case (game_dialog_state_e_typing): {
                const f32 page_char_count = (f32)game_dialog_page_char_count();
                _dlg->reveal += dt * GAME_DIALOG_CHARS_PER_SECOND;
                if (_dlg->reveal >= page_char_count) {
                    _dlg->reveal  = page_char_count;
                    _dlg->blink_s = 0.0f;
                    _dlg->state   = game_dialog_state_e_paused;
                }
            } break;

            case (game_dialog_state_e_paused): {
                _dlg->blink_s += dt;
                if (_dlg->blink_s >= GAME_DIALOG_ARROW_BLINK_S * 2.0f) {
                    _dlg->blink_s -= GAME_DIALOG_ARROW_BLINK_S * 2.0f;
                }
            } break;

            default: break;
        }
    }

    // the whole dialog for this pass: the action button, the reveal, and drawing it
    IFB_INTERNAL void
    game_dialog_update(
        game_context* ctx) {

        assert(ctx);

        game_dialog_read_action (ctx);
        game_dialog_tick        (eng_system_get_delta_time_s());
        game_dialog_render      ();
    }

    // has to be called every frame the dialog should be seen, like any immediate drawing
    IFB_INTERNAL void
    game_dialog_render(
        void) {

        assert(_dlg);
        assert(_dlg->fnt);

        auto fnt = _dlg->fnt;

        if (!game_dialog_is_open()) return;

        rect box;
        rect text_area;
        game_dialog_get_layout(box, text_area);

        // the border is the box, the fill is drawn over it inside the border
        rect fill;
        fill.x      = box.x      + GAME_DIALOG_BORDER;
        fill.y      = box.y      + GAME_DIALOG_BORDER;
        fill.width  = box.width  - (GAME_DIALOG_BORDER * 2.0f);
        fill.height = box.height - (GAME_DIALOG_BORDER * 2.0f);
        eng_draw_rect_screen(box,  color_rgba_f32(GAME_DIALOG_COLOR_BORDER));
        eng_draw_rect_screen(fill, color_rgba_f32(GAME_DIALOG_COLOR_FILL));

        // the page's lines, up to the characters revealed so far
        const color_rgba_f32 text_color      = color_rgba_f32(GAME_DIALOG_COLOR_TEXT);
        const u32            page_line_count = game_dialog_page_line_count();
        u32                  chars_left      = (u32)_dlg->reveal;
        vec2                 pen             = { text_area.x, text_area.y };
        for (
              u32 i_line = 0;
                  i_line < page_line_count && chars_left > 0;
                ++i_line) {

            const font_line& line      = _dlg->lines[_dlg->page_line + i_line];
            const u32        draw_len  = (line.length < chars_left) ? line.length : chars_left;
            eng_draw_text_screen(_dlg->fnt, &_dlg->text[line.start], draw_len, pen, text_color);

            chars_left -= draw_len;
            pen.y      += fnt->line_height + GAME_DIALOG_LINE_SPACING;
        }

        // once the page is all shown, a blinking marker in the corner says there's more to read
        const bool is_marker_shown = (
            _dlg->state   == game_dialog_state_e_paused &&
            _dlg->blink_s <  GAME_DIALOG_ARROW_BLINK_S
        );
        if (is_marker_shown) {
            rect marker;
            marker.width  = GAME_DIALOG_ARROW_SIZE;
            marker.height = GAME_DIALOG_ARROW_SIZE;
            marker.x      = (fill.x + fill.width)  - GAME_DIALOG_PADDING;
            marker.y      = (fill.y + fill.height) - GAME_DIALOG_PADDING;
            eng_draw_rect_screen(marker, text_color);
        }
    }
};
