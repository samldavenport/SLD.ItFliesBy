#pragma once

#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 FONT_FILTER_SIZE      = IFB_CONFIG_ASSET_NAME_SIZE + 1;
    static constexpr u32 FONT_NAME_SIZE        = IFB_CONFIG_ASSET_NAME_SIZE + 1;
    static constexpr u32 FONT_PATH_SIZE        = IFB_CONFIG_FILE_PATH_SIZE;
    static constexpr u32 FONT_SAMPLE_SIZE      = 512;
    static constexpr u32 FONT_ZOOM_MAX         = 8;
    static constexpr u32 FONT_PREVIEW_BG_COLOR = IM_COL32(32, 32, 32, 255);
    static constexpr u32 FONT_HOVER_COLOR      = IM_COL32(255, 255, 0, 255);

    static const char* _font_file_filter_type = "*.ttf";
    static const char* _font_file_filter_desc = "Font File";
    static const char* _font_file_ext         = ".ttf";
    static const char* _font_atlas_filter_type = "*.ifbfont";
    static const char* _font_atlas_filter_desc = "Font Atlas";
    static const char* _font_atlas_ext         = ".ifbfont";
    static const char* _font_sample_default   = "The quick brown fox jumps over the lazy dog.\nTHE QUICK BROWN FOX JUMPS OVER THE LAZY DOG!\n0123456789 #$%&*()[]{}<>?@ AV To Wa";

    // the font is baked into the window's own large arena, so opening another or baking again resets it
    // the preview texture is the engine's font preview slot
    //
    // h_arena has to start as INVALID_HANDLE, a zeroed handle is arena 0, which belongs to someone else
    struct font_viewer_state {
        const char*   status                     = NULL;
        char          path     [FONT_PATH_SIZE]   = {0}; // empty unless a font is open
        char          sample   [FONT_SAMPLE_SIZE] = {0};
        font*         fnt                        = NULL;
        hnd_arena     h_arena                    = INVALID_HANDLE;
        font_config   cfg;
        asset_preview preview                    = {0};
        bool          is_preview_loaded          = false;
        int           zoom_sample                = 2;
        int           zoom_atlas                 = 2;
        char          filter   [FONT_FILTER_SIZE] = {0};
        char          selected [FONT_NAME_SIZE]   = {0};
    } static _font_viewer_state;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void font_viewer_init     (void);
    IFB_INLINE void font_viewer_controls (void);
    IFB_INLINE void font_viewer_settings (void);
    IFB_INLINE void font_viewer_info     (void);
    IFB_INLINE void font_viewer_sample   (void);
    IFB_INLINE void font_viewer_atlas    (void);
    IFB_INLINE void font_file_open       (void);
    IFB_INLINE bool font_file_bake       (void);
    IFB_INLINE void font_file_close      (void);
    IFB_INLINE void font_file_save       (void);
    IFB_INLINE void font_viewer_glyph_tooltip (const u32 codepoint);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_font_viewer(
        bool& is_open) {

        auto& state = _font_viewer_state;

        if (!is_open) {

            // make sure our arena is freed
            if (state.h_arena != INVALID_HANDLE) {
                eng_arena_free(state.h_arena);
                state.fnt               = NULL;
                state.h_arena           = INVALID_HANDLE;
                state.path[0]           = 0;
                state.is_preview_loaded = false;
            }
            return;
        }

        font_viewer_init();

        const bool begin = ImGui::Begin(_gui_str->menu_engine_menu_tools_font_viewer, &is_open);
        if (begin) {

            // make sure we have a valid arena
            if (state.h_arena == INVALID_HANDLE) {
                state.h_arena = eng_arena_large_alloc();
                state.fnt     = NULL;
                assert(state.h_arena != INVALID_HANDLE);
            }

            font_viewer_controls ();
            font_viewer_settings ();
            font_viewer_info     ();
            font_viewer_sample   ();
            font_viewer_atlas    ();
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the sample text starts as the default the first time the window opens
    IFB_INLINE void
    font_viewer_init(
        void) {

        auto& state = _font_viewer_state;

        static bool is_init = false;
        if (is_init) return;
        is_init = true;

        strncpy_s(state.sample, _font_sample_default, FONT_SAMPLE_SIZE - 1);
    }

    // open and close the font
    IFB_INLINE void
    font_viewer_controls(
        void) {

        auto& state = _font_viewer_state;

        ImGui::SeparatorText("Font");

        if (ImGui::Button("Open...")) {
            font_file_open();
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(state.fnt == NULL);
        if (ImGui::Button("Save...")) {
            font_file_save();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            font_file_close();
        }
        ImGui::EndDisabled();

        ImGui::TextWrapped("%s", (state.path[0] != 0) ? state.path : "(none)");

        if (state.status) {
            ImGui::TextWrapped("%s", state.status);
        }
    }

    // what the font is baked with, baking again reads the ttf again
    IFB_INLINE void
    font_viewer_settings(
        void) {

        auto& state = _font_viewer_state;

        ImGui::SeparatorText("Bake");

        const ImU32 step_one = 1;
        int oversample_x = (int)state.cfg.oversample_x;
        int oversample_y = (int)state.cfg.oversample_y;

        ImGui::PushItemWidth(ImGui::GetFontSize() * 10.0f);
        ImGui::InputFloat  ("Pixel Height",    &state.cfg.pixel_height, 1.0f, 8.0f, "%.1f");
        ImGui::InputScalar ("First Codepoint", ImGuiDataType_U32, &state.cfg.first_codepoint, &step_one);
        ImGui::InputScalar ("Glyph Count",     ImGuiDataType_U32, &state.cfg.glyph_count,     &step_one);
        ImGui::SliderInt   ("Oversample X",    &oversample_x, 1, 8);
        ImGui::SliderInt   ("Oversample Y",    &oversample_y, 1, 8);
        ImGui::PopItemWidth();

        state.cfg.oversample_x = (u32)oversample_x;
        state.cfg.oversample_y = (u32)oversample_y;

        ImGui::BeginDisabled(state.path[0] == 0);
        if (ImGui::Button("Bake")) {
            state.status = font_file_bake()
                ? "Baked the font"
                : "Failed to bake the font, check the settings, it may not fit in a 4096 x 4096 atlas";
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Defaults")) {
            state.cfg = font_config();
        }
    }

    // the numbers the blob was baked to
    IFB_INLINE void
    font_viewer_info(
        void) {

        auto& state = _font_viewer_state;
        const font* fnt = state.fnt;
        if (!fnt) return;

        ImGui::SeparatorText("Details");

        ImGui::Text("Blob        %u bytes", fnt->size);
        ImGui::Text("Atlas       %u x %u",  fnt->atlas_width, fnt->atlas_height);
        ImGui::Text("Glyphs      %u (U+%04X to U+%04X)", fnt->glyph_count, fnt->first_codepoint, fnt->first_codepoint + fnt->glyph_count - 1);
        ImGui::Text("Kern pairs  %u",       fnt->kern_pair_count);
        ImGui::Text("Oversample  %u x %u",  fnt->oversample_x, fnt->oversample_y);
        ImGui::Text("Ascent %.2f  Descent %.2f  Line gap %.2f  Line height %.2f", fnt->ascent, fnt->descent, fnt->line_gap, fnt->line_height);
    }

    // the sample text drawn glyph by glyph from the blob, the way the game would draw it
    IFB_INLINE void
    font_viewer_sample(
        void) {

        auto& state = _font_viewer_state;
        const font* fnt = state.fnt;
        if (!fnt || !state.is_preview_loaded) return;

        ImGui::SeparatorText("Sample");

        ImGui::InputTextMultiline("##font-sample", state.sample, FONT_SAMPLE_SIZE, ImVec2(-1.0f, ImGui::GetTextLineHeight() * 4.0f));
        ImGui::SliderInt("Scale##sample", &state.zoom_sample, 1, FONT_ZOOM_MAX, "%dx");

        const f32 scale = (f32)state.zoom_sample;

        // the size of the text, so the window can lay it out and scroll it
        u32 line_count = 1;
        f32 line_width = 0.0f;
        f32 max_width  = 0.0f;
        for (const char* c = state.sample; *c; ++c) {
            if (*c == '\n') {
                ++line_count;
                line_width = 0.0f;
                continue;
            }
            const font_glyph* g = fnt->glyph((u32)(u8)*c);
            if (g) line_width += g->advance + fnt->kerning((u32)(u8)c[0], (u32)(u8)c[1]);
            if (line_width > max_width) max_width = line_width;
        }

        const f32    padding = 8.0f;
        const ImVec2 size(
            (max_width                         * scale) + (padding * 2.0f),
            ((fnt->line_height * (f32)line_count) * scale) + (padding * 2.0f)
        );

        if (ImGui::BeginChild("font-sample-view", ImVec2(0.0f, size.y + ImGui::GetStyle().ScrollbarSize), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {

            const ImVec2 origin    = ImGui::GetCursorScreenPos();
            ImDrawList*  draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + size.y), FONT_PREVIEW_BG_COLOR);

            const ImTextureID texture = (ImTextureID)(intptr_t)state.preview.texture;
            const f32         atlas_w = (f32)fnt->atlas_width;
            const f32         atlas_h = (f32)fnt->atlas_height;
            const ImU32       color   = ImGui::GetColorU32(ImGuiCol_Text);

            // the pen starts on the first line's baseline
            f32 pen_x = 0.0f;
            f32 pen_y = fnt->ascent;

            for (const char* c = state.sample; *c; ++c) {

                if (*c == '\n') {
                    pen_x  = 0.0f;
                    pen_y += fnt->line_height;
                    continue;
                }

                const u32         codepoint = (u32)(u8)c[0];
                const font_glyph* g         = fnt->glyph(codepoint);
                if (!g) continue;

                // whole screen pixels, so a pixel font scaled by a whole number stays sharp
                const ImVec2 p_min(
                    floorf(origin.x + padding + ((pen_x + g->quad_x0) * scale)),
                    floorf(origin.y + padding + ((pen_y + g->quad_y0) * scale))
                );
                const ImVec2 p_max(
                    p_min.x + ((g->quad_x1 - g->quad_x0) * scale),
                    p_min.y + ((g->quad_y1 - g->quad_y0) * scale)
                );
                const ImVec2 uv_min((f32)g->atlas_x0 / atlas_w, (f32)g->atlas_y0 / atlas_h);
                const ImVec2 uv_max((f32)g->atlas_x1 / atlas_w, (f32)g->atlas_y1 / atlas_h);

                draw_list->AddImage(texture, p_min, p_max, uv_min, uv_max, color);

                pen_x += g->advance + fnt->kerning(codepoint, (u32)(u8)c[1]);
            }

            ImGui::Dummy(size);
        }
        ImGui::EndChild();
    }

    // the whole atlas, the glyph under the mouse is outlined and described
    IFB_INLINE void
    font_viewer_atlas(
        void) {

        auto& state = _font_viewer_state;
        const font* fnt = state.fnt;

        ImGui::SeparatorText("Atlas");

        if (!fnt || !state.is_preview_loaded) {
            ImGui::TextUnformatted("No font open");
            return;
        }

        ImGui::SliderInt("Zoom##atlas", &state.zoom_atlas, 1, FONT_ZOOM_MAX, "%dx");

        const f32    scale = (f32)state.zoom_atlas;
        const ImVec2 size((f32)fnt->atlas_width * scale, (f32)fnt->atlas_height * scale);

        if (ImGui::BeginChild("font-atlas-view", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {

            const ImVec2 image_min = ImGui::GetCursorScreenPos();
            ImDrawList*  draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(image_min, ImVec2(image_min.x + size.x, image_min.y + size.y), FONT_PREVIEW_BG_COLOR);

            ImGui::Image((ImTextureID)(intptr_t)state.preview.texture, size);

            if (ImGui::IsItemHovered()) {

                const ImVec2 mouse   = ImGui::GetMousePos();
                const f32    atlas_x = (mouse.x - image_min.x) / scale;
                const f32    atlas_y = (mouse.y - image_min.y) / scale;

                // the glyphs don't overlap, so the first one that has the pixel is the only one
                const font_glyph* glyphs = fnt->glyphs();
                for (u32 i = 0; i < fnt->glyph_count; ++i) {

                    const font_glyph& g = glyphs[i];
                    const bool is_inside = (
                        atlas_x >= (f32)g.atlas_x0 && atlas_x < (f32)g.atlas_x1 &&
                        atlas_y >= (f32)g.atlas_y0 && atlas_y < (f32)g.atlas_y1
                    );
                    if (!is_inside) continue;

                    draw_list->AddRect(
                        ImVec2(image_min.x + ((f32)g.atlas_x0 * scale), image_min.y + ((f32)g.atlas_y0 * scale)),
                        ImVec2(image_min.x + ((f32)g.atlas_x1 * scale), image_min.y + ((f32)g.atlas_y1 * scale)),
                        FONT_HOVER_COLOR
                    );
                    font_viewer_glyph_tooltip(fnt->first_codepoint + i);
                    break;
                }
            }
        }
        ImGui::EndChild();
    }

    IFB_INLINE void
    font_viewer_glyph_tooltip(
        const u32 codepoint) {

        const font*       fnt = _font_viewer_state.fnt;
        const font_glyph* g   = fnt->glyph(codepoint);
        if (!g) return;

        if (ImGui::BeginTooltip()) {
            if (codepoint >= 32 && codepoint < 127) ImGui::Text("'%c'  U+%04X", (char)codepoint, codepoint);
            else                                    ImGui::Text("U+%04X", codepoint);
            ImGui::Separator();
            ImGui::Text("Atlas    %u, %u to %u, %u", g->atlas_x0, g->atlas_y0, g->atlas_x1, g->atlas_y1);
            ImGui::Text("Quad     %.2f, %.2f to %.2f, %.2f", g->quad_x0, g->quad_y0, g->quad_x1, g->quad_y1);
            ImGui::Text("Advance  %.2f", g->advance);
            ImGui::EndTooltip();
        }
    }

    // a cancelled or failed open leaves the open font as it was
    IFB_INLINE void
    font_file_open(
        void) {

        auto& state = _font_viewer_state;

        const char* path = eng_file_dialog_open(_font_file_filter_type, _font_file_filter_desc);
        if (!path) return;

        // the dialog's buffer is reused, and a failed bake keeps the old path
        char path_prev[FONT_PATH_SIZE];
        strncpy_s(path_prev,  state.path, FONT_PATH_SIZE - 1);
        strncpy_s(state.path, path,       FONT_PATH_SIZE - 1);

        if (!font_file_bake()) {
            strncpy_s(state.path, path_prev, FONT_PATH_SIZE - 1);
            if (state.path[0] != 0) (void)font_file_bake();
            state.status = "Failed to open the font, it may not be a ttf, or it doesn't fit with these settings";
            return;
        }

        state.status = "Opened the font";
    }

    // reads the ttf at the path and bakes it with the settings, replacing the open font
    IFB_INLINE bool
    font_file_bake(
        void) {

        auto& state = _font_viewer_state;

        assert(state.h_arena != INVALID_HANDLE);

        state.fnt               = NULL;
        state.is_preview_loaded = false;
        eng_arena_reset(state.h_arena);

        if (state.path[0] == 0) return(false);

        const hnd_file h_file = eng_file_ro_open_existing(state.path);
        if (h_file == 0) return(false);

        state.fnt = eng_font_load_to_arena(h_file, state.h_arena, state.cfg);
        eng_file_close(h_file);
        if (!state.fnt) return(false);

        state.is_preview_loaded = eng_font_load_preview(state.fnt, state.preview);
        return(state.is_preview_loaded);
    }

    // saves the font as it was baked, so what's saved is what the preview shows
    // the file goes in the asset dictionary as a font atlas
    IFB_INLINE void
    font_file_save(
        void) {

        auto& state = _font_viewer_state;
        if (!state.fnt) return;

        const char* dlg_path = eng_file_dialog_save(_font_atlas_filter_type, _font_atlas_filter_desc);
        if (!dlg_path) return;

        char path[FONT_PATH_SIZE];
        strncpy_s(path, dlg_path, FONT_PATH_SIZE - 1);
        asset_database_add_extension(path, _font_atlas_ext);

        state.status = eng_font_save(state.fnt, path)
            ? "Saved the font atlas, add it to the asset dictionary as a Font Atlas"
            : "Failed to save the font atlas, the file may not be writable";
    }

    IFB_INLINE void
    font_file_close(
        void) {

        auto& state = _font_viewer_state;

        eng_arena_reset(state.h_arena);
        state.fnt               = NULL;
        state.path[0]           = 0;
        state.is_preview_loaded = false;
        state.status            = "Closed the font";
    }
};
