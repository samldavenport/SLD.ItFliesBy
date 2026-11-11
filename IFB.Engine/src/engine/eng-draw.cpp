#pragma once

#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "eng-quad.hpp"
#include "eng-renderer.hpp"
#include "eng-sprite.hpp"
#include <cassert>
#include <math.h>

namespace ifb {

    //--------------------------------------------------------------------
    // INTERNAL
    //--------------------------------------------------------------------

    // a quad's texture coordinates, the top row of an atlas is v = 0
    struct draw_uv {
        f32 u_left;
        f32 v_top;
        f32 u_right;
        f32 v_bottom;
    };

    inline void
    draw_set_vertex(
        quad_vertex&          v,
        const f32             x,
        const f32             y,
        const f32             z,
        const color_rgba_f32& color,
        const f32             u,
        const f32             tv,
        const f32             sample) {

        v.position.x    = x;
        v.position.y    = y;
        v.position.z    = z;
        v.color         = color;
        v.text_coords.x = u;
        v.text_coords.y = tv;
        v.textured      = sample;
    }

    // a screen quad, y goes down so the top edge is rect.y
    inline bool
    draw_push_screen_quad(
        const draw_rect&      rect,
        const color_rgba_f32& color,
        const draw_uv&        uv,
        const f32             sample) {

        const f32 x0 = rect.x;
        const f32 y0 = rect.y;
        const f32 x1 = rect.x + rect.width;
        const f32 y1 = rect.y + rect.height;

        quad_vertices q;
        draw_set_vertex(q.top_right,    x1, y0, 0.0f, color, uv.u_right, uv.v_top,    sample);
        draw_set_vertex(q.bottom_right, x1, y1, 0.0f, color, uv.u_right, uv.v_bottom, sample);
        draw_set_vertex(q.bottom_left,  x0, y1, 0.0f, color, uv.u_left,  uv.v_bottom, sample);
        draw_set_vertex(q.top_left,     x0, y0, 0.0f, color, uv.u_left,  uv.v_top,    sample);

        const bool did_push = renderer_immediate_push_screen(q);
        return(did_push);
    }

    // the point on the 1 x 1 quad, moved by the xform
    // the matrices are row major and the vectors are columns, translation is the last column
    inline vec3
    draw_xform_point(
        const mat4& m,
        const f32   x,
        const f32   y) {

        vec3 p;
        p.x = (m.r0c0 * x) + (m.r0c1 * y) + m.r0c3;
        p.y = (m.r1c0 * x) + (m.r1c1 * y) + m.r1c3;
        p.z = (m.r2c0 * x) + (m.r2c1 * y) + m.r2c3;
        return(p);
    }

    // a world quad, the 1 x 1 quad on the xy plane centered on the origin, moved by the xform
    inline bool
    draw_push_world_quad(
        const mat4&           xform,
        const color_rgba_f32& color,
        const draw_uv&        uv,
        const f32             sample) {

        const vec3 tr = draw_xform_point(xform,  0.5f,  0.5f);
        const vec3 br = draw_xform_point(xform,  0.5f, -0.5f);
        const vec3 bl = draw_xform_point(xform, -0.5f, -0.5f);
        const vec3 tl = draw_xform_point(xform, -0.5f,  0.5f);

        quad_vertices q;
        draw_set_vertex(q.top_right,    tr.x, tr.y, tr.z, color, uv.u_right, uv.v_top,    sample);
        draw_set_vertex(q.bottom_right, br.x, br.y, br.z, color, uv.u_right, uv.v_bottom, sample);
        draw_set_vertex(q.bottom_left,  bl.x, bl.y, bl.z, color, uv.u_left,  uv.v_bottom, sample);
        draw_set_vertex(q.top_left,     tl.x, tl.y, tl.z, color, uv.u_left,  uv.v_top,    sample);

        const bool did_push = renderer_immediate_push_world(q);
        return(did_push);
    }

    // the uvs of a region of a sprite in the active atlas, region in the sprite's pixels
    // NULL region is the whole sprite
    // false if the atlas isn't the active one or the sprite isn't in it
    inline bool
    draw_get_sprite_uv(
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        const draw_rect*       region,
        draw_uv&               uv) {

        if (h_atlas == INVALID_HANDLE)                    return(false);
        if (h_atlas != renderer_get_active_sprite_atlas()) return(false);

        sprite_info info;
        if (!sprite_mngr_get_sprite_info_by_index(h_atlas, i_sprite, info)) return(false);
        if (info.atlas_width == 0 || info.atlas_height == 0)                return(false);

        f32 x0 = (f32)info.pixel_x;
        f32 y0 = (f32)info.pixel_y;
        f32 x1 = x0 + (f32)info.pixel_width;
        f32 y1 = y0 + (f32)info.pixel_height;
        if (region != NULL) {
            x0 = (f32)info.pixel_x + region->x;
            y0 = (f32)info.pixel_y + region->y;
            x1 = x0 + region->width;
            y1 = y0 + region->height;
        }

        const f32 atlas_width  = (f32)info.atlas_width;
        const f32 atlas_height = (f32)info.atlas_height;
        uv.u_left   = x0 / atlas_width;
        uv.v_top    = y0 / atlas_height;
        uv.u_right  = x1 / atlas_width;
        uv.v_bottom = y1 / atlas_height;
        return(true);
    }

    static constexpr draw_uv DRAW_UV_NONE = { 0.0f, 0.0f, 1.0f, 1.0f };

    //--------------------------------------------------------------------
    // SCREEN SIZE
    //--------------------------------------------------------------------

    IFB_ENGINE_API void
    eng_draw_set_screen_size(
        const u32 width,
        const u32 height) {

        renderer_set_screen_size(width, height);
    }

    IFB_ENGINE_API void
    eng_draw_get_screen_size(
        u32& width,
        u32& height) {

        renderer_get_screen_size(width, height);
    }

    //--------------------------------------------------------------------
    // SCREEN
    //--------------------------------------------------------------------

    IFB_ENGINE_API bool
    eng_draw_rect_screen(
        const draw_rect&      rect,
        const color_rgba_f32& color) {

        const bool did_push = draw_push_screen_quad(rect, color, DRAW_UV_NONE, QUAD_SAMPLE_SOLID);
        return(did_push);
    }

    IFB_ENGINE_API bool
    eng_draw_sprite_screen(
        const draw_rect&       rect,
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        const color_rgba_f32&  tint) {

        draw_uv uv;
        if (!draw_get_sprite_uv(h_atlas, i_sprite, NULL, uv)) return(false);

        const bool did_push = draw_push_screen_quad(rect, tint, uv, QUAD_SAMPLE_SPRITE);
        return(did_push);
    }

    IFB_ENGINE_API bool
    eng_draw_sprite_region_screen(
        const draw_rect&       rect,
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        const draw_rect&       region,
        const color_rgba_f32&  tint) {

        draw_uv uv;
        if (!draw_get_sprite_uv(h_atlas, i_sprite, &region, uv)) return(false);

        const bool did_push = draw_push_screen_quad(rect, tint, uv, QUAD_SAMPLE_SPRITE);
        return(did_push);
    }

    // each glyph is placed from the pen on the baseline, the first baseline is the font's ascent below pos
    // pixel fonts (no oversampling) are snapped to whole screen pixels so they stay sharp
    IFB_ENGINE_API bool
    eng_draw_text_screen(
        const font*           fnt,
        const cchar*          str,
        const u32             len,
        const vec2&           pos,
        const color_rgba_f32& color) {

        assert(fnt != NULL);
        assert(str != NULL || len == 0);

        if (fnt != renderer_get_active_font()) return(false);

        const bool is_pixel_font = (fnt->oversample_x == 1 && fnt->oversample_y == 1);
        const f32  atlas_width   = (f32)fnt->atlas_width;
        const f32  atlas_height  = (f32)fnt->atlas_height;

        f32 pen_x    = pos.x;
        f32 baseline = pos.y + fnt->ascent;
        if (is_pixel_font) baseline = floorf(baseline + 0.5f);

        for (
              u32 i = 0;
                  i < len;
                ++i) {

            const cchar c = str[i];
            if (c == '\n') {
                pen_x     = pos.x;
                baseline += fnt->line_height;
                if (is_pixel_font) baseline = floorf(baseline + 0.5f);
                continue;
            }

            const font_glyph* g = fnt->glyph((u32)(u8)c);
            if (g == NULL) continue;

            // spaces and anything else without pixels only move the pen
            const f32 quad_width  = g->quad_x1 - g->quad_x0;
            const f32 quad_height = g->quad_y1 - g->quad_y0;
            if (quad_width > 0.0f && quad_height > 0.0f) {

                draw_rect rect;
                rect.x      = pen_x    + g->quad_x0;
                rect.y      = baseline + g->quad_y0;
                rect.width  = quad_width;
                rect.height = quad_height;
                if (is_pixel_font) {
                    rect.x = floorf(rect.x + 0.5f);
                    rect.y = floorf(rect.y + 0.5f);
                }

                draw_uv uv;
                uv.u_left   = (f32)g->atlas_x0 / atlas_width;
                uv.v_top    = (f32)g->atlas_y0 / atlas_height;
                uv.u_right  = (f32)g->atlas_x1 / atlas_width;
                uv.v_bottom = (f32)g->atlas_y1 / atlas_height;

                if (!draw_push_screen_quad(rect, color, uv, QUAD_SAMPLE_FONT)) return(false);
            }

            pen_x += g->advance;
            if (i + 1 < len) {
                pen_x += fnt->kerning((u32)(u8)c, (u32)(u8)str[i + 1]);
            }
        }

        return(true);
    }

    //--------------------------------------------------------------------
    // WORLD
    //--------------------------------------------------------------------

    IFB_ENGINE_API bool
    eng_draw_rect_world(
        const mat4&           xform,
        const color_rgba_f32& color) {

        const bool did_push = draw_push_world_quad(xform, color, DRAW_UV_NONE, QUAD_SAMPLE_SOLID);
        return(did_push);
    }

    IFB_ENGINE_API bool
    eng_draw_sprite_world(
        const mat4&            xform,
        const hnd_sprite_atlas h_atlas,
        const u32              i_sprite,
        const color_rgba_f32&  tint) {

        draw_uv uv;
        if (!draw_get_sprite_uv(h_atlas, i_sprite, NULL, uv)) return(false);

        const bool did_push = draw_push_world_quad(xform, tint, uv, QUAD_SAMPLE_SPRITE);
        return(did_push);
    }
};
