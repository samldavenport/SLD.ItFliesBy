#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "asset.hpp"
#include "asset-database.cpp"
#include "eng-asset.cpp"
#include "renderer.hpp"
#include "memory.hpp"
#include "sprites.hpp"
#include "sprite-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the list being built, everything it has lives in its arena
    // the builder has its own sprite memory, it's made the first time the list is packed
    // the builder is only packed for the list as it was, so changing the list means packing again
    struct eng_sprite_atlas_builder {
        hnd_arena             h_arena;
        sprite_list*          list;
        sprite_atlas_builder* builder;
        bool                  is_packed;
    } static _eng_sprite_atlas_builder;

    // the atlas open in the inspector stays loaded until it's closed or another one is opened
    struct eng_sprite_atlas_inspector {
        sprite_atlas* atlas;
    } static _eng_sprite_atlas_inspector;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE sprite_list* eng_sprite_atlas_builder_get_list  (void);
    IFB_INLINE const image* eng_sprite_atlas_builder_get_image (void);

    //--------------------------------------------------------------------
    // API METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_ENGINE_API void
    eng_sprite_atlas_builder_clear(
        void) {

        auto& state = _eng_sprite_atlas_builder;
        if (!state.list) return;

        // the old list goes away with everything in the arena
        arena_reset(state.h_arena);
        state.list      = sprite_list_create(state.h_arena);
        state.is_packed = false;
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_builder_add(
        const cchar* name) {

        assert(name);

        auto& state = _eng_sprite_atlas_builder;

        sprite_list* list = eng_sprite_atlas_builder_get_list();
        if (!list || name[0] == 0) return(false);

        // only images can be sprites, the list would load anything by that name
        const asset_database* db = asset_mngr_get_database();
        if (!db) return(false);

        const u32 slot = asset_database_slot_of(db, asset_name_cstr_to_handle(name));
        if (slot == INVALID_INDEX || db->array_indexes[slot].type.val != asset_type_e_image) return(false);

        const bool did_add = sprite_list_add(list, name, state.h_arena);
        if (did_add) state.is_packed = false;
        return(did_add);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_builder_remove(
        const cchar* name) {

        assert(name);

        auto& state = _eng_sprite_atlas_builder;

        sprite_list* list = eng_sprite_atlas_builder_get_list();
        if (!list || name[0] == 0) return(false);

        const bool did_remove = sprite_list_remove(list, name);
        if (did_remove) state.is_packed = false;
        return(did_remove);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_builder_contains(
        const cchar* name) {

        assert(name);

        const sprite_list* list = eng_sprite_atlas_builder_get_list();
        if (!list || name[0] == 0) return(false);

        const hnd_asset h_asset = asset_name_cstr_to_handle(name);
        if (h_asset == INVALID_HANDLE) return(false);

        for (
            const sprite_list_node* node = sprite_list_get_first(list);
            node != NULL;
            node = sprite_list_get_next(node)) {

            if (sprite_list_get_img_asset_hnd(node) == h_asset) return(true);
        }

        return(false);
    }

    IFB_ENGINE_API u32
    eng_sprite_atlas_builder_get_count(
        void) {

        const sprite_list* list = eng_sprite_atlas_builder_get_list();
        if (!list) return(0);

        u32 count = 0;
        for (
            const sprite_list_node* node = sprite_list_get_first(list);
            node != NULL;
            node = sprite_list_get_next(node)) {

            ++count;
        }

        return(count);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_builder_get_info(
        const u32                  index,
        sprite_atlas_builder_info& info) {

        const sprite_list* list = eng_sprite_atlas_builder_get_list();
        if (!list) return(false);

        u32 node_index = 0;
        for (
            const sprite_list_node* node = sprite_list_get_first(list);
            node != NULL;
            node = sprite_list_get_next(node)) {

            if (node_index == index) {

                const hnd_asset      h_asset = sprite_list_get_img_asset_hnd (node);
                const dimensions_2d& dims    = sprite_list_get_img_dims      (node);

                // the list only has handles, the name comes from the database
                const asset_database* db   = asset_mngr_get_database();
                const u32             slot = db ? asset_database_slot_of(db, h_asset) : INVALID_INDEX;
                if (slot == INVALID_INDEX) return(false);

                memcpy(info.name, db->array_names[slot].cstr, IFB_CONFIG_ASSET_NAME_SIZE);
                info.name[IFB_CONFIG_ASSET_NAME_SIZE] = 0;

                info.hnd    = h_asset;
                info.width  = (u32)dims.width;
                info.height = (u32)dims.height;
                return(true);
            }

            ++node_index;
        }

        return(false);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_builder_pack(
        asset_preview& preview) {

        eng_asset_preview_clear(preview);

        // always packed again, so the preview is never stale
        _eng_sprite_atlas_builder.is_packed = false;

        const image* atlas = eng_sprite_atlas_builder_get_image();
        if (!atlas) return(false);

        preview.type   = asset_type_e_image;
        preview.texture = renderer_set_asset_preview_texture(eng_asset_preview_e_runtime, atlas->pixel_width, atlas->pixel_height, atlas->data.bytes);
        preview.width   = atlas->pixel_width;
        preview.height  = atlas->pixel_height;
        return(preview.texture != 0);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_builder_save(
        const cchar* path) {

        assert(path);

        auto& state = _eng_sprite_atlas_builder;

        if (path[0] == 0) return(false);
        if (eng_sprite_atlas_builder_get_count() == 0) return(false);

        // the list may have changed since it was packed
        if (!eng_sprite_atlas_builder_get_image()) return(false);

        // the atlas is only needed long enough to write it, its block goes back after
        sprite_atlas* atlas = sprite_atlas_create();
        if (!atlas) return(false);

        const bool did_save = (
            sprite_atlas_build        (atlas, state.builder) &&
            sprite_atlas_save_to_file (atlas, path)
        );

        sprite_atlas_destroy(atlas);
        return(did_save);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_inspector_open(
        const cchar*   path,
        asset_preview& preview) {

        assert(path);

        auto& state = _eng_sprite_atlas_inspector;

        if (path[0] == 0) return(false);

        // the new atlas is loaded next to the open one, so a failed open leaves the open one as it was
        sprite_atlas* atlas = sprite_atlas_create();
        if (!atlas) return(false);

        if (!sprite_atlas_load_from_file(atlas, path)) {
            sprite_atlas_destroy(atlas);
            return(false);
        }

        // the texture has its own copy of the pixels
        const image* img     = sprite_atlas_get_image(atlas);
        const u32    texture = renderer_set_asset_preview_texture(eng_asset_preview_e_atlas, img->pixel_width, img->pixel_height, img->data.bytes);
        if (texture == 0) {
            sprite_atlas_destroy(atlas);
            return(false);
        }

        eng_sprite_atlas_inspector_close();
        state.atlas = atlas;

        eng_asset_preview_clear(preview);
        preview.type    = asset_type_e_image;
        preview.texture = texture;
        preview.width   = img->pixel_width;
        preview.height  = img->pixel_height;
        return(true);
    }

    IFB_ENGINE_API void
    eng_sprite_atlas_inspector_close(
        void) {

        auto& state = _eng_sprite_atlas_inspector;
        if (!state.atlas) return;

        sprite_atlas_destroy(state.atlas);
        state.atlas = NULL;
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_inspector_get_info(
        sprite_atlas_info& info) {

        const sprite_atlas* atlas = _eng_sprite_atlas_inspector.atlas;
        if (!atlas) return(false);

        const image* img = sprite_atlas_get_image(atlas);

        info.width        = img->pixel_width;
        info.height       = img->pixel_height;
        info.sprite_count = sprite_atlas_get_sprite_count(atlas);
        return(true);
    }

    IFB_ENGINE_API bool
    eng_sprite_atlas_inspector_get_sprite(
        const u32                 index,
        sprite_atlas_sprite_info& info) {

        const sprite_atlas* atlas = _eng_sprite_atlas_inspector.atlas;
        if (!atlas) return(false);

        const bool did_get = sprite_atlas_get_sprite_info(atlas, index, info);
        return(did_get);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // packs the list if it changed since the last time, the image stays on the builder's memory
    IFB_INLINE const image*
    eng_sprite_atlas_builder_get_image(
        void) {

        auto& state = _eng_sprite_atlas_builder;

        sprite_list* list = eng_sprite_atlas_builder_get_list();
        if (!list) return(NULL);

        if (!state.builder) {
            state.builder = sprite_atlas_builder_create();
            if (!state.builder) return(NULL);
        }

        // the last atlas goes away with everything else on the builder's memory
        if (!state.is_packed) {
            sprite_atlas_builder_reset    (state.builder);
            sprite_atlas_builder_set_list (state.builder, list);
        }

        const image* atlas = sprite_atlas_builder_get_image(state.builder);
        state.is_packed = (atlas != NULL);
        return(atlas);
    }

    // the list is made the first time it's needed
    IFB_INLINE sprite_list*
    eng_sprite_atlas_builder_get_list(
        void) {

        auto& state = _eng_sprite_atlas_builder;
        if (state.list) return(state.list);

        state.h_arena = arena_alloc();
        if (state.h_arena == INVALID_HANDLE) return(NULL);

        state.list = sprite_list_create(state.h_arena);
        return(state.list);
    }
};
