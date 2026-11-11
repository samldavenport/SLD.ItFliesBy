#pragma once

#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "eng-font.hpp"
#include "eng-memory.hpp"
#include "eng-renderer.hpp"
#include "eng-asset.hpp"

namespace ifb {

    IFB_ENGINE_API font*
    eng_font_load_to_arena(
        const hnd_file     h_font_file,
        const hnd_arena    h_arena,
        const font_config& cfg) {

        assert(h_font_file != INVALID_HANDLE);
        assert(h_arena     != INVALID_HANDLE);

        font* f = font_load_to_arena(h_font_file, h_arena, cfg);

        return(f);
    }

    // the font's atlas as an rgba image on the arena, white with the coverage as the alpha,
    // so it can be uploaded like any other image and tinted any color when it's drawn
    // the pixels go right after the image struct, NULL if the arena doesn't have room
    IFB_ENGINE_API image*
    eng_font_glyph_table_to_image(
        const font*     fnt,
        const hnd_arena h_arena) {

        assert(fnt     != NULL);
        assert(h_arena != INVALID_HANDLE);

        const u32 width       = fnt->atlas_width;
        const u32 height      = fnt->atlas_height;
        const u32 pixel_count = width * height;
        const u32 size_pixels = pixel_count * 4;

        auto img = (image*)arena_push(h_arena, sizeof(image) + size_pixels);
        if (img == NULL) return(NULL);

        img->data.address = ((addr)img) + sizeof(image);
        img->data.size    = size_pixels;
        img->pixel_width  = width;
        img->pixel_height = height;

        const byte* coverage = fnt->atlas_pixels();
        byte*       rgba     = img->data.bytes;
        for (u32 i = 0; i < pixel_count; ++i) {
            rgba[0] = 0xFF;
            rgba[1] = 0xFF;
            rgba[2] = 0xFF;
            rgba[3] = coverage[i];
            rgba += 4;
        }

        return(img);
    }

    // uploads the font's atlas to the font viewer's preview texture
    // a font without oversampling is filtered nearest, so pixel fonts stay sharp when they're scaled up
    IFB_ENGINE_API bool
    eng_font_load_preview(
        const font*    fnt,
        asset_preview& preview) {

        assert(fnt != NULL);

        eng_asset_preview_clear(preview);

        const hnd_arena h_arena = eng_asset_preview_get_arena(eng_asset_preview_e_font);
        if (h_arena == INVALID_HANDLE) return(false);

        const image* img = eng_font_glyph_table_to_image(fnt, h_arena);
        if (img == NULL) return(false);

        const bool is_nearest = (fnt->oversample_x == 1 && fnt->oversample_y == 1);
        preview.texture = renderer_set_asset_preview_texture(
            eng_asset_preview_e_font,
            img->pixel_width,
            img->pixel_height,
            img->data.bytes,
            is_nearest
        );
        preview.width  = img->pixel_width;
        preview.height = img->pixel_height;

        // the texture has the pixels now
        arena_reset(h_arena);

        return(preview.texture != 0);
    }

    // writes the font to an .ifbfont file, which can go in the asset dictionary as a font atlas
    // false if the font isn't valid or the file can't be written
    IFB_ENGINE_API bool
    eng_font_save(
        const font*  fnt,
        const cchar* path) {

        assert(fnt);
        assert(path);

        return(font_file_save(fnt, path));
    }

    // reads an .ifbfont file onto the arena
    // NULL if it can't be opened, isn't a font file, the font in it isn't valid, or there's no room
    IFB_ENGINE_API font*
    eng_font_load_file_to_arena(
        const cchar*    path,
        const hnd_arena h_arena) {

        assert(path);
        assert(h_arena != INVALID_HANDLE);

        return(font_file_load_to_arena(path, h_arena));
    }

    // copies a font atlas from the runtime database onto the arena
    // the copy is the caller's, so unlike asset memory it doesn't move or get evicted
    // NULL if there's no font atlas by that name, or no room
    IFB_ENGINE_API font*
    eng_font_atlas_load_to_arena(
        const cchar*    name,
        const hnd_arena h_arena) {

        assert(name);
        assert(h_arena != INVALID_HANDLE);

        const asset_database* db = asset_mngr_get_database();
        if (!db || name[0] == 0) return(NULL);

        const asset_font_atlas* atlas = asset_database_load_font_atlas_by_name_to_arena(db, name, h_arena);
        if (!atlas) return(NULL);

        return((font*)&atlas->fnt);
    }
    //--------------------------------------------------------------------
    // GPU
    //--------------------------------------------------------------------

    // true if the font is on the gpu, uploading it again does nothing
    // false if the renderer has no room for another font
    IFB_ENGINE_API bool
    eng_font_upload(
        const font* fnt) {

        assert(fnt != NULL);

        const bool did_upload = renderer_upload_font(fnt);
        return(did_upload);
    }

    // false if the font was never uploaded
    IFB_ENGINE_API bool
    eng_font_release(
        const font* fnt) {

        assert(fnt != NULL);

        const bool did_release = renderer_release_font(fnt);
        return(did_release);
    }

    // false if the font isn't uploaded
    IFB_ENGINE_API bool
    eng_font_set_active(
        const font* fnt) {

        assert(fnt != NULL);

        const bool did_set = renderer_set_active_font(fnt);
        return(did_set);
    }

    // NULL if no font is active
    IFB_ENGINE_API const font*
    eng_font_get_active(
        void) {

        const font* fnt = renderer_get_active_font();
        return(fnt);
    }

    //--------------------------------------------------------------------
    // LAYOUT
    //--------------------------------------------------------------------

    // how far the pen moves for a character, 0 for characters the font doesn't have
    inline f32
    font_char_advance(
        const font* fnt,
        const cchar c) {

        const font_glyph* g = fnt->glyph((u32)(u8)c);
        return((g != NULL) ? g->advance : 0.0f);
    }

    IFB_ENGINE_API f32
    eng_font_measure_text(
        const font*  fnt,
        const cchar* str,
        const u32    len) {

        assert(fnt != NULL);
        assert(str != NULL || len == 0);

        f32 width_max  = 0.0f;
        f32 width_line = 0.0f;
        for (
              u32 i = 0;
                  i < len;
                ++i) {

            const cchar c = str[i];
            if (c == '\n') {
                width_line = 0.0f;
                continue;
            }

            // kerning only applies between characters on the same line
            if (i > 0 && str[i - 1] != '\n') {
                width_line += fnt->kerning((u32)(u8)str[i - 1], (u32)(u8)c);
            }
            width_line += font_char_advance(fnt, c);
            if (width_line > width_max) width_max = width_line;
        }
        return(width_max);
    }

    IFB_ENGINE_API u32
    eng_font_wrap_text(
        const font*  fnt,
        const cchar* str,
        const u32    len,
        const f32    max_width,
        font_line*   lines,
        const u32    line_capacity) {

        assert(fnt != NULL);
        assert(str != NULL || len == 0);
        assert(lines != NULL || line_capacity == 0);

        u32 line_count = 0;
        u32 i          = 0;
        while (i < len) {

            const u32 start      = i;
            f32       pen        = 0.0f;
            u32       break_at   = len; // the last space on the line, len for none
            f32       break_pen  = 0.0f; // the line's width before that space
            u32       line_len   = 0;
            f32       line_width = 0.0f;
            u32       next       = len;

            u32 j = start;
            for (;;) {

                // the end of the text, or a newline, ends the line where it is
                if (j == len || str[j] == '\n') {
                    line_len   = j - start;
                    line_width = pen;
                    next       = (j == len) ? len : j + 1;
                    break;
                }

                const cchar c       = str[j];
                const f32   kerning = (j > start) ? fnt->kerning((u32)(u8)str[j - 1], (u32)(u8)c) : 0.0f;
                const f32   advance = font_char_advance(fnt, c);

                if (c == ' ') {
                    break_at  = j;
                    break_pen = pen;
                }
                else if (j > start && (pen + kerning + advance) > max_width) {

                    // out of room, break at the last space, or in the word if there isn't one
                    if (break_at != len) {
                        line_len   = break_at - start;
                        line_width = break_pen;
                        next       = break_at + 1;
                    }
                    else {
                        line_len   = j - start;
                        line_width = pen;
                        next       = j;
                    }

                    // the spaces where the line broke aren't drawn on the next line either
                    while (next < len && str[next] == ' ') ++next;
                    break;
                }

                pen += kerning + advance;
                ++j;
            }

            // trailing spaces don't count toward the width
            while (line_len > 0 && str[start + line_len - 1] == ' ') {
                --line_len;
                line_width -= font_char_advance(fnt, ' ');
            }

            if (line_count < line_capacity) {
                lines[line_count].start  = start;
                lines[line_count].length = line_len;
                lines[line_count].width  = (line_width > 0.0f) ? line_width : 0.0f;
            }
            ++line_count;
            i = next;
        }

        return(line_count);
    }
};
