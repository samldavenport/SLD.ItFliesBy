#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "asset.hpp"
#include "asset-database.cpp"
#include "asset-internal.hpp"
#include "ifb-files.hpp"
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

    // the database being viewed keeps its file open, the error is for the gui
    static constexpr u32 ENG_ASSET_ERROR_SIZE = (ENG_ASSET_PATH_SIZE * 2) + 128;

    struct eng_asset_database {
        hnd_arena       h_arena;
        asset_database* db;
        cchar           error[ENG_ASSET_ERROR_SIZE];
    } static _eng_asset_database;

    // the dictionary, the database, the runtime database, and the atlas inspector each have a preview, so every window can show one
    enum eng_asset_preview_e {
        eng_asset_preview_e_dictionary = 0,
        eng_asset_preview_e_database   = 1,
        eng_asset_preview_e_runtime    = 2,
        eng_asset_preview_e_atlas      = 3,
        eng_asset_preview_e_count      = 4
    };
    static_assert(eng_asset_preview_e_count == RENDERER_ASSET_PREVIEW_COUNT);

    // the text of the last file that was previewed has its own arena
    struct eng_asset_preview {
        bool      is_init;
        hnd_arena h_arena;
    } static _eng_asset_preview[eng_asset_preview_e_count];

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE asset_dictionary*                 eng_asset_dictionary_get       (void);
    IFB_INLINE const asset_dictionary_file_list* eng_asset_dictionary_get_list  (const u32 type);
    IFB_INLINE bool                              eng_asset_dictionary_has_id    (const asset_dictionary_file_list& list, const u32 id);
    IFB_INLINE const cchar*                      eng_asset_dictionary_push_cstr (const cchar* cstr, const u32 length);
    IFB_INLINE bool                              eng_asset_dictionary_rebase    (const cchar* dir);
    IFB_INLINE bool                              eng_asset_database_check       (asset_dictionary* dict, const cchar* dir, const hnd_arena h_arena);
    IFB_INLINE bool                              eng_asset_database_check_file  (asset_dictionary_file_node* node, const u32 type, const cchar* dir, const u32 size_header, const hnd_arena h_arena);
    IFB_INLINE void                              eng_asset_database_set_error   (const cchar* format, ...);
    IFB_INLINE void                              eng_asset_preview_clear        (asset_preview& preview);
    IFB_INLINE hnd_arena                         eng_asset_preview_get_arena    (const u32 slot);
    IFB_INLINE bool                              eng_asset_preview_load_text    (const cchar* path, asset_preview& preview);
    IFB_INLINE bool                              eng_asset_preview_load_image   (const cchar* path, asset_preview& preview);
    IFB_INLINE bool                              eng_asset_preview_load_sprite_atlas (const cchar* path, asset_preview& preview);
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
            eng_asset_dictionary_has_id(dict->text_files,         id) ||
            eng_asset_dictionary_has_id(dict->image_files,        id) ||
            eng_asset_dictionary_has_id(dict->sprite_atlas_files, id)
        );
        if (is_in_use) return(INVALID_HANDLE);

        // the dictionary only copies the name, so the path goes in the arena with the node
        const u32    save      = arena_save(state.h_arena);
        const cchar* path_copy = eng_asset_dictionary_push_cstr(path, length_path);

        hnd_asset hnd = INVALID_HANDLE;
        if (path_copy) {
            switch (type) {
                case asset_type_e_text:         hnd = asset_dictionary_add_text_file         (dict, name_upper, path_copy, state.h_arena); break;
                case asset_type_e_image:        hnd = asset_dictionary_add_image_file        (dict, name_upper, path_copy, state.h_arena); break;
                case asset_type_e_sprite_atlas: hnd = asset_dictionary_add_sprite_atlas_file (dict, name_upper, path_copy, state.h_arena); break;
                default: break;
            }
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
        const u32      id,
        asset_preview& preview) {

        auto& state = _eng_asset_dictionary;

        eng_asset_preview_clear(preview);

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

        bool did_load = false;
        switch (preview.type) {
            case asset_type_e_text:         did_load = eng_asset_preview_load_text         (path, preview); break;
            case asset_type_e_image:        did_load = eng_asset_preview_load_image        (path, preview); break;
            case asset_type_e_sprite_atlas: did_load = eng_asset_preview_load_sprite_atlas (path, preview); break;
            default: break;
        }

        return(did_load);
    }

    IFB_ENGINE_API bool
    eng_asset_database_build(
        const cchar* path_dictionary,
        const cchar* path_database) {

        assert(path_dictionary);
        assert(path_database);

        auto& state = _eng_asset_database;
        state.error[0] = 0;

        // the dictionary is only needed for the build, so it gets its own arena
        const hnd_arena h_arena_dict = arena_large_alloc();
        if (h_arena_dict == INVALID_HANDLE) {
            eng_asset_database_set_error("There is no memory for the dictionary");
            return(false);
        }

        asset_dictionary* dict = asset_dictionary_load_from_json(path_dictionary, h_arena_dict);
        if (!dict) {
            eng_asset_database_set_error("Failed to load the dictionary %s", path_dictionary);
            arena_free(h_arena_dict);
            return(false);
        }

        // the paths in the dictionary are relative to it
        cchar dir[ENG_ASSET_PATH_SIZE];
        bool  is_valid = eng_asset_path_get_dir(path_dictionary, dir);
        if (!is_valid) {
            eng_asset_database_set_error("The dictionary path %s isn't absolute", path_dictionary);
        }
        else if (asset_dictionary_count_assets(dict) == 0) {
            eng_asset_database_set_error("The dictionary has no files");
            is_valid = false;
        }

        // the build asserts on a file it can't read, so every file is checked first
        is_valid = is_valid && eng_asset_database_check(dict, dir, h_arena_dict);

        // the database file can't be open while it is written
        if (is_valid) {
            eng_asset_database_close();

            state.h_arena = arena_large_alloc();
            state.db      = (state.h_arena != INVALID_HANDLE)
                ? asset_database_build(path_database, dict, state.h_arena)
                : NULL;

            if (!state.db) {
                eng_asset_database_set_error("Failed to write the database %s", path_database);
                if (state.h_arena != INVALID_HANDLE) {
                    arena_free(state.h_arena);
                }
                is_valid = false;
            }
        }

        arena_free(h_arena_dict);
        return(is_valid);
    }

    IFB_ENGINE_API bool
    eng_asset_database_load(
        const cchar* path) {

        assert(path);

        auto& state = _eng_asset_database;
        state.error[0] = 0;

        // the file may be the one that's open, which can't be opened again while it is
        eng_asset_database_close();

        state.h_arena = arena_large_alloc();
        if (state.h_arena == INVALID_HANDLE) {
            eng_asset_database_set_error("There is no memory for the database");
            return(false);
        }

        state.db = asset_database_load(path, state.h_arena);
        if (!state.db) {
            eng_asset_database_set_error("Failed to load %s, it isn't a database or it is damaged", path);
            arena_free(state.h_arena);
            return(false);
        }

        return(true);
    }

    IFB_ENGINE_API void
    eng_asset_database_close(
        void) {

        auto& state = _eng_asset_database;
        if (!state.db) return;

        file_close (state.db->file_handle);
        arena_free (state.h_arena);
        state.db = NULL;
    }

    // copies the open database over the runtime database in the working directory, then reloads the runtime database
    // the runtime database is closed while its file is written, so nothing can be pinned
    // textures already made from the old assets stay the way they were
    IFB_ENGINE_API bool
    eng_asset_database_copy_to_runtime(
        void) {

        auto& state = _eng_asset_database;
        state.error[0] = 0;

        if (!state.db) {
            eng_asset_database_set_error("There is no database open to copy");
            return(false);
        }

        if (!asset_mngr_close_database()) {
            eng_asset_database_set_error("Failed to close the runtime database, an asset is pinned");
            return(false);
        }

        const auto&    cfg    = config_instance();
        const hnd_file h_src  = state.db->file_handle;
        const hnd_file h_dst  = file_wo_overwrite(cfg.asset_database_path);
        bool           is_valid = (h_dst != FILE_HANDLE_INVALID);
        if (!is_valid) {
            eng_asset_database_set_error("Failed to open %s in the working directory", cfg.asset_database_path);
        }

        // the database can be bigger than an arena, so it's copied a piece at a time
        static constexpr u32 COPY_CHUNK_SIZE = size_kilobytes(64);
        const hnd_arena h_arena = is_valid ? arena_alloc() : INVALID_HANDLE;
        byte*           chunk   = (h_arena != INVALID_HANDLE) ? (byte*)arena_push(h_arena, COPY_CHUNK_SIZE) : NULL;
        if (is_valid && !chunk) {
            eng_asset_database_set_error("There is no memory to copy the database");
            is_valid = false;
        }

        if (is_valid) {

            const u32 size_file = file_get_size(h_src);
            for (
                u32 offset = 0;
                    offset < size_file && is_valid;
                    offset += COPY_CHUNK_SIZE) {

                const u32 size_left  = size_file - offset;
                const u32 size_chunk = (size_left < COPY_CHUNK_SIZE) ? size_left : COPY_CHUNK_SIZE;
                is_valid = asset_database_read_at(h_src, offset, size_chunk, chunk);
                if (is_valid) {
                    (void)asset_database_write_at(h_dst, offset, size_chunk, chunk);
                }
            }
            if (!is_valid) {
                eng_asset_database_set_error("Failed to read the open database");
            }
        }

        if (h_arena != INVALID_HANDLE) arena_free(h_arena);
        if (h_dst   != FILE_HANDLE_INVALID) file_close(h_dst);

        // the runtime database comes back even if the copy failed, a partial copy won't load
        if (!asset_mngr_open_database() && is_valid) {
            eng_asset_database_set_error("Copied the database, but failed to load it as the runtime database");
            is_valid = false;
        }

        return(is_valid);
    }

    IFB_ENGINE_API const cchar*
    eng_asset_database_get_error(
        void) {

        auto& state = _eng_asset_database;
        return(state.error);
    }

    IFB_ENGINE_API u32
    eng_asset_database_get_count(
        void) {

        auto& state = _eng_asset_database;

        const u32 count = state.db ? state.db->asset_count : 0;
        return(count);
    }

    IFB_ENGINE_API bool
    eng_asset_database_get_info(
        const u32            index,
        asset_database_info& info) {

        auto& state = _eng_asset_database;
        if (!state.db || index >= state.db->asset_count) return(false);

        const asset_database* db = state.db;

        memcpy(info.name, db->array_names[index].cstr, IFB_CONFIG_ASSET_NAME_SIZE);
        info.name[IFB_CONFIG_ASSET_NAME_SIZE] = 0;

        info.hnd    = db->array_handles[index];
        info.type   = db->array_indexes[index].type.val;
        info.offset = db->array_indexes[index].offset;
        info.size   = db->array_indexes[index].size;
        return(true);
    }

    IFB_ENGINE_API bool
    eng_asset_database_load_preview(
        const u32      index,
        asset_preview& preview) {

        auto& state = _eng_asset_database;

        eng_asset_preview_clear(preview);
        if (!state.db || index >= state.db->asset_count) return(false);

        const asset_database* db      = state.db;
        const hnd_asset       h_asset = db->array_handles[index];
        const hnd_arena       h_arena = eng_asset_preview_get_arena(eng_asset_preview_e_database);
        if (h_arena == INVALID_HANDLE) return(false);

        preview.type = db->array_indexes[index].type.val;

        if (preview.type == asset_type_e_text) {

            const asset_text* txt = asset_database_load_text_by_handle_to_arena(db, h_asset, h_arena);
            if (!txt) return(false);

            preview.text = txt->cstr;
            return(true);
        }

        // an atlas is previewed as its whole image
        if (preview.type == asset_type_e_sprite_atlas) {

            const asset_sprite_atlas* atlas = asset_database_load_sprite_atlas_by_handle_to_arena(db, h_asset, h_arena);
            if (!atlas) return(false);

            preview.texture = renderer_set_asset_preview_texture(eng_asset_preview_e_database, atlas->width, atlas->height, (const byte*)atlas->array_pixels);
            preview.width   = atlas->width;
            preview.height  = atlas->height;
            return(preview.texture != 0);
        }

        const asset_image* img = asset_database_load_image_by_handle_to_arena(db, h_asset, h_arena);
        if (!img || img->width == 0 || img->height == 0) return(false);

        preview.texture = renderer_set_asset_preview_texture(eng_asset_preview_e_database, img->width, img->height, (const byte*)img->pixels);
        preview.width   = img->width;
        preview.height  = img->height;
        return(preview.texture != 0);
    }

    IFB_ENGINE_API u32
    eng_asset_runtime_get_count(
        void) {

        const asset_database* db = asset_mngr_get_database();

        const u32 count = db ? db->asset_count : 0;
        return(count);
    }

    IFB_ENGINE_API bool
    eng_asset_runtime_get_info(
        const u32            index,
        asset_database_info& info) {

        const asset_database* db = asset_mngr_get_database();
        if (!db || index >= db->asset_count) return(false);

        memcpy(info.name, db->array_names[index].cstr, IFB_CONFIG_ASSET_NAME_SIZE);
        info.name[IFB_CONFIG_ASSET_NAME_SIZE] = 0;

        info.hnd    = db->array_handles[index];
        info.type   = db->array_indexes[index].type.val;
        info.offset = db->array_indexes[index].offset;
        info.size   = db->array_indexes[index].size;
        return(true);
    }

    // the image is read from the file, so the asset manager's memory isn't touched
    IFB_ENGINE_API bool
    eng_asset_runtime_load_image_preview(
        const cchar*   name,
        asset_preview& preview) {

        assert(name);

        eng_asset_preview_clear(preview);

        const asset_database* db = asset_mngr_get_database();
        if (!db || db->asset_count == 0 || name[0] == 0) return(false);

        const hnd_asset h_asset = asset_name_cstr_to_handle(name);
        const hnd_arena h_arena = eng_asset_preview_get_arena(eng_asset_preview_e_runtime);
        if (h_asset == INVALID_HANDLE || h_arena == INVALID_HANDLE) return(false);

        const asset_image* img = asset_database_load_image_by_handle_to_arena(db, h_asset, h_arena);
        if (!img || img->width == 0 || img->height == 0) return(false);

        preview.type    = asset_type_e_image;
        preview.texture = renderer_set_asset_preview_texture(eng_asset_preview_e_runtime, img->width, img->height, (const byte*)img->pixels);
        preview.width   = img->width;
        preview.height  = img->height;
        return(preview.texture != 0);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // every file has to be readable by the build, and the paths are made absolute for it
    IFB_INLINE bool
    eng_asset_database_check(
        asset_dictionary* dict,
        const cchar*      dir,
        const hnd_arena   h_arena) {

        asset_dictionary_file_list* lists[asset_type_e_count] = {
            &dict->text_files,
            &dict->image_files,
            &dict->sprite_atlas_files
        };

        // the build needs the header in its arena with each file
        const u32 count       = asset_dictionary_count_assets(dict);
        const u32 size_header = sizeof(asset_database) + (count * (sizeof(hnd_asset) + sizeof(asset_name) + sizeof(asset_index)));

        // the database only keeps the start of each name, so those have to be unique
        auto handles = (hnd_asset*)arena_push(h_arena, count * sizeof(hnd_asset));
        auto nodes   = (const asset_dictionary_file_node**)arena_push(h_arena, count * sizeof(asset_dictionary_file_node*));
        if (!handles || !nodes) {
            eng_asset_database_set_error("There is no memory to check the dictionary");
            return(false);
        }

        u32 index = 0;
        for (
            u32 type = 0;
                type < asset_type_e_count;
              ++type) {

            for (
                asset_dictionary_file_node* node = lists[type]->first;
                node != NULL;
                node = node->next) {

                if (!eng_asset_database_check_file(node, type, dir, size_header, h_arena)) return(false);

                asset_name name = {};
                name.init(node->name);
                const hnd_asset hnd = name.to_handle();

                for (
                    u32 index_other = 0;
                        index_other < index;
                      ++index_other) {

                    if (handles[index_other] != hnd) continue;

                    eng_asset_database_set_error(
                        "%s and %s are the same in the first %u characters, which is all the database keeps",
                        nodes[index_other]->name,
                        node->name,
                        IFB_CONFIG_ASSET_NAME_SIZE
                    );
                    return(false);
                }

                handles [index] = hnd;
                nodes   [index] = node;
                ++index;
            }
        }

        return(true);
    }

    // the build reads files through the file buffer, and loads each one into the arena with the header
    IFB_INLINE bool
    eng_asset_database_check_file(
        asset_dictionary_file_node* node,
        const u32                   type,
        const cchar*                dir,
        const u32                   size_header,
        const hnd_arena             h_arena) {

        cchar path[ENG_ASSET_PATH_SIZE];
        if (!eng_asset_path_to_absolute(dir, node->path, path)) {
            eng_asset_database_set_error("%s has a path that doesn't fit: %s", node->name, node->path);
            return(false);
        }

        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == FILE_HANDLE_INVALID) {
            eng_asset_database_set_error("%s can't be opened: %s", node->name, path);
            return(false);
        }
        const u32 size = file_get_size(h_file);
        file_close(h_file);

        // the last byte of the file buffer is always a terminator
        const u32 size_max = IFB_CONFIG_FILE_GRANULARITY - 1;
        if (size == 0) {
            eng_asset_database_set_error("%s is empty: %s", node->name, path);
            return(false);
        }

        // atlases are read straight into the arena, so they aren't limited by the file buffer
        if (type == asset_type_e_sprite_atlas) {

            // the atlas in memory is smaller than the file, it doesn't keep the sprite names
            const u64 size_atlas = (u64)size + sizeof(asset_sprite_atlas) + size_header;
            if (size_atlas > IFB_CONFIG_ARENA_LARGE_MEM_GRANULARITY) {
                eng_asset_database_set_error("%s is %u bytes, which is too big to load: %s", node->name, size, path);
                return(false);
            }

            const u32                 save  = arena_save(h_arena);
            const asset_sprite_atlas* atlas = asset_database_load_sprite_atlas_file(path, h_arena);
            arena_revert(h_arena, save);
            if (!atlas) {
                eng_asset_database_set_error("%s isn't a sprite atlas that can be read: %s", node->name, path);
                return(false);
            }
        }
        else if (size > size_max) {
            eng_asset_database_set_error("%s is %u bytes, files can be up to %u bytes: %s", node->name, size, size_max, path);
            return(false);
        }

        if (type == asset_type_e_image) {

            s32 width    = 0;
            s32 height   = 0;
            s32 channels = 0;
            if (!stbi_info(path, &width, &height, &channels) || width <= 0 || height <= 0) {
                eng_asset_database_set_error("%s isn't an image that can be read: %s", node->name, path);
                return(false);
            }

            // the pixels are always rgba
            const u64 size_pixels = ((u64)width * height * 4) + sizeof(image) + size_header;
            if (size_pixels > IFB_CONFIG_ARENA_LARGE_MEM_GRANULARITY) {
                eng_asset_database_set_error("%s is %d x %d, which is too big to load: %s", node->name, width, height, path);
                return(false);
            }
        }

        // the build opens the files itself, so it gets the absolute path
        const u32    length    = cstr_nvar_length(path, ENG_ASSET_PATH_SIZE);
        auto         path_copy = (cchar*)arena_push(h_arena, length + 1);
        if (!path_copy) {
            eng_asset_database_set_error("There is no memory to check the dictionary");
            return(false);
        }
        memcpy(path_copy, path, length);
        path_copy[length] = 0;

        node->path = path_copy;
        return(true);
    }

    IFB_INLINE void
    eng_asset_database_set_error(
        const cchar* format,
        ...) {

        auto& state = _eng_asset_database;

        va_list args;
        va_start  (args, format);
        vsnprintf (state.error, ENG_ASSET_ERROR_SIZE, format, args);
        va_end    (args);
    }

    IFB_INLINE void
    eng_asset_preview_clear(
        asset_preview& preview) {

        preview.type              = asset_type_e_count;
        preview.text              = NULL;
        preview.is_text_truncated = false;
        preview.texture           = 0;
        preview.width             = 0;
        preview.height            = 0;
    }

    // the arena is emptied for the next preview
    IFB_INLINE hnd_arena
    eng_asset_preview_get_arena(
        const u32 slot) {

        auto& state = _eng_asset_preview[slot];

        if (!state.is_init) {
            state.h_arena = arena_large_alloc();
            state.is_init = (state.h_arena != INVALID_HANDLE);
            if (!state.is_init) return(INVALID_HANDLE);
        }
        arena_reset(state.h_arena);

        return(state.h_arena);
    }

    // the text is copied out of the file buffer, which only holds the start of a big file
    IFB_INLINE bool
    eng_asset_preview_load_text(
        const cchar*   path,
        asset_preview& preview) {

        const hnd_arena h_arena = eng_asset_preview_get_arena(eng_asset_preview_e_dictionary);
        if (h_arena == INVALID_HANDLE) return(false);

        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == FILE_HANDLE_INVALID) return(false);

        const u32 size       = file_get_size(h_file);
        const u32 length_max = IFB_CONFIG_FILE_GRANULARITY - 1;
        const u32 length     = (size < length_max) ? size : length_max;

        auto text = (cchar*)arena_push(h_arena, length + 1);
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
        const cchar*   path,
        asset_preview& preview) {

        // always four channels, whatever the file has
        s32 width    = 0;
        s32 height   = 0;
        s32 channels = 0;
        stbi_uc* pixels = stbi_load(path, &width, &height, &channels, 4);
        if (!pixels) return(false);

        const u32 texture = (width > 0 && height > 0)
            ? renderer_set_asset_preview_texture(eng_asset_preview_e_dictionary, width, height, (const byte*)pixels)
            : 0;
        stbi_image_free(pixels);
        if (texture == 0) return(false);

        preview.texture = texture;
        preview.width   = width;
        preview.height  = height;
        return(true);
    }

    // the atlas is read from the file, and its whole image goes to a texture
    IFB_INLINE bool
    eng_asset_preview_load_sprite_atlas(
        const cchar*   path,
        asset_preview& preview) {

        const hnd_arena h_arena = eng_asset_preview_get_arena(eng_asset_preview_e_dictionary);
        if (h_arena == INVALID_HANDLE) return(false);

        const asset_sprite_atlas* atlas = asset_database_load_sprite_atlas_file(path, h_arena);
        if (!atlas) return(false);

        const u32 texture = renderer_set_asset_preview_texture(eng_asset_preview_e_dictionary, atlas->width, atlas->height, (const byte*)atlas->array_pixels);
        if (texture == 0) return(false);

        preview.texture = texture;
        preview.width   = atlas->width;
        preview.height  = atlas->height;
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

        const asset_dictionary_file_list* list = asset_dictionary_get_list(dict, type);  
        return(list);
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
            &state.dict->image_files,
            &state.dict->sprite_atlas_files
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
