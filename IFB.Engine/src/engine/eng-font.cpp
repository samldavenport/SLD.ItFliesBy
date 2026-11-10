#pragma once

#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "eng-font.hpp"
#include "eng-memory.hpp"
#include "eng-renderer.hpp"

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
};
