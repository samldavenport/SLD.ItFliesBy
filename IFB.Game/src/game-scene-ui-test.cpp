#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include <cassert>
#include <stdio.h>
#include <string.h>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    // screen pixels, the panel is centered on the screen
    static constexpr f32 GAME_UI_TEST_ROW_HEIGHT     = 32.0f;
    static constexpr f32 GAME_UI_TEST_ROW_GAP        = 8.0f;
    static constexpr f32 GAME_UI_TEST_COLUMN_WIDTH   = 360.0f; // the controls
    static constexpr f32 GAME_UI_TEST_PREVIEW_SIZE   = 240.0f; // the preview box beside them
    static constexpr f32 GAME_UI_TEST_COLUMN_GAP     = 32.0f;
    static constexpr f32 GAME_UI_TEST_PANEL_PADDING  = 24.0f;
    static constexpr u32 GAME_UI_TEST_ROW_COUNT      = 13;     // the rows the controls column lays out
    static constexpr f32 GAME_UI_TEST_SIZE_MIN       = 24.0f;  // the swatch's size, in screen pixels
    static constexpr f32 GAME_UI_TEST_SIZE_MAX       = 200.0f;
    static constexpr u32 GAME_UI_TEST_TITLE_COLOR    = 0x2850C8FF; // game_ui_set_text_color, on the panel's light fill
    static constexpr u32 GAME_UI_TEST_HINT_COLOR     = 0x7880A0FF;

    // the swatch's proportions, width then height, against its size
    static const cchar* const GAME_UI_TEST_SHAPES[]       = { "Square", "Wide", "Tall", "Thin" };
    static constexpr f32      GAME_UI_TEST_SHAPE_SCALE[]  = { 1.0f, 1.0f,  0.5f, 1.0f,  1.0f, 0.5f,  1.0f, 0.15f };
    static constexpr u32      GAME_UI_TEST_SHAPE_COUNT    = sizeof(GAME_UI_TEST_SHAPES) / sizeof(GAME_UI_TEST_SHAPES[0]);

    // behind everything
    static const cchar* const GAME_UI_TEST_BACKDROPS[]      = { "Slate", "Night", "Moss", "Plum", "Sand" };
    static constexpr u32      GAME_UI_TEST_BACKDROP_COLOR[] = { 0x3A4050FF, 0x141828FF, 0x2E4A34FF, 0x4A2E48FF, 0xB8A888FF };
    static constexpr u32      GAME_UI_TEST_BACKDROP_COUNT   = sizeof(GAME_UI_TEST_BACKDROPS) / sizeof(GAME_UI_TEST_BACKDROPS[0]);

    static_assert(sizeof(GAME_UI_TEST_SHAPE_SCALE)    / sizeof(f32) == GAME_UI_TEST_SHAPE_COUNT * 2, "a width and height per shape");
    static_assert(sizeof(GAME_UI_TEST_BACKDROP_COLOR) / sizeof(u32) == GAME_UI_TEST_BACKDROP_COUNT,  "a color per backdrop");

    //--------------------------------------------------------------------
    // STATE
    //--------------------------------------------------------------------

    // what the controls change, in the scene's memory, NULL while it isn't the active scene
    struct game_scene_ui_test {
        hnd_scene h_scene;
        u32       press_count;
        bool      show_preview;
        bool      has_outline;
        f32       red;
        f32       green;
        f32       blue;
        f32       size;
        u32       shape;
        u32       backdrop;
        cchar     last_event[64]; // the last thing a control did, so every one is seen to report
    } static * _scene_ui_test;

    inline void game_scene_ui_test_reset          (game_scene_ui_test* test);
    inline void game_scene_ui_test_draw_backdrop  (const game_scene_ui_test* test);
    inline void game_scene_ui_test_draw_controls  (game_scene_ui_test* test, const rect& column);
    inline void game_scene_ui_test_draw_preview   (const game_scene_ui_test* test, const rect& area);

    //--------------------------------------------------------------------
    // SCENE PROCS
    //--------------------------------------------------------------------

    // there's no map or entities, only the screen layer
    IFB_GAME void
    game_scene_ui_test_enter(
        game_context*   ctx,
        const hnd_scene h_scene) {

        assert(ctx);
        assert(h_scene != INVALID_HANDLE);
        assert(_scene_ui_test == NULL);

        auto test = (game_scene_ui_test*)eng_scene_alloc(h_scene, sizeof(game_scene_ui_test));
        assert(test != NULL);
        memset(test, 0, sizeof(game_scene_ui_test));

        test->h_scene  = h_scene;
        _scene_ui_test = test;

        game_scene_ui_test_reset(test);
    }

    IFB_GAME void
    game_scene_ui_test_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(_scene_ui_test);

        auto test = _scene_ui_test;

        const hnd_scene h_scene = test->h_scene;
        if (!eng_scene_is_updating(h_scene) &&
            !eng_scene_is_rendering(h_scene)) {
            return;
        }

        // drawn while paused or fading, but nothing can be used
        const bool is_input_enabled = (
            eng_scene_is_updating(h_scene) &&
            !ctx->is_paused                &&
            !game_scene_is_switching()
        );

        u32 screen_width  = 0;
        u32 screen_height = 0;
        eng_draw_get_screen_size(screen_width, screen_height);

        // the controls on the left, the preview on the right, the pair centered on the screen
        const f32 column_height = (GAME_UI_TEST_ROW_HEIGHT * (f32)GAME_UI_TEST_ROW_COUNT) + (GAME_UI_TEST_ROW_GAP * (f32)(GAME_UI_TEST_ROW_COUNT - 1));
        const f32 content_width = GAME_UI_TEST_COLUMN_WIDTH + GAME_UI_TEST_COLUMN_GAP + GAME_UI_TEST_PREVIEW_SIZE;

        rect panel;
        panel.width  = content_width + (GAME_UI_TEST_PANEL_PADDING * 2.0f);
        panel.height = column_height + (GAME_UI_TEST_PANEL_PADDING * 2.0f);
        panel.x      = ((f32)screen_width  - panel.width)  * 0.5f;
        panel.y      = ((f32)screen_height - panel.height) * 0.5f;

        rect column;
        column.x      = panel.x + GAME_UI_TEST_PANEL_PADDING;
        column.y      = panel.y + GAME_UI_TEST_PANEL_PADDING;
        column.width  = GAME_UI_TEST_COLUMN_WIDTH;
        column.height = column_height;

        rect preview;
        preview.x      = column.x + GAME_UI_TEST_COLUMN_WIDTH + GAME_UI_TEST_COLUMN_GAP;
        preview.y      = column.y + GAME_UI_TEST_ROW_HEIGHT + GAME_UI_TEST_ROW_GAP; // under the title row
        preview.width  = GAME_UI_TEST_PREVIEW_SIZE;
        preview.height = GAME_UI_TEST_PREVIEW_SIZE;

        game_scene_ui_test_draw_backdrop(test);

        // the panel, in the controls' own border and fill
        rect panel_fill = panel;
        panel_fill.x      += GAME_UI_BORDER * 2.0f;
        panel_fill.y      += GAME_UI_BORDER * 2.0f;
        panel_fill.width  -= GAME_UI_BORDER * 4.0f;
        panel_fill.height -= GAME_UI_BORDER * 4.0f;
        (void)eng_draw_rect_screen(panel,      color_rgba_f32(GAME_UI_COLOR_BORDER));
        (void)eng_draw_rect_screen(panel_fill, color_rgba_f32(GAME_UI_COLOR_FILL));

        game_scene_ui_test_draw_preview(test, preview);

        game_ui_begin(ctx->fnt, is_input_enabled);
        game_scene_ui_test_draw_controls(test, column);
        game_ui_end();
    }

    // the scene memory is gone with the scene, there's nothing else to release
    IFB_GAME void
    game_scene_ui_test_exit(
        game_context* ctx) {

        assert(ctx);
        assert(_scene_ui_test);
        _scene_ui_test = NULL;
    }

    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    inline void
    game_scene_ui_test_reset(
        game_scene_ui_test* test) {

        test->press_count  = 0;
        test->show_preview = true;
        test->has_outline  = true;
        test->red          = 0.95f;
        test->green        = 0.60f;
        test->blue         = 0.20f;
        test->size         = 140.0f;
        test->shape        = 0;
        test->backdrop     = 0;
        (void)snprintf(test->last_event, sizeof(test->last_event), "Nothing yet");
    }

    inline void
    game_scene_ui_test_draw_backdrop(
        const game_scene_ui_test* test) {

        u32 screen_width  = 0;
        u32 screen_height = 0;
        eng_draw_get_screen_size(screen_width, screen_height);

        rect screen;
        screen.x      = 0.0f;
        screen.y      = 0.0f;
        screen.width  = (f32)screen_width;
        screen.height = (f32)screen_height;
        (void)eng_draw_rect_screen(screen, color_rgba_f32(GAME_UI_TEST_BACKDROP_COLOR[test->backdrop]));
    }

    // one row after another down the column, every control there is,
    // the two drop downs are stacked so the first one's list covers the second
    inline void
    game_scene_ui_test_draw_controls(
        game_scene_ui_test* test,
        const rect&         column) {

        const f32 step = GAME_UI_TEST_ROW_HEIGHT + GAME_UI_TEST_ROW_GAP;

        rect row;
        row.x      = column.x;
        row.y      = column.y;
        row.width  = column.width;
        row.height = GAME_UI_TEST_ROW_HEIGHT;

        cchar text[96];

        // the title and the hint use their own text color, everything between is the default
        game_ui_set_text_color(GAME_UI_TEST_TITLE_COLOR);
        game_ui_label(row, "UI TEST");
        game_ui_set_text_color(GAME_UI_COLOR_TEXT);
        row.y += step;

        // buttons, side by side
        rect half = row;
        half.width = (row.width - GAME_UI_TEST_ROW_GAP) * 0.5f;
        if (game_ui_button(half, "Press Me")) {
            ++test->press_count;
            (void)snprintf(test->last_event, sizeof(test->last_event), "Pressed %u times", test->press_count);
        }
        half.x += half.width + GAME_UI_TEST_ROW_GAP;
        if (game_ui_button(half, "Reset")) {
            game_scene_ui_test_reset(test);
            (void)snprintf(test->last_event, sizeof(test->last_event), "Reset everything");
        }
        row.y += step;

        // checkboxes
        if (game_ui_checkbox(row, "Show Preview", test->show_preview)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Preview %s", test->show_preview ? "on" : "off");
        }
        row.y += step;
        if (game_ui_checkbox(row, "Outline", test->has_outline)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Outline %s", test->has_outline ? "on" : "off");
        }
        row.y += step;

        // sliders
        if (game_ui_slider(row, "Red", test->red, 0.0f, 1.0f)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Red %.2f", test->red);
        }
        row.y += step;
        if (game_ui_slider(row, "Green", test->green, 0.0f, 1.0f)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Green %.2f", test->green);
        }
        row.y += step;
        if (game_ui_slider(row, "Blue", test->blue, 0.0f, 1.0f)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Blue %.2f", test->blue);
        }
        row.y += step;
        if (game_ui_slider(row, "Size", test->size, GAME_UI_TEST_SIZE_MIN, GAME_UI_TEST_SIZE_MAX)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Size %.0f", test->size);
        }
        row.y += step;

        // drop downs, the shape list opens over the backdrop one
        if (game_ui_dropdown(row, "Shape", GAME_UI_TEST_SHAPES, GAME_UI_TEST_SHAPE_COUNT, test->shape)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Shape %s", GAME_UI_TEST_SHAPES[test->shape]);
        }
        row.y += step;
        if (game_ui_dropdown(row, "Backdrop", GAME_UI_TEST_BACKDROPS, GAME_UI_TEST_BACKDROP_COUNT, test->backdrop)) {
            (void)snprintf(test->last_event, sizeof(test->last_event), "Backdrop %s", GAME_UI_TEST_BACKDROPS[test->backdrop]);
        }
        row.y += step;

        // readouts
        (void)snprintf(text, sizeof(text), "Presses: %u", test->press_count);
        game_ui_label(row, text);
        row.y += step;
        (void)snprintf(text, sizeof(text), "Last: %s", test->last_event);
        game_ui_label(row, text);
        row.y += step;
        game_ui_set_text_color(GAME_UI_TEST_HINT_COLOR);
        game_ui_label(row, "Arrows+Enter work too");
    }

    // a box with the swatch centered in it, colored, sized, and shaped by the controls
    inline void
    game_scene_ui_test_draw_preview(
        const game_scene_ui_test* test,
        const rect&               area) {

        rect area_fill = area;
        area_fill.x      += GAME_UI_BORDER;
        area_fill.y      += GAME_UI_BORDER;
        area_fill.width  -= GAME_UI_BORDER * 2.0f;
        area_fill.height -= GAME_UI_BORDER * 2.0f;
        (void)eng_draw_rect_screen(area,      color_rgba_f32(GAME_UI_COLOR_BORDER));
        (void)eng_draw_rect_screen(area_fill, color_rgba_f32(GAME_UI_COLOR_TRACK));

        if (!test->show_preview) return;

        const f32 width  = test->size * GAME_UI_TEST_SHAPE_SCALE[(test->shape * 2) + 0];
        const f32 height = test->size * GAME_UI_TEST_SHAPE_SCALE[(test->shape * 2) + 1];

        rect swatch;
        swatch.width  = width;
        swatch.height = height;
        swatch.x      = area.x + ((area.width  - width)  * 0.5f);
        swatch.y      = area.y + ((area.height - height) * 0.5f);

        if (test->has_outline) {
            rect outline = swatch;
            outline.x      -= GAME_UI_BORDER * 2.0f;
            outline.y      -= GAME_UI_BORDER * 2.0f;
            outline.width  += GAME_UI_BORDER * 4.0f;
            outline.height += GAME_UI_BORDER * 4.0f;
            (void)eng_draw_rect_screen(outline, color_rgba_f32(0xFFFFFFFF));
        }

        color_rgba_f32 color;
        color.r = test->red;
        color.g = test->green;
        color.b = test->blue;
        color.a = 1.0f;
        (void)eng_draw_rect_screen(swatch, color);
    }
};
