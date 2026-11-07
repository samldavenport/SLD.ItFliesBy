#pragma once

#include "ifb.hpp"
#include "eng-renderer.hpp"
#include "eng-asset.hpp"
#include "eng-memory.hpp"
#include "sld-opengl.hpp"
#include <cassert>
#include <cmath>
#include <cstring>

namespace ifb {

    static constexpr u32   PARALLAX_LAYER_COUNT          = 3;
    static constexpr u32   PARALLAX_VERTEX_COUNT         = 6;
    static constexpr u32   PARALLAX_TEXTURE_SLOT         = 1; // the sprite atlas is on slot 0
    static const     char* PARALLAX_UNIFORM_NAME_LAYERS  = "u_layers";
    static const     char* PARALLAX_UNIFORM_NAME_OFFSET  = "u_uv_offset";
    static const     char* PARALLAX_UNIFORM_NAME_SCALE   = "u_uv_scale";

    // how far each layer scrolls in uv per world unit the camera moves, back to front
    static constexpr f32   PARALLAX_DEFAULT_FACTORS[PARALLAX_LAYER_COUNT] = { 0.05f, 0.15f, 0.35f };

    struct renderer_parallax_shader {
        struct {
            gl_program program;
            gl_vertex  vertex;        // the quad comes from gl_VertexID, but the context still needs these bound
            gl_buffer  buf_vertex;
            gl_texture texture;       // GL_TEXTURE_2D_ARRAY, layer 0 is the back
            gl_uniform u_layers;
            gl_uniform u_uv_offset;
            gl_uniform u_uv_scale;
        } gl;
        f32  factors[PARALLAX_LAYER_COUNT];
        u32  texture_width;
        u32  texture_height;
        bool is_set;
    };

    IFB_INTERNAL renderer_parallax_shader*
    renderer_parallax_shader_create(
        void) {

        auto shdr = (renderer_parallax_shader*)global_alloc(sizeof(renderer_parallax_shader));
        assert(shdr);
        return(shdr);
    }

    IFB_INTERNAL void
    renderer_parallax_shader_init(
        renderer_parallax_shader*     shdr,
        gl_context*                   gl,
        const renderer_shader_source& src_vertex,
        const renderer_shader_source& src_fragment) {

        assert(shdr);
        assert(gl);

        bool gl_ok = true;

        // create gl objects
        shdr->gl.program          = gl_shader_program_create        (gl);
        shdr->gl.vertex           = gl_vertex_create                (gl);
        shdr->gl.buf_vertex       = gl_buffer_create                (gl);
        shdr->gl.texture          = gl_texture_create               (gl);
        const gl_shader shdr_vert = gl_shader_stage_create_vertex   (gl);
        const gl_shader shdr_frag = gl_shader_stage_create_fragment (gl);
        gl_ok = (
            shdr->gl.program    != GL_ID_INVALID &&
            shdr->gl.vertex     != GL_ID_INVALID &&
            shdr->gl.buf_vertex != GL_ID_INVALID &&
            shdr->gl.texture    != GL_ID_INVALID &&
            shdr_vert           != GL_ID_INVALID &&
            shdr_frag           != GL_ID_INVALID
        );
        assert(gl_ok);

        // compile and link shader
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_vert, src_vertex.data,   src_vertex.size);
        gl_ok &= gl_shader_stage_compile_from_source (gl, shdr_frag, src_fragment.data, src_fragment.size);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program, shdr_vert);
        gl_ok &= gl_shader_program_attach_stage      (gl, shdr->gl.program, shdr_frag);
        gl_ok &= gl_shader_program_link              (gl, shdr->gl.program);
        gl_shader_stage_destroy                      (gl, shdr_vert);
        gl_shader_stage_destroy                      (gl, shdr_frag);
        assert(gl_ok);

        // get the uniform locations
        shdr->gl.u_layers    = gl_uniform_get_location(gl, shdr->gl.program, PARALLAX_UNIFORM_NAME_LAYERS);
        shdr->gl.u_uv_offset = gl_uniform_get_location(gl, shdr->gl.program, PARALLAX_UNIFORM_NAME_OFFSET);
        shdr->gl.u_uv_scale  = gl_uniform_get_location(gl, shdr->gl.program, PARALLAX_UNIFORM_NAME_SCALE);
        assert(
            shdr->gl.u_layers    != GL_UNIFORM_INVALID &&
            shdr->gl.u_uv_offset != GL_UNIFORM_INVALID &&
            shdr->gl.u_uv_scale  != GL_UNIFORM_INVALID
        );

        // nothing is drawn until the layers are set
        for (
            u32 i_layer = 0;
                i_layer < PARALLAX_LAYER_COUNT;
              ++i_layer) {

            shdr->factors[i_layer] = PARALLAX_DEFAULT_FACTORS[i_layer];
        }
        shdr->texture_width  = 0;
        shdr->texture_height = 0;
        shdr->is_set         = false;
    }

    // the names are image assets, back to front, and every image has to be the same size
    IFB_INTERNAL bool
    renderer_parallax_shader_set_layers(
        renderer_parallax_shader* shdr,
        gl_context*               gl,
        const cchar*              names[PARALLAX_LAYER_COUNT]) {

        assert(shdr);
        assert(gl);
        assert(names);

        const asset_database* db = asset_mngr_get_database();
        if (db == NULL || db->asset_count == 0) return(false);

        const hnd_arena h_arena = arena_large_alloc();
        if (h_arena == INVALID_HANDLE) return(false);

        // load every layer, they all have to match the first one
        const asset_image* images[PARALLAX_LAYER_COUNT];
        bool is_valid = true;
        for (
            u32 i_layer = 0;
                i_layer < PARALLAX_LAYER_COUNT && is_valid;
              ++i_layer) {

            assert(names[i_layer]);

            const hnd_asset h_asset = asset_name_cstr_to_handle(names[i_layer]);
            images[i_layer] = (h_asset != INVALID_HANDLE)
                ? asset_database_load_image_by_handle_to_arena(db, h_asset, h_arena)
                : NULL;

            is_valid &= (
                images[i_layer]         != NULL &&
                images[i_layer]->width  != 0    &&
                images[i_layer]->height != 0    &&
                images[i_layer]->width  == images[0]->width &&
                images[i_layer]->height == images[0]->height
            );
        }
        if (!is_valid) {
            arena_free(h_arena);
            return(false);
        }

        // pack the layers back to back
        const u32 width      = images[0]->width;
        const u32 height     = images[0]->height;
        const u64 layer_size = (u64)width * (u64)height * sizeof(color_rgba_u32);
        byte*     packed     = (byte*)arena_push(h_arena, (u32)(layer_size * PARALLAX_LAYER_COUNT));
        if (packed == NULL) {
            arena_free(h_arena);
            return(false);
        }
        for (
            u32 i_layer = 0;
                i_layer < PARALLAX_LAYER_COUNT;
              ++i_layer) {

            memcpy(packed + (layer_size * i_layer), images[i_layer]->pixels, layer_size);
        }

        // upload
        bool did_upload = gl_texture_set_data_rgba8_layered(
            gl,
            shdr->gl.texture,
            width,
            height,
            packed,
            PARALLAX_LAYER_COUNT
        );
        arena_free(h_arena);

        // the layers repeat horizontally so they can scroll forever
        if (did_upload) {
            glBindTexture   (GL_TEXTURE_2D_ARRAY, shdr->gl.texture);
            glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glBindTexture   (GL_TEXTURE_2D_ARRAY, GL_ID_INVALID);
            did_upload = (glGetError() == GL_NO_ERROR);
        }

        shdr->texture_width  = did_upload ? width  : 0;
        shdr->texture_height = did_upload ? height : 0;
        shdr->is_set         = did_upload;
        return(did_upload);
    }

    // draws behind everything, so it goes before the map
    IFB_INTERNAL void
    renderer_parallax_shader_draw(
        renderer_parallax_shader* shdr,
        gl_context*               gl,
        const vec3&               camera_origin,
        const f32                 viewport_width,
        const f32                 viewport_height) {

        assert(shdr);
        assert(gl);

        if (!shdr->is_set)                                  return;
        if (viewport_width <= 0.0f || viewport_height <= 0.0f) return;

        // each layer scrolls by its own factor, kept in [0, 1) so the uvs don't lose precision
        f32 offsets[PARALLAX_LAYER_COUNT * 4];
        for (
            u32 i_layer = 0;
                i_layer < PARALLAX_LAYER_COUNT;
              ++i_layer) {

            const f32 u = camera_origin.x * shdr->factors[i_layer];
            offsets[(i_layer * 4) + 0] = u - floorf(u);
            offsets[(i_layer * 4) + 1] = 0.0f;
            offsets[(i_layer * 4) + 2] = 0.0f;
            offsets[(i_layer * 4) + 3] = 0.0f;
        }

        // fill the screen vertically and keep the image's aspect ratio
        const f32 aspect_viewport = viewport_width / viewport_height;
        const f32 aspect_texture  = (f32)shdr->texture_width / (f32)shdr->texture_height;
        const f32 scale[2]        = { aspect_viewport / aspect_texture, 1.0f };

        gl_context_set_shader_program (gl, shdr->gl.program);
        gl_context_set_vertex_object  (gl, shdr->gl.vertex);
        gl_context_set_buffer_vertex  (gl, shdr->gl.buf_vertex);

        glActiveTexture (GL_TEXTURE0 + PARALLAX_TEXTURE_SLOT);
        glBindTexture   (GL_TEXTURE_2D_ARRAY, shdr->gl.texture);

        gl_uniform_set_s32x1     (gl, shdr->gl.u_layers,    (s32)PARALLAX_TEXTURE_SLOT);
        gl_uniform_set_f32_array (gl, shdr->gl.u_uv_offset, offsets, PARALLAX_LAYER_COUNT);
        gl_uniform_set_f32x2     (gl, shdr->gl.u_uv_scale,  scale);

        // the background stays out of the depth buffer so the map's depth isn't touched
        glDepthMask              (GL_FALSE);
        gl_context_draw_vertices (gl, PARALLAX_VERTEX_COUNT);
        glDepthMask              (GL_TRUE);

        // put the texture state back for the sprite atlas
        glBindTexture   (GL_TEXTURE_2D_ARRAY, GL_ID_INVALID);
        glActiveTexture (GL_TEXTURE0);
    }
};
