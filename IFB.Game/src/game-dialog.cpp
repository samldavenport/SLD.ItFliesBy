#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    // the lines on the current page, the last page can be short
    inline u32
    game_dialog_page_line_count(
        const game_dialog* dialog) {

        const u32 lines_left = dialog->line_count - dialog->page_line;
        return((lines_left < GAME_DIALOG_LINES_PER_PAGE) ? lines_left : GAME_DIALOG_LINES_PER_PAGE);
    }

    // the characters on the current page, the spaces and newlines the lines broke on aren't counted
    inline u32
    game_dialog_page_char_count(
        const game_dialog* dialog) {

        u32 char_count = 0;
        const u32 page_line_count = game_dialog_page_line_count(dialog);
        for (
              u32 i_line = 0;
                  i_line < page_line_count;
                ++i_line) {

            char_count += dialog->lines[dialog->page_line + i_line].length;
        }
        return(char_count);
    }

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    game_dialog_init(
        game_dialog* dialog) {

        assert(dialog);

        dialog->text       = NULL;
        dialog->text_len   = 0;
        dialog->line_count = 0;
        dialog->page_line  = 0;
        dialog->reveal     = 0.0f;
        dialog->blink_s    = 0.0f;
        dialog->state      = game_dialog_state_e_closed;
    }

    // the box along the bottom of the screen, and the area inside it the text goes in
    // the box is as tall as a page of lines, so it changes with the font
    IFB_INTERNAL void
    game_dialog_get_layout(
        const font* fnt,
        rect&       box,
        rect&       text_area) {

        assert(fnt);

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
        game_dialog* dialog,
        const font*  fnt,
        const cchar* text,
        const u32    text_len) {

        assert(dialog);
        assert(fnt);
        assert(text != NULL || text_len == 0);

        rect box;
        rect text_area;
        game_dialog_get_layout(fnt, box, text_area);

        u32 line_count = eng_font_wrap_text(
            fnt,
            text,
            text_len,
            text_area.width,
            dialog->lines,
            GAME_DIALOG_LINE_CAPACITY
        );
        if (line_count > GAME_DIALOG_LINE_CAPACITY) line_count = GAME_DIALOG_LINE_CAPACITY;
        if (line_count == 0) return(false);

        dialog->text       = text;
        dialog->text_len   = text_len;
        dialog->line_count = line_count;
        dialog->page_line  = 0;
        dialog->reveal     = 0.0f;
        dialog->blink_s    = 0.0f;
        dialog->state      = game_dialog_state_e_typing;
        return(true);
    }

    IFB_INTERNAL bool
    game_dialog_is_open(
        const game_dialog* dialog) {

        assert(dialog);
        return(dialog->state != game_dialog_state_e_closed);
    }

    // the player pressed the button
    // while typing, the rest of the page shows at once
    // once it's all shown, the next page starts, or the dialog closes after the last one
    IFB_INTERNAL void
    game_dialog_advance(
        game_dialog* dialog) {

        assert(dialog);

        switch (dialog->state.val) {

            case (game_dialog_state_e_typing): {
                dialog->reveal  = (f32)game_dialog_page_char_count(dialog);
                dialog->blink_s = 0.0f;
                dialog->state   = game_dialog_state_e_paused;
            } break;

            case (game_dialog_state_e_paused): {
                const u32 next_page_line = dialog->page_line + GAME_DIALOG_LINES_PER_PAGE;
                if (next_page_line >= dialog->line_count) {
                    game_dialog_init(dialog);
                    break;
                }
                dialog->page_line = next_page_line;
                dialog->reveal    = 0.0f;
                dialog->state     = game_dialog_state_e_typing;
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

        if (game_dialog_is_open(ctx->dialog)) {
            game_dialog_advance(ctx->dialog);
        }
        else {
            ctx->is_action_pressed = true;
        }
    }

    // reveals the page, or blinks the marker once it's all shown
    inline void
    game_dialog_tick(
        game_dialog* dialog,
        const f32    dt) {

        assert(dialog);

        switch (dialog->state.val) {

            case (game_dialog_state_e_typing): {
                const f32 page_char_count = (f32)game_dialog_page_char_count(dialog);
                dialog->reveal += dt * GAME_DIALOG_CHARS_PER_SECOND;
                if (dialog->reveal >= page_char_count) {
                    dialog->reveal  = page_char_count;
                    dialog->blink_s = 0.0f;
                    dialog->state   = game_dialog_state_e_paused;
                }
            } break;

            case (game_dialog_state_e_paused): {
                dialog->blink_s += dt;
                if (dialog->blink_s >= GAME_DIALOG_ARROW_BLINK_S * 2.0f) {
                    dialog->blink_s -= GAME_DIALOG_ARROW_BLINK_S * 2.0f;
                }
            } break;

            default: break;
        }
    }

    // the whole dialog for this pass: the action button, the reveal, and drawing it
    // the dialog needs the font, without it there's nothing to show, and nothing uses the button
    IFB_INTERNAL void
    game_dialog_update(
        game_context* ctx) {

        assert(ctx);
        assert(ctx->dialog);

        if (ctx->fnt == NULL) {
            ctx->is_action_pressed = false;
            return;
        }

        game_dialog_read_action (ctx);
        game_dialog_tick        (ctx->dialog, eng_system_get_delta_time_s());
        game_dialog_render      (ctx->dialog, ctx->fnt);
    }

    // has to be called every frame the dialog should be seen, like any immediate drawing
    IFB_INTERNAL void
    game_dialog_render(
        const game_dialog* dialog,
        const font*        fnt) {

        assert(dialog);
        assert(fnt);

        if (!game_dialog_is_open(dialog)) return;

        rect box;
        rect text_area;
        game_dialog_get_layout(fnt, box, text_area);

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
        const u32            page_line_count = game_dialog_page_line_count(dialog);
        u32                  chars_left      = (u32)dialog->reveal;
        vec2                 pen             = { text_area.x, text_area.y };
        for (
              u32 i_line = 0;
                  i_line < page_line_count && chars_left > 0;
                ++i_line) {

            const font_line& line      = dialog->lines[dialog->page_line + i_line];
            const u32        draw_len  = (line.length < chars_left) ? line.length : chars_left;
            eng_draw_text_screen(fnt, &dialog->text[line.start], draw_len, pen, text_color);

            chars_left -= draw_len;
            pen.y      += fnt->line_height + GAME_DIALOG_LINE_SPACING;
        }

        // once the page is all shown, a blinking marker in the corner says there's more to read
        const bool is_marker_shown = (
            dialog->state   == game_dialog_state_e_paused &&
            dialog->blink_s <  GAME_DIALOG_ARROW_BLINK_S
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
