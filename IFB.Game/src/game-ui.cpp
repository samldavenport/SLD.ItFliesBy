#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"
#include "sld.hpp"

#include <cassert>
#include <stdio.h>
#include <string.h>

namespace ifb {

    //--------------------------------------------------------------------
    // STATE
    //--------------------------------------------------------------------

    static constexpr u32 GAME_UI_ID_NONE    = 0;
    static constexpr u32 GAME_UI_INDEX_NONE = 0xFFFFFFFF;

    // the keys the controls use, a bit each
    enum game_ui_key_e {
        game_ui_key_e_up       = 0,
        game_ui_key_e_down     = 1,
        game_ui_key_e_left     = 2,
        game_ui_key_e_right    = 3,
        game_ui_key_e_activate = 4
    };

    // what one control knows about itself this pass
    struct game_ui_control {
        u32  id;
        u32  index;      // its place in the call order, the keyboard focus moves along it
        bool is_hot;     // the mouse is over it, and nothing else has the mouse
        bool is_active;  // the mouse was pressed on it, and hasn't been let go
        bool is_focused; // the keyboard works it
    };

    // the open drop down's list, game_ui_end draws it over everything
    struct game_ui_open_list {
        rect                list;
        const cchar* const* items;
        u32                 item_count; // the ones shown
        u32                 selected;
        u32                 highlight;
        u32                 text_color; // the text color when the drop down was called
        bool                is_pending;
    };

    // one for the whole game, the controls are found again each pass by their labels
    struct game_ui {
        const font*       fnt;
        u32               text_color;         // 0xRRGGBBAA, back to GAME_UI_COLOR_TEXT each pass
        bool              is_input_enabled;
        vec2              mouse;
        vec2              mouse_prev;
        bool              has_mouse;          // the mouse is over the screen, and the gui doesn't have it
        bool              is_mouse_down;
        bool              is_mouse_pressed;   // went down this pass
        bool              is_mouse_released;  // went up this pass
        bool              is_mouse_moved;
        u32               keys_down;          // a bit per game_ui_key_e
        u32               keys_pressed;       // went down this pass
        u32               id_active;          // pressed with the mouse, until it's let go
        u32               id_open;            // the open drop down
        u32               focus_index;        // the control the keyboard works, by call order
        bool              has_focus;          // false until a key or a click gives a control the focus
        u32               control_count;      // called so far this pass
        u32               control_count_prev; // called last pass, the focus wraps around this
        bool              is_focus_visible;   // the outline shows once a key moves the focus, and hides on a click
        bool              was_open_seen;      // the open drop down was called this pass, if it wasn't it closes
        rect              open_box;           // the open drop down's box
        rect              open_list;          // and its list, the controls under it don't get the mouse
        u32               open_highlight;     // the item the keys or the mouse are on
        game_ui_open_list open_draw;
    } static _ui;

    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    // fnv-1a, 0 is kept for none
    inline u32
    game_ui_hash_label(
        const cchar* label) {

        u32 hash = 2166136261u;
        for (const cchar* c = label; *c != 0; ++c) {
            hash ^= (u32)(byte)(*c);
            hash *= 16777619u;
        }
        return((hash == GAME_UI_ID_NONE) ? 1 : hash);
    }

    inline bool
    game_ui_rect_contains(
        const rect& r,
        const vec2& p) {

        return(
            p.x >= r.x && p.x < (r.x + r.width) &&
            p.y >= r.y && p.y < (r.y + r.height)
        );
    }

    inline bool
    game_ui_is_key_pressed(
        const game_ui_key_e key) {

        return(((_ui.keys_pressed >> key) & 1) == 1);
    }

    // registers the control, and gives it the mouse if it's pressed on it
    inline game_ui_control
    game_ui_control_begin(
        const rect&  dst,
        const cchar* label) {

        assert(label);

        game_ui_control c;
        c.id         = game_ui_hash_label(label);
        c.index      = _ui.control_count;
        c.is_focused = (_ui.has_focus && _ui.focus_index == c.index);
        ++_ui.control_count;

        // an open list is over everything, except the drop down it belongs to
        const bool is_under_list = (
            _ui.id_open != GAME_UI_ID_NONE &&
            _ui.id_open != c.id            &&
            game_ui_rect_contains(_ui.open_list, _ui.mouse)
        );

        c.is_hot = (
            _ui.has_mouse                                               &&
            !is_under_list                                              &&
            game_ui_rect_contains(dst, _ui.mouse)                       &&
            (_ui.id_active == GAME_UI_ID_NONE || _ui.id_active == c.id)
        );

        if (c.is_hot && _ui.is_mouse_pressed) {
            _ui.id_active   = c.id;
            _ui.focus_index = c.index;
            _ui.has_focus   = true;
            c.is_focused    = true;
        }
        c.is_active = (_ui.id_active == c.id);
        return(c);
    }

    // enter or space on the focused control, while no list has the keys
    inline bool
    game_ui_is_activated(
        const game_ui_control& c) {

        return(
            c.is_focused                               &&
            _ui.id_open == GAME_UI_ID_NONE             &&
            game_ui_is_key_pressed(game_ui_key_e_activate)
        );
    }

    inline u32
    game_ui_fill_color(
        const game_ui_control& c) {

        if (c.is_active && c.is_hot) return(GAME_UI_COLOR_FILL_ACTIVE);
        if (c.is_hot)                return(GAME_UI_COLOR_FILL_HOT);
        return(GAME_UI_COLOR_FILL);
    }

    // a border, then the fill inside it, the border shows the focus
    inline void
    game_ui_draw_frame(
        const rect& dst,
        const u32   fill,
        const bool  is_focused) {

        const bool is_outlined = (is_focused && _ui.is_focus_visible);
        const u32  border      = is_outlined ? GAME_UI_COLOR_FOCUS : GAME_UI_COLOR_BORDER;
        const f32  thickness   = is_outlined ? (GAME_UI_BORDER * 2.0f) : GAME_UI_BORDER;

        rect inner;
        inner.x      = dst.x      + thickness;
        inner.y      = dst.y      + thickness;
        inner.width  = dst.width  - (thickness * 2.0f);
        inner.height = dst.height - (thickness * 2.0f);

        (void)eng_draw_rect_screen(dst,   color_rgba_f32(border));
        (void)eng_draw_rect_screen(inner, color_rgba_f32(fill));
    }

    // vertically centered in dst, from its left edge plus the padding, or centered across it
    inline void
    game_ui_draw_text_color(
        const rect&  dst,
        const cchar* text,
        const bool   is_centered,
        const u32    color) {

        if (_ui.fnt == NULL || text == NULL) return;

        const u32 len = (u32)strlen(text);
        if (len == 0) return;

        vec2 pos;
        pos.y = dst.y + ((dst.height - _ui.fnt->line_height) * 0.5f);
        if (is_centered) {
            const f32 width = eng_font_measure_text(_ui.fnt, text, len);
            pos.x = dst.x + ((dst.width - width) * 0.5f);
        }
        else {
            pos.x = dst.x + GAME_UI_PADDING;
        }

        (void)eng_draw_text_screen(_ui.fnt, text, len, pos, color_rgba_f32(color));
    }

    // in the current text color
    inline void
    game_ui_draw_text(
        const rect&  dst,
        const cchar* text,
        const bool   is_centered) {

        game_ui_draw_text_color(dst, text, is_centered, _ui.text_color);
    }

    inline void
    game_ui_close_list(
        void) {

        _ui.id_open = GAME_UI_ID_NONE;
    }

    //--------------------------------------------------------------------
    // PASS
    //--------------------------------------------------------------------

    // reads the mouse and keys once for the whole pass, and moves the keyboard focus
    IFB_GAME void
    game_ui_begin(
        const font* fnt,
        const bool  is_input_enabled) {

        _ui.fnt              = fnt;
        _ui.text_color       = GAME_UI_COLOR_TEXT;
        _ui.is_input_enabled = is_input_enabled;

        // the mouse, nothing reaches the controls while input is off
        vec2       mouse      = _ui.mouse;
        const bool has_mouse  = is_input_enabled && eng_input_mouse_get_screen_pos(mouse);
        const bool mouse_down = is_input_enabled && eng_input_mouse_is_left_down();

        _ui.is_mouse_pressed  = ( mouse_down && !_ui.is_mouse_down);
        _ui.is_mouse_released = (!mouse_down &&  _ui.is_mouse_down);
        _ui.is_mouse_down     = mouse_down;
        _ui.has_mouse         = has_mouse;
        _ui.mouse_prev        = _ui.mouse;
        _ui.mouse             = mouse;
        _ui.is_mouse_moved    = has_mouse && (mouse.x != _ui.mouse_prev.x || mouse.y != _ui.mouse_prev.y);

        // the keys, as presses, so holding one only moves once
        u32 keys = 0;
        if (is_input_enabled) {
            if (eng_input_is_key_down(input_keycode_e_arrow_up))    keys |= (1 << game_ui_key_e_up);
            if (eng_input_is_key_down(input_keycode_e_arrow_down))  keys |= (1 << game_ui_key_e_down);
            if (eng_input_is_key_down(input_keycode_e_arrow_left))  keys |= (1 << game_ui_key_e_left);
            if (eng_input_is_key_down(input_keycode_e_arrow_right)) keys |= (1 << game_ui_key_e_right);
            if (eng_input_is_key_down(input_keycode_e_enter)    ||
                eng_input_is_key_down(input_keycode_e_kp_enter) ||
                eng_input_is_key_down(input_keycode_e_space))       keys |= (1 << game_ui_key_e_activate);
        }
        _ui.keys_pressed = keys & ~_ui.keys_down;
        _ui.keys_down    = keys;

        if (_ui.keys_pressed != 0) _ui.is_focus_visible = true;
        if (_ui.is_mouse_pressed)  _ui.is_focus_visible = false;

        // nothing's held while input is off
        if (!is_input_enabled) {
            _ui.id_active = GAME_UI_ID_NONE;
        }

        // up and down walk the controls from last pass, unless an open list has them
        _ui.control_count_prev = _ui.control_count;
        _ui.control_count      = 0;
        const u32 count        = _ui.control_count_prev;
        if (_ui.id_open == GAME_UI_ID_NONE && count > 0) {

            if (game_ui_is_key_pressed(game_ui_key_e_down)) {
                _ui.focus_index = (!_ui.has_focus || _ui.focus_index + 1 >= count)
                    ? 0
                    : _ui.focus_index + 1;
                _ui.has_focus = true;
            }
            if (game_ui_is_key_pressed(game_ui_key_e_up)) {
                _ui.focus_index = (!_ui.has_focus || _ui.focus_index == 0)
                    ? count - 1
                    : _ui.focus_index - 1;
                _ui.has_focus = true;
            }
        }
        // fewer controls than last time, the focus stays on the last one
        if (_ui.has_focus && count > 0 && _ui.focus_index >= count) {
            _ui.focus_index = count - 1;
        }

        _ui.was_open_seen         = false;
        _ui.open_draw.is_pending = false;
    }

    // closes the list when it's clicked off of, draws it last so it's over the other controls,
    // and lets go of the mouse once the button is up
    IFB_GAME void
    game_ui_end(
        void) {

        if (_ui.id_open != GAME_UI_ID_NONE) {

            const bool is_click_outside = (
                _ui.is_mouse_pressed                                 &&
                !game_ui_rect_contains(_ui.open_list, _ui.mouse)     &&
                !game_ui_rect_contains(_ui.open_box,  _ui.mouse)
            );

            // a drop down that isn't called anymore can't be closed by anything else
            if (!_ui.was_open_seen || is_click_outside) {
                game_ui_close_list();
            }
        }

        if (_ui.id_open != GAME_UI_ID_NONE && _ui.open_draw.is_pending) {

            const game_ui_open_list& open = _ui.open_draw;

            game_ui_draw_frame(open.list, GAME_UI_COLOR_FILL, false);

            const f32 item_height = open.list.height / (f32)open.item_count;
            for (
                u32 i_item = 0;
                    i_item < open.item_count;
                  ++i_item) {

                rect row;
                row.x      = open.list.x + GAME_UI_BORDER;
                row.y      = open.list.y + (item_height * (f32)i_item);
                row.width  = open.list.width - (GAME_UI_BORDER * 2.0f);
                row.height = item_height;

                if (i_item == open.highlight) {
                    rect band = row;
                    band.y      += GAME_UI_BORDER;
                    band.height -= GAME_UI_BORDER * 2.0f;
                    const u32 band_color = (i_item == open.selected) ? GAME_UI_COLOR_FILL_ACTIVE : GAME_UI_COLOR_FILL_HOT;
                    (void)eng_draw_rect_screen(band, color_rgba_f32(band_color));
                }
                game_ui_draw_text_color(row, open.items[i_item], false, open.text_color);
            }
        }
        _ui.open_draw.is_pending = false;

        if (!_ui.is_mouse_down) {
            _ui.id_active = GAME_UI_ID_NONE;
        }
    }

    // the text color for the controls called after this, until it's set again or the pass ends
    IFB_GAME void
    game_ui_set_text_color(
        const color_rgba_u32 color) {

        _ui.text_color = color.hex;
    }

    IFB_GAME color_rgba_u32 
    game_ui_get_text_color(
        void) {

        return(_ui.text_color);
    }

    //--------------------------------------------------------------------
    // CONTROLS
    //--------------------------------------------------------------------

    // clicked when the mouse is let go over it, after being pressed on it
    IFB_GAME bool
    game_ui_button(
        const rect&  dst,
        const cchar* label) {

        const game_ui_control c = game_ui_control_begin(dst, label);

        const bool is_clicked = (
            (c.is_active && c.is_hot && _ui.is_mouse_released) ||
            game_ui_is_activated(c)
        );

        game_ui_draw_frame (dst, game_ui_fill_color(c), c.is_focused);
        game_ui_draw_text  (dst, label, true);
        return(is_clicked);
    }

    // a box on the left of dst with the label after it, clicking anywhere in dst flips it
    IFB_GAME bool
    game_ui_checkbox(
        const rect&  dst,
        const cchar* label,
        bool&        value) {

        const game_ui_control c = game_ui_control_begin(dst, label);

        const bool is_flipped = (
            (c.is_active && c.is_hot && _ui.is_mouse_released) ||
            game_ui_is_activated(c)
        );
        if (is_flipped) value = !value;

        rect box;
        box.x      = dst.x;
        box.y      = dst.y;
        box.width  = dst.height;
        box.height = dst.height;
        game_ui_draw_frame(box, game_ui_fill_color(c), c.is_focused);

        if (value) {
            const f32 inset = dst.height * 0.25f;
            rect tick;
            tick.x      = box.x      + inset;
            tick.y      = box.y      + inset;
            tick.width  = box.width  - (inset * 2.0f);
            tick.height = box.height - (inset * 2.0f);
            (void)eng_draw_rect_screen(tick, color_rgba_f32(GAME_UI_COLOR_BORDER));
        }

        rect text_area = dst;
        text_area.x     += box.width;
        text_area.width -= box.width;
        game_ui_draw_text(text_area, label, false);

        return(is_flipped);
    }

    // dragging anywhere on it moves the knob to the mouse, the label and value are written over it
    IFB_GAME bool
    game_ui_slider(
        const rect&  dst,
        const cchar* label,
        f32&         value,
        const f32    min,
        const f32    max) {

        assert(max > min);

        const game_ui_control c      = game_ui_control_begin(dst, label);
        const f32             before = value;

        // the knob travels inside the border
        const f32 travel_x     = dst.x + GAME_UI_BORDER;
        const f32 travel_width = dst.width - (GAME_UI_BORDER * 2.0f) - GAME_UI_SLIDER_KNOB_WIDTH;

        // held, it follows the mouse even off the end, so it's easy to pin to min or max
        if (c.is_active && _ui.has_mouse && travel_width > 0.0f) {
            const f32 t = (_ui.mouse.x - travel_x - (GAME_UI_SLIDER_KNOB_WIDTH * 0.5f)) / travel_width;
            value = min + ((max - min) * t);
        }

        if (c.is_focused && _ui.id_open == GAME_UI_ID_NONE) {
            const f32 step = (max - min) / GAME_UI_SLIDER_KEY_STEPS;
            if (game_ui_is_key_pressed(game_ui_key_e_left))  value -= step;
            if (game_ui_is_key_pressed(game_ui_key_e_right)) value += step;
        }

        if (value < min) value = min;
        if (value > max) value = max;

        const f32 t = (value - min) / (max - min);

        game_ui_draw_frame(dst, game_ui_fill_color(c), c.is_focused);

        // filled up to the knob, so the value reads at a glance
        rect filled;
        filled.x      = travel_x;
        filled.y      = dst.y + GAME_UI_BORDER;
        filled.width  = travel_width * t;
        filled.height = dst.height - (GAME_UI_BORDER * 2.0f);
        if (filled.width > 0.0f) {
            (void)eng_draw_rect_screen(filled, color_rgba_f32(GAME_UI_COLOR_FILL_ACTIVE));
        }

        rect knob;
        knob.x      = travel_x + (travel_width * t);
        knob.y      = filled.y;
        knob.width  = GAME_UI_SLIDER_KNOB_WIDTH;
        knob.height = filled.height;
        (void)eng_draw_rect_screen(knob, color_rgba_f32(GAME_UI_COLOR_TRACK));

        cchar text[64];
        (void)snprintf(text, sizeof(text), "%s %.2f", label, value);
        game_ui_draw_text(dst, text, true);

        return(value != before);
    }

    // a box with the selected item, clicking it opens the list of items under it
    // the list is handled here, but drawn by game_ui_end so it's over the controls after this one
    IFB_GAME bool
    game_ui_dropdown(
        const rect&         dst,
        const cchar*        label,
        const cchar* const* items,
        const u32           item_count,
        u32&                selected) {

        assert(items != NULL);
        assert(item_count > 0);

        const game_ui_control c = game_ui_control_begin(dst, label);

        if (selected >= item_count) selected = item_count - 1;
        const u32 before = selected;

        const u32 shown_count = (item_count < GAME_UI_DROPDOWN_ITEMS_MAX) ? item_count : GAME_UI_DROPDOWN_ITEMS_MAX;

        // one row per item, as tall as the box, right under it
        rect list;
        list.x      = dst.x;
        list.y      = dst.y + dst.height;
        list.width  = dst.width;
        list.height = dst.height * (f32)shown_count;

        if (_ui.id_open == c.id) {

            _ui.was_open_seen = true;
            _ui.open_box      = dst;
            _ui.open_list     = list;

            // the mouse highlights the item it's over, when it moves, so it doesn't fight the keys
            const bool is_over_list = _ui.has_mouse && game_ui_rect_contains(list, _ui.mouse);
            u32        i_under      = GAME_UI_INDEX_NONE;
            if (is_over_list) {
                i_under = (u32)((_ui.mouse.y - list.y) / dst.height);
                if (i_under >= shown_count) i_under = shown_count - 1;
                if (_ui.is_mouse_moved) _ui.open_highlight = i_under;
            }

            if (game_ui_is_key_pressed(game_ui_key_e_up)   && _ui.open_highlight > 0)               --_ui.open_highlight;
            if (game_ui_is_key_pressed(game_ui_key_e_down) && _ui.open_highlight + 1 < shown_count) ++_ui.open_highlight;

            // letting go over an item picks it, so pressing the box and dragging down to an item works too
            if (is_over_list && _ui.is_mouse_released) {
                selected = i_under;
                game_ui_close_list();
            }
            else if (game_ui_is_key_pressed(game_ui_key_e_activate)) {
                selected = _ui.open_highlight;
                game_ui_close_list();
            }
            // pressing the box again closes it
            else if (c.is_hot && _ui.is_mouse_pressed) {
                game_ui_close_list();
            }
        }
        else {

            const bool is_opened = (
                (c.is_hot && _ui.is_mouse_pressed) ||
                game_ui_is_activated(c)
            );

            if (is_opened) {
                _ui.id_open        = c.id;
                _ui.was_open_seen  = true;
                _ui.open_box       = dst;
                _ui.open_list      = list;
                _ui.open_highlight = (selected < shown_count) ? selected : 0;
            }
            else if (c.is_focused && _ui.id_open == GAME_UI_ID_NONE) {
                if (game_ui_is_key_pressed(game_ui_key_e_left)  && selected > 0)              --selected;
                if (game_ui_is_key_pressed(game_ui_key_e_right) && selected + 1 < item_count) ++selected;
            }
        }

        const bool is_open = (_ui.id_open == c.id);

        game_ui_draw_frame(dst, is_open ? GAME_UI_COLOR_FILL_HOT : game_ui_fill_color(c), c.is_focused);

        cchar text[64];
        (void)snprintf(text, sizeof(text), "%s: %s", label, items[selected]);
        game_ui_draw_text(dst, text, false);

        // the marker on the right, it points down while closed and up while open
        rect marker_area;
        marker_area.width  = dst.height;
        marker_area.height = dst.height;
        marker_area.x      = (dst.x + dst.width) - dst.height;
        marker_area.y      = dst.y;
        game_ui_draw_text(marker_area, is_open ? "^" : "v", true);

        if (is_open) {
            _ui.open_draw.list       = list;
            _ui.open_draw.items      = items;
            _ui.open_draw.item_count = shown_count;
            _ui.open_draw.selected   = selected;
            _ui.open_draw.highlight  = _ui.open_highlight;
            _ui.open_draw.text_color = _ui.text_color;
            _ui.open_draw.is_pending = true;
        }

        return(selected != before);
    }

    IFB_GAME void
    game_ui_label(
        const rect&  dst,
        const cchar* text) {

        rect area = dst;
        area.x -= GAME_UI_PADDING; // labels line up with the edges of the controls, not their text
        game_ui_draw_text(area, text, false);
    }
};
