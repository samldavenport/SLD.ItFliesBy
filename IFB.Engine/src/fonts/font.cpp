#pragma once

#include "eng-font.hpp"
#include "eng-memory.hpp"
#include "ifb-engine.hpp"
#include <stb_truetype.h>
#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // INLINE  
    //--------------------------------------------------------------------

    inline bool
    font_config_is_valid(
        const font_config& cfg) {

        const bool is_valid = (
            cfg.pixel_height >  0.0f                 &&
            cfg.glyph_count  != 0                    &&
            cfg.glyph_count  <= 0x10000              && // kern keys hold glyph indexes in 16 bits
            cfg.oversample_x >= 1                    &&
            cfg.oversample_x <= FONT_OVERSAMPLE_MAX  &&
            cfg.oversample_y >= 1                    &&
            cfg.oversample_y <= FONT_OVERSAMPLE_MAX
        );
        return(is_valid);
    }

    // the number of glyph pairs in the range that the font kerns
    inline u32
    font_count_kern_pairs(
        const stbtt_fontinfo& info,
        const font_config&    cfg) {

        // no kern or GPOS table, nothing to look up
        if (info.kern == 0 && info.gpos == 0) return(0);

        u32 count = 0;
        for (u32 i_first = 0; i_first < cfg.glyph_count; ++i_first) {
            for (u32 i_second = 0; i_second < cfg.glyph_count; ++i_second) {

                const int kern = stbtt_GetCodepointKernAdvance(
                    &info,
                    (int)(cfg.first_codepoint + i_first),
                    (int)(cfg.first_codepoint + i_second)
                );
                if (kern != 0) ++count;
            }
        }
        return(count);
    }

    // the smallest power of two atlas that could hold every glyph, growing the width
    // and height in turns, the packer grows it from there if they don't fit
    inline void
    font_atlas_start_size(
        const font_config& cfg,
        u32&               width,
        u32&               height) {

        const u32 cell_w = (u32)(cfg.pixel_height * (f32)cfg.oversample_x) + FONT_ATLAS_PADDING;
        const u32 cell_h = (u32)(cfg.pixel_height * (f32)cfg.oversample_y) + FONT_ATLAS_PADDING;
        const u64 area   = (u64)cell_w * (u64)cell_h * (u64)cfg.glyph_count;

        width  = 32;
        height = 32;
        while (((u64)width * (u64)height) < area && height < FONT_ATLAS_SIZE_MAX) {
            if (width <= height) width  *= 2;
            else                 height *= 2;
        }
    }

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // bakes the ttf into a font blob (see struct font) at the arena spot the call started at
    //
    // the ttf is read onto the arena first and the blob is built after it, then the blob
    // is moved down over the ttf and the rest is given back, so only the blob is left.
    // stb_truetype also uses malloc for its packer and rasterizer while it bakes
    IFB_INTERNAL font*
    font_load_to_arena(
        const hnd_file     h_font_file,
        const hnd_arena    h_arena,
        const font_config& cfg) {

        assert(h_font_file != INVALID_HANDLE);
        assert(h_arena     != INVALID_HANDLE);

        if (!font_config_is_valid(cfg)) return(NULL);

        // save the arena spot, the blob ends up here
        const u32 save_start = arena_save(h_arena);

        // load the ttf
        const memory ttf = eng_file_read_to_arena(h_font_file, h_arena);
        if (ttf.ptr == NULL || ttf.size == 0) {
            arena_revert(h_arena, save_start);
            return(NULL);
        }

        // parse the font
        stbtt_fontinfo info;
        const int font_offset = stbtt_GetFontOffsetForIndex(ttf.bytes, 0);
        if (font_offset < 0 || stbtt_InitFont(&info, ttf.bytes, font_offset) == 0) {
            arena_revert(h_arena, save_start);
            return(NULL);
        }

        // vertical metrics, scaled to pixels
        const f32 scale = stbtt_ScaleForPixelHeight(&info, cfg.pixel_height);
        int ascent   = 0;
        int descent  = 0;
        int line_gap = 0;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

        // blob layout, everything but the atlas is a fixed size
        const u32 kern_pair_count  = font_count_kern_pairs(info, cfg);
        const u32 glyph_offset     = sizeof(font);
        const u32 kern_pair_offset = glyph_offset     + (cfg.glyph_count * sizeof(font_glyph));
        const u32 atlas_offset     = kern_pair_offset + (kern_pair_count * sizeof(font_kern_pair));

        // pack the glyphs, growing the atlas until they fit
        // the blob and stb's packed chars are pushed after the ttf, and given back on every retry
        const u32         save_after_ttf = arena_save(h_arena);
        font*             fnt            = NULL;
        stbtt_packedchar* packed         = NULL;
        u32               atlas_w        = 0;
        u32               atlas_h        = 0;
        bool              is_packed      = false;
        font_atlas_start_size(cfg, atlas_w, atlas_h);

        while (atlas_w <= FONT_ATLAS_SIZE_MAX && atlas_h <= FONT_ATLAS_SIZE_MAX) {

            arena_revert(h_arena, save_after_ttf);

            const u32 blob_size = atlas_offset + (atlas_w * atlas_h);
            fnt    = (font*)arena_push(h_arena, blob_size);
            packed = arena_push<stbtt_packedchar>(h_arena, cfg.glyph_count);
            if (fnt == NULL || packed == NULL) break; // out of arena, a bigger atlas won't help

            byte* atlas_pixels = ((byte*)fnt) + atlas_offset;

            // clears the atlas
            stbtt_pack_context pack;
            if (stbtt_PackBegin(&pack, atlas_pixels, (int)atlas_w, (int)atlas_h, 0, (int)FONT_ATLAS_PADDING, NULL) == 0) {
                break; // stb couldn't allocate
            }
            stbtt_PackSetOversampling(&pack, cfg.oversample_x, cfg.oversample_y);

            is_packed = (stbtt_PackFontRange(
                &pack,
                ttf.bytes,
                0,
                cfg.pixel_height,
                (int)cfg.first_codepoint,
                (int)cfg.glyph_count,
                packed
            ) != 0);

            stbtt_PackEnd(&pack);
            if (is_packed) break;

            if (atlas_w <= atlas_h) atlas_w *= 2;
            else                    atlas_h *= 2;
        }

        if (!is_packed) {
            arena_revert(h_arena, save_start);
            return(NULL);
        }

        // header
        const u32 blob_size = atlas_offset + (atlas_w * atlas_h);
        fnt->size             = blob_size;
        fnt->pixel_height     = cfg.pixel_height;
        fnt->ascent           = (f32)ascent   * scale;
        fnt->descent          = (f32)descent  * scale;
        fnt->line_gap         = (f32)line_gap * scale;
        fnt->line_height      = fnt->ascent - fnt->descent + fnt->line_gap;
        fnt->oversample_x     = cfg.oversample_x;
        fnt->oversample_y     = cfg.oversample_y;
        fnt->first_codepoint  = cfg.first_codepoint;
        fnt->glyph_count      = cfg.glyph_count;
        fnt->glyph_offset     = glyph_offset;
        fnt->kern_pair_count  = kern_pair_count;
        fnt->kern_pair_offset = kern_pair_offset;
        fnt->atlas_width      = atlas_w;
        fnt->atlas_height     = atlas_h;
        fnt->atlas_offset     = atlas_offset;

        // glyphs
        font_glyph* glyphs = (font_glyph*)(((byte*)fnt) + glyph_offset);
        for (u32 i = 0; i < cfg.glyph_count; ++i) {

            const stbtt_packedchar& pc = packed[i];
            font_glyph&             g  = glyphs[i];
            g.atlas_x0 = pc.x0;
            g.atlas_y0 = pc.y0;
            g.atlas_x1 = pc.x1;
            g.atlas_y1 = pc.y1;
            g.quad_x0  = pc.xoff;
            g.quad_y0  = pc.yoff;
            g.quad_x1  = pc.xoff2;
            g.quad_y1  = pc.yoff2;
            g.advance  = pc.xadvance;
        }

        // kerning, looping first then second keeps the keys sorted
        if (kern_pair_count != 0) {

            font_kern_pair* pairs  = (font_kern_pair*)(((byte*)fnt) + kern_pair_offset);
            u32             i_pair = 0;
            for (u32 i_first = 0; i_first < cfg.glyph_count; ++i_first) {
                for (u32 i_second = 0; i_second < cfg.glyph_count; ++i_second) {

                    const int kern = stbtt_GetCodepointKernAdvance(
                        &info,
                        (int)(cfg.first_codepoint + i_first),
                        (int)(cfg.first_codepoint + i_second)
                    );
                    if (kern == 0) continue;

                    assert(i_pair < kern_pair_count);
                    pairs[i_pair].key    = font::kern_key(i_first, i_second);
                    pairs[i_pair].amount = (f32)kern * scale;
                    ++i_pair;
                }
            }
            assert(i_pair == kern_pair_count);
        }

        // move the blob down over the ttf, and give back everything after it
        (void)memmove((void*)ttf.ptr, (void*)fnt, blob_size);
        arena_revert(h_arena, save_start);

        font* result = (font*)arena_push(h_arena, blob_size);
        assert(result == (font*)ttf.ptr);

        return(result);
    }
};
