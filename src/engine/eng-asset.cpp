#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "asset.hpp"
#include "asset-dictionary.cpp"
#include "files.hpp"
#include "memory.hpp"
#include "renderer.hpp"
#include "sld-strings.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    static constexpr u32 ENG_ASSET_PATH_SIZE = IFB_CONFIG_FILE_PATH_SIZE;

    // the dictionary being edited, everything it has lives in its arena
    // file paths are relative to the directory of the dictionary, which is empty until it is loaded or saved
    struct eng_asset_dictionary {
        hnd_arena         h_arena;
        asset_dictionary* dict;
        cchar             dir[ENG_ASSET_PATH_SIZE];
    } static _eng_asset_dictionary;

    // the text of the last file that was previewed has its own arena
    struct eng_asset_preview {
        bool      is_init;
        hnd_arena h_arena;
    } static _eng_asset_preview;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE asset_dictionary*                 eng_asset_dictionary_get       (void);
    IFB_INLINE const asset_dictionary_file_list* eng_asset_dictionary_get_list  (const u32 type);
    IFB_INLINE bool                              eng_asset_dictionary_has_id    (const asset_dictionary_file_list& list, const u32 id);
    IFB_INLINE const cchar*                      eng_asset_dictionary_push_cstr (const cchar* cstr, const u32 length);
    IFB_INLINE bool                              eng_asset_dictionary_rebase    (const cchar* dir);
    IFB_INLINE bool                              eng_asset_preview_load_text    (const cchar* path, asset_dictionary_preview& preview);
    IFB_INLINE bool                              eng_asset_preview_load_image   (const cchar* path, asset_dictionary_preview& preview);
    IFB_INLINE bool                              eng_asset_path_is_separator    (const cchar c);
    IFB_INLINE bool                              eng_asset_path_is_absolute     (const cchar* path);
    IFB_INLINE bool                              eng_asset_path_equals          (const cchar* path_a, const cchar* path_b);
    IFB_INLINE bool                              eng_asset_path_normalize       (const cchar* path, cchar* path_out);
    IFB_INLINE bool                              eng_asset_path_get_dir         (const cchar* file_path, cchar* dir_out);
    IFB_INLINE bool                              eng_asset_path_to_absolute     (const cchar* dir, const cchar* path, cchar* path_out);
    IFB_INLINE bool                              eng_asset_path_to_relative     (const cchar* dir, const cchar* path, cchar* path_out);

    //--------------------------------------------------------------------
    // API METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_ENGINE_API bool
    eng_asset_dictionary_new(
        void) {

        auto& state = _eng_asset_dictionary;

        // the old dictionary goes away with everything in the arena
        if (state.dict) {
            arena_reset(state.h_arena);
        }
        else {
            state.h_arena = arena_alloc();
        }

        state.dict   = asset_dictionary_create(state.h_arena);
        state.dir[0] = 0;
        return(state.dict != NULL);
    }

    IFB_ENGINE_API bool
    eng_asset_dictionary_load(
        const cchar* path) {

        assert(path);

        auto& state = _eng_asset_dictionary;

        // load into a new arena, so a failed load leaves the current dictionary alone
        const hnd_arena   h_arena = arena_alloc();
        asset_dictionary* dict    = asset_dictionary_load_from_json(path, h_arena);
        if (!dict) {
            arena_free(h_arena);
            return(false);
        }

        if (state.dict) {
            arena_free(state.h_arena);
        }
        state.h_arena = h_arena;
        state.dict    = dict;

        // the paths in the file are relative to it
        if (!eng_asset_path_get_dir(path, state.dir)) {
            state.dir[0] = 0;
        }
        return(true);
    }

    IFB_ENGINE_API bool
    eng_asset_dictionary_save(
        const cchar* path) {

        assert(path);

        auto& state = _eng_asset_dictionary;

        asset_dictionary* dict = eng_asset_dictionary_get();
        if (!dict) return(false);

        // the file paths are written relative to where the dictionary is going
        cchar dir[ENG_ASSET_PATH_SIZE];
        if (eng_asset_path_get_dir(path, dir)) {
            if (!eng_asset_dictionary_rebase(dir)) return(false);
        }

        const bool did_save = asset_dictionary_save_to_json(dict, path, state.h_arena);
        return(did_save);
    }

    IFB_ENGINE_API hnd_asset
    eng_asset_dictionary_add_file(
        const u32    type,
        const cchar* name,
        const cchar* path) {

        assert(name);
        assert(path);

        auto& state = _eng_asset_dictionary;

        asset_dictionary* dict = eng_asset_dictionary_get();
        if (!dict || type >= asset_type_e_count || path[0] == 0) return(INVALID_HANDLE);

        // an absolute path is made relative to the dictionary, if it has a location yet
        // it stays absolute if it can't be, like a file on another drive
        cchar path_abs[ENG_ASSET_PATH_SIZE];
        cchar path_rel[ENG_ASSET_PATH_SIZE];
        const bool is_relative = (
            state.dir[0] != 0                                      &&
            eng_asset_path_is_absolute (path)                      &&
            eng_asset_path_normalize   (path, path_abs)            &&
            eng_asset_path_to_relative (state.dir, path_abs, path_rel)
        );
        if (is_relative) {
            path = path_rel;
        }

        // names are always upper case, and both strings need to fit with a terminator
        cchar      name_upper[IFB_CONFIG_FILE_PATH_SIZE];
        const u32  length_name = asset_dictionary_name_to_upper(name, name_upper);
        const u32  length_path = cstr_nvar_length(path, IFB_CONFIG_FILE_PATH_SIZE);
        const bool is_valid    = (
            length_name != 0 &&
            length_path != 0 &&
            length_path <  IFB_CONFIG_FILE_PATH_SIZE
        );
        if (!is_valid) return(INVALID_HANDLE);

        // the id is the hash of the upper case name, so it has to be unique across every list
        const u32  id        = hash_u32((void*)name_upper, length_name);
        const bool is_in_use = (
            eng_asset_dictionary_has_id(dict->text_files,  id) ||
            eng_asset_dictionary_has_id(dict->image_files, id)
        );
        if (is_in_use) return(INVALID_HANDLE);

        // the dictionary only copies the name, so the path goes in the arena with the node
        const u32    save      = arena_save(state.h_arena);
        const cchar* path_copy = eng_asset_dictionary_push_cstr(path, length_path);

        hnd_asset hnd = INVALID_HANDLE;
        if (path_copy) {
            hnd = (type == asset_type_e_text)
                ? asset_dictionary_add_text_file  (dict, name_upper, path_copy, state.h_arena)
                : asset_dictionary_add_image_file (dict, name_upper, path_copy, state.h_arena);
        }

        if (hnd == INVALID_HANDLE) {
            arena_revert(state.h_arena, save);
        }
        return(hnd);
    }

    IFB_ENGINE_API bool
    eng_asset_dictionary_remove_file(
        const cchar* name) {

        assert(name);

        asset_dictionary* dict = eng_asset_dictionary_get();
        if (!dict || name[0] == 0) return(false);

        const bool did_remove = asset_dictionary_remove_file(dict, name);
        return(did_remove);
    }

    IFB_ENGINE_API u32
    eng_asset_dictionary_get_file_count(
        const u32 type) {

        const asset_dictionary_file_list* list = eng_asset_dictionary_get_list(type);
        if (!list) return(0);

        u32 count = 0;
        for (
            const asset_dictionary_file_node* node = list->first;
            node != NULL;
            node = node->next) {

            ++count;
        }

        return(count);
    }

    IFB_ENGINE_API bool
    eng_asset_dictionary_get_file_info(
        const u32                   type,
        const u32                   index,
        asset_dictionary_file_info& info) {

        const asset_dictionary_file_list* list = eng_asset_dictionary_get_list(type);
        if (!list) return(false);

        u32 node_index = 0;
        for (
            const asset_dictionary_file_node* node = list->first;
            node != NULL;
            node = node->next) {

            if (node_index == index) {
                info.name = node->name;
                info.path = node->path;
                info.id   = node->id;
                info.type = type;
                return(true);
            }

            ++node_index;
        }

        return(false);
    }

    IFB_ENGINE_API bool
    eng_asset_dictionary_load_preview(
        const u32                 id,
        asset_dictionary_preview& preview) {

        auto& state = _eng_asset_dictionary;

        preview.type              = asset_type_e_count;
        preview.text              = NULL;
        preview.is_text_truncated = false;
        preview.texture           = 0;
        preview.width             = 0;
        preview.height            = 0;

        // find the file
        const asset_dictionary_file_node* node_found = NULL;
        for (
            u32 type = 0;
                type < asset_type_e_count && node_found == NULL;
              ++type) {

            const asset_dictionary_file_list* list = eng_asset_dictionary_get_list(type);
            if (!list) continue;

            for (
                const asset_dictionary_file_node* node = list->first;
                node != NULL;
                node = node->next) {

                if (node->id == id) {
                    node_found   = node;
                    preview.type = type;
                    break;
                }
            }
        }
        if (!node_found) return(false);

        // a relative path can't be found until the dictionary has a location
        cchar path[ENG_ASSET_PATH_SIZE];
        if (!eng_asset_path_to_absolute(state.dir, node_found->path, path)) return(false);

        const bool did_load = (preview.type == asset_type_e_text)
            ? eng_asset_preview_load_text  (path, preview)
            : eng_asset_preview_load_image (path, preview);

        return(did_load);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the text is copied out of the file buffer, which only holds the start of a big file
    IFB_INLINE bool
    eng_asset_preview_load_text(
        const cchar*              path,
        asset_dictionary_preview& preview) {

        auto& state = _eng_asset_preview;

        if (!state.is_init) {
            state.is_init = true;
            state.h_arena = arena_alloc();
        }
        arena_reset(state.h_arena);

        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == FILE_HANDLE_INVALID) return(false);

        const u32 size       = file_get_size(h_file);
        const u32 length_max = IFB_CONFIG_FILE_GRANULARITY - 1;
        const u32 length     = (size < length_max) ? size : length_max;

        auto text = (cchar*)arena_push(state.h_arena, length + 1);
        if (text && length != 0) {

            const cchar* data = file_read(h_file, size);
            for (
                u32 index = 0;
                    index < length;
                  ++index) {

                text[index] = data[index];
            }
        }
        file_close(h_file);
        if (!text) return(false);

        text[length] = 0;

        preview.text              = text;
        preview.is_text_truncated = (size > length);
        return(true);
    }

    // the image is decoded straight from the file, and the pixels go to a texture
    IFB_INLINE bool
    eng_asset_preview_load_image(
        const cchar*              path,
        asset_dictionary_preview& preview) {

        // always four channels, whatever the file has
        s32 width    = 0;
        s32 height   = 0;
        s32 channels = 0;
        stbi_uc* pixels = stbi_load(path, &width, &height, &channels, 4);
        if (!pixels) return(false);

        const u32 texture = (width > 0 && height > 0)
            ? renderer_set_asset_preview_texture(width, height, (const byte*)pixels)
            : 0;
        stbi_image_free(pixels);
        if (texture == 0) return(false);

        preview.texture = texture;
        preview.width   = width;
        preview.height  = height;
        return(true);
    }

    // there is always a dictionary to edit, it starts out empty
    IFB_INLINE asset_dictionary*
    eng_asset_dictionary_get(
        void) {

        auto& state = _eng_asset_dictionary;

        if (!state.dict) {
            eng_asset_dictionary_new();
        }

        return(state.dict);
    }

    IFB_INLINE const asset_dictionary_file_list*
    eng_asset_dictionary_get_list(
        const u32 type) {

        const asset_dictionary* dict = eng_asset_dictionary_get();
        if (!dict) return(NULL);

        switch (type) {
            case asset_type_e_text:  return(&dict->text_files);
            case asset_type_e_image: return(&dict->image_files);
            default:                 return(NULL);
        }
    }

    IFB_INLINE bool
    eng_asset_dictionary_has_id(
        const asset_dictionary_file_list& list,
        const u32                         id) {

        for (
            const asset_dictionary_file_node* node = list.first;
            node != NULL;
            node = node->next) {

            if (node->id == id) return(true);
        }

        return(false);
    }

    IFB_INLINE const cchar*
    eng_asset_dictionary_push_cstr(
        const cchar* cstr,
        const u32    length) {

        auto& state = _eng_asset_dictionary;

        auto copy = (cchar*)arena_push(state.h_arena, length + 1);
        if (!copy) return(NULL);

        for (
            u32 index = 0;
            index < length;
            ++index) {

            copy[index] = cstr[index];
        }
        copy[length] = 0;

        return(copy);
    }

    // every file path is made relative to a new dictionary directory
    IFB_INLINE bool
    eng_asset_dictionary_rebase(
        const cchar* dir) {

        auto& state = _eng_asset_dictionary;

        asset_dictionary_file_list* lists[asset_type_e_count] = {
            &state.dict->text_files,
            &state.dict->image_files
        };

        for (
            u32 type = 0;
                type < asset_type_e_count;
              ++type) {

            for (
                asset_dictionary_file_node* node = lists[type]->first;
                node != NULL;
                node = node->next) {

                // a relative path with no directory is already relative to wherever the dictionary goes
                cchar path_abs[ENG_ASSET_PATH_SIZE];
                cchar path_rel[ENG_ASSET_PATH_SIZE];
                if (!eng_asset_path_to_absolute(state.dir, node->path, path_abs)) continue;

                // the path stays absolute if it can't be relative, like a file on another drive
                const cchar* path = eng_asset_path_to_relative(dir, path_abs, path_rel)
                    ? path_rel
                    : path_abs;
                if (eng_asset_path_equals(path, node->path)) continue;

                const u32    length    = cstr_nvar_length(path, ENG_ASSET_PATH_SIZE);
                const cchar* path_copy = eng_asset_dictionary_push_cstr(path, length);
                if (!path_copy) return(false);

                node->path = path_copy;
            }
        }

        // the paths are relative to the new directory now, even if the save fails
        for (
            u32 index = 0;
                index < ENG_ASSET_PATH_SIZE;
              ++index) {

            state.dir[index] = dir[index];
            if (dir[index] == 0) break;
        }

        return(true);
    }

    IFB_INLINE bool
    eng_asset_path_is_separator(
        const cchar c) {

        return(c == '/' || c == '\\');
    }

    IFB_INLINE bool
    eng_asset_path_is_absolute(
        const cchar* path) {

        const bool is_absolute = (
            eng_asset_path_is_separator(path[0]) ||
            (path[0] != 0 && path[1] == ':')
        );
        return(is_absolute);
    }

    IFB_INLINE bool
    eng_asset_path_equals(
        const cchar* path_a,
        const cchar* path_b) {

        u32 index = 0;
        while (path_a[index] != 0 && path_a[index] == path_b[index]) {
            ++index;
        }

        return(path_a[index] == path_b[index]);
    }

    // forward slashes, with no empty, current (.) or parent (..) directories
    // false if the result is empty or doesn't fit
    IFB_INLINE bool
    eng_asset_path_normalize(
        const cchar* path,
        cchar*       path_out) {

        u32 length = 0;
        u32 index  = 0;

        // leading separators are kept, for rooted and network paths
        while (eng_asset_path_is_separator(path[index])) {
            if (length >= ENG_ASSET_PATH_SIZE - 1) return(false);
            path_out[length] = '/';
            ++length;
            ++index;
        }

        // the first directory is the root, a parent directory can't go above it
        u32 length_root = 0;

        while (path[index] != 0) {

            const u32 start = index;
            while (path[index] != 0 && !eng_asset_path_is_separator(path[index])) {
                ++index;
            }
            const u32 length_dir = (index - start);
            while (eng_asset_path_is_separator(path[index])) {
                ++index;
            }

            const bool is_current = (length_dir == 1 && path[start] == '.');
            const bool is_parent  = (length_dir == 2 && path[start] == '.' && path[start + 1] == '.');
            if (is_current) continue;

            if (is_parent) {
                while (length > length_root && path_out[length - 1] != '/') {
                    --length;
                }
                if (length > length_root) {
                    --length;
                }
                continue;
            }

            const bool needs_separator = (length != 0 && path_out[length - 1] != '/');
            const u32  length_new      = length + length_dir + (needs_separator ? 1 : 0);
            if (length_new >= ENG_ASSET_PATH_SIZE) return(false);

            if (needs_separator) {
                path_out[length] = '/';
                ++length;
            }
            for (
                u32 offset = 0;
                    offset < length_dir;
                  ++offset) {

                path_out[length] = path[start + offset];
                ++length;
            }

            if (length_root == 0) {
                length_root = length;
            }
        }

        path_out[length] = 0;
        return(length != 0);
    }

    // the normalized directory of an absolute file path
    IFB_INLINE bool
    eng_asset_path_get_dir(
        const cchar* file_path,
        cchar*       dir_out) {

        if (!eng_asset_path_is_absolute (file_path))          return(false);
        if (!eng_asset_path_normalize   (file_path, dir_out)) return(false);

        // everything after the last separator is the file name
        u32 length = 0;
        for (
            u32 index = 0;
            dir_out[index] != 0;
            ++index) {

            if (dir_out[index] == '/') length = index;
        }
        if (length == 0) return(false);

        dir_out[length] = 0;
        return(true);
    }

    // a normalized absolute path, the directory is only used if the path is relative
    IFB_INLINE bool
    eng_asset_path_to_absolute(
        const cchar* dir,
        const cchar* path,
        cchar*       path_out) {

        if (eng_asset_path_is_absolute(path)) {
            return(eng_asset_path_normalize(path, path_out));
        }
        if (dir[0] == 0) return(false);

        // the directory, a separator, and the path
        cchar path_full[(ENG_ASSET_PATH_SIZE * 2) + 2];
        u32   length = 0;
        for (
            u32 index = 0;
            index < ENG_ASSET_PATH_SIZE && dir[index] != 0;
            ++index) {

            path_full[length] = dir[index];
            ++length;
        }
        path_full[length] = '/';
        ++length;
        for (
            u32 index = 0;
            index < ENG_ASSET_PATH_SIZE && path[index] != 0;
            ++index) {

            path_full[length] = path[index];
            ++length;
        }
        path_full[length] = 0;

        return(eng_asset_path_normalize(path_full, path_out));
    }

    // the directory and the path are both normalized and absolute
    // false if they have nothing in common, like different drives, or the result doesn't fit
    IFB_INLINE bool
    eng_asset_path_to_relative(
        const cchar* dir,
        const cchar* path,
        cchar*       path_out) {

        // the directories they share, windows paths are not case sensitive
        u32 length_common = 0;
        for (
            u32 index = 0;
            ;
            ++index) {

            const cchar c_dir       = dir [index];
            const cchar c_path      = path[index];
            const bool  is_end_dir  = (c_dir  == 0 || c_dir  == '/');
            const bool  is_end_path = (c_path == 0 || c_path == '/');

            if (is_end_dir && is_end_path) {
                length_common = index;
                if (c_dir == 0 || c_path == 0) break;
                continue;
            }
            if (is_end_dir || is_end_path) break;

            const cchar lower_dir  = (c_dir  >= 'A' && c_dir  <= 'Z') ? (c_dir  + 32) : c_dir;
            const cchar lower_path = (c_path >= 'A' && c_path <= 'Z') ? (c_path + 32) : c_path;
            if (lower_dir != lower_path) break;
        }
        if (length_common == 0) return(false);

        // up one for every directory they don't share
        u32 length = 0;
        for (
            u32 index = length_common;
            dir[index] != 0;
            ++index) {

            if (dir[index] != '/') continue;

            if (length + 3 >= ENG_ASSET_PATH_SIZE) return(false);
            path_out[length + 0] = '.';
            path_out[length + 1] = '.';
            path_out[length + 2] = '/';
            length += 3;
        }

        // then down the rest of the path
        u32 index = length_common;
        if (path[index] == '/') {
            ++index;
        }
        for (
            ;
            path[index] != 0;
            ++index) {

            if (length + 1 >= ENG_ASSET_PATH_SIZE) return(false);
            path_out[length] = path[index];
            ++length;
        }

        // the path is the directory
        if (length == 0) {
            path_out[length] = '.';
            ++length;
        }

        path_out[length] = 0;
        return(true);
    }
};
