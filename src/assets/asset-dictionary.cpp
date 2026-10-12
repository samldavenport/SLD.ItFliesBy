#pragma once

#include "asset.hpp"
#include "file-common.cpp"
#include "file-ro.cpp"
#include "file-wo.cpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "json.cpp"
#include "memory-arena.cpp"
#include "sld.hpp"
#include "sld-strings.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    struct asset_dictionary_file_node {
        asset_dictionary_file_node* next;
        const cchar*                path;
        const cchar*                name;
        u32                         id;
    };

    struct asset_dictionary_file_list {
        asset_dictionary_file_node* first;
        asset_dictionary_file_node* last;
    };

    struct asset_dictionary {
        asset_dictionary_file_list text_files;
        asset_dictionary_file_list image_files;
    };
 
    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------
    
    inline u32       asset_dictionary_json_write_cstr          (cchar* buffer, const u32 offset,const cchar* cstr); 
    inline u32       asset_dictionary_file_list_json_size      (const asset_dictionary_file_list& list); 
    inline bool      asset_dictionary_file_list_load_from_json (asset_dictionary_file_list& list,const json_arr* arr,const hnd_arena h_arena);
    inline bool      asset_dictionary_file_list_remove_node    (asset_dictionary_file_list& list,const u32 id); 
    inline u32       asset_dictionary_json_write_cstr          (cchar* buffer,const u32 offset, const cchar* cstr); 
    inline u32       asset_dictionary_file_list_json_size      (const asset_dictionary_file_list& list); 
    inline bool      asset_dictionary_file_list_load_from_json (asset_dictionary_file_list& list,const json_arr* arr,const hnd_arena h_arena);
    inline bool      asset_dictionary_file_list_remove_node    (asset_dictionary_file_list& list, const u32 id);
    inline hnd_asset asset_dictionary_file_list_add_node       (asset_dictionary_file_list& list, const cchar* name, const cchar* path, const hnd_arena h_arena);
    inline u32       asset_dictionary_file_list_json_write     (const asset_dictionary_file_list& list, const cchar* name, const bool is_last, cchar* buffer, const u32 offset); 

    //--------------------------------------------------------------------
    // INTERNAL METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL asset_dictionary*
    asset_dictionary_create(
        const hnd_arena h_arena) {

        auto dict = (asset_dictionary*)arena_push(h_arena, sizeof(asset_dictionary));
        if (dict) {
            dict->text_files.first  = NULL;
            dict->text_files.last   = NULL;
            dict->image_files.first = NULL;
            dict->image_files.last  = NULL;
        }
        return(dict);
    }

    IFB_INTERNAL asset_dictionary*
    asset_dictionary_load_from_json(
        const cchar*    path,
        const hnd_arena h_arena) {

        assert(path);
        assert(h_arena != INVALID_HANDLE);

        // read the json data
        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == FILE_HANDLE_INVALID) return(NULL);

        const u32 size = file_get_size(h_file);
        if (size == 0 || size >= IFB_CONFIG_FILE_GRANULARITY) {
            file_close(h_file);
            return(NULL);
        }
        const cchar* data = file_read(h_file, size);

        // everything is pushed on the arena, so we can undo it all on failure
        const u32 save = arena_save(h_arena);

        // parsing copies the strings into the arena, so we are done with the file
        const json_doc* doc = json_doc_create(h_arena, size, data);
        file_close(h_file);

        asset_dictionary* dict = asset_dictionary_create(h_arena);

        bool did_load = (dict != NULL);
        if (did_load) {
            const json_arr* arr_text  = json_doc_get_arr(doc, "text");
            const json_arr* arr_image = json_doc_get_arr(doc, "image");

            did_load &= asset_dictionary_file_list_load_from_json(dict->text_files,  arr_text,  h_arena);
            did_load &= asset_dictionary_file_list_load_from_json(dict->image_files, arr_image, h_arena);
        }

        if (!did_load) {
            arena_revert(h_arena, save);
            return(NULL);
        }

        return(dict);
    }

    IFB_INTERNAL bool
    asset_dictionary_save_to_json(
        asset_dictionary* dict,
        const cchar*      path,
        const hnd_arena   h_arena) {

        assert(dict);
        assert(path);
        assert(h_arena != INVALID_HANDLE);

        // the arena is only scratch space for the json text
        const u32 save = arena_save(h_arena);

        // the braces, newlines and terminator, plus the lists
        u32 size = 8;
        size += asset_dictionary_file_list_json_size(dict->text_files);
        size += asset_dictionary_file_list_json_size(dict->image_files);

        auto buffer = (cchar*)arena_push(h_arena, size);
        if (!buffer) return(false);

        u32 length = 0;
        length = asset_dictionary_json_write_cstr      (buffer, length, "{\n");
        length = asset_dictionary_file_list_json_write (dict->text_files,  "text",  false, buffer, length);
        length = asset_dictionary_file_list_json_write (dict->image_files, "image", true,  buffer, length);
        length = asset_dictionary_json_write_cstr      (buffer, length, "}\n");
        assert(length < size);
        buffer[length] = 0;

        // write the json data
        bool did_save = false;
        const hnd_file h_file = file_wo_overwrite(path);
        if (h_file != FILE_HANDLE_INVALID) {
            const u32 bytes_written = file_write(h_file, length, (const byte*)buffer);
            did_save = (bytes_written == length);
            file_close(h_file);
        }

        arena_revert(h_arena, save);
        return(did_save);
    }

    IFB_INTERNAL hnd_asset
    asset_dictionary_add_text_file(
        asset_dictionary* dict,
        const cchar*      name,
        const cchar*      path,
        const hnd_arena   h_arena) {

        const hnd_asset hnd = asset_dictionary_file_list_add_node(dict->text_files, name, path, h_arena);
        return(hnd);
    }

    IFB_INTERNAL hnd_asset
    asset_dictionary_add_image_file(
        asset_dictionary* dict,
        const cchar*      name,
        const cchar*      path,
        const hnd_arena   h_arena) {

        const hnd_asset hnd = asset_dictionary_file_list_add_node(dict->image_files, name, path, h_arena);
        return(hnd);
    }

    IFB_INTERNAL bool
    asset_dictionary_remove_file(
        asset_dictionary* dict,
        const cchar*      name) {

        assert(dict);
        assert(name);

        const u32 length = cstr_nvar_length(name, IFB_CONFIG_FILE_PATH_SIZE);
        assert(length != 0);

        // files are identified by the hash of their name, same as when they are added
        const u32 id = hash_u32((void*)name, length);

        bool did_remove = asset_dictionary_file_list_remove_node(dict->text_files, id);
        if (!did_remove) {
            did_remove = asset_dictionary_file_list_remove_node(dict->image_files, id);
        }

        return(did_remove);
    }

    //--------------------------------------------------------------------
    // INLINE METHODS 
    //--------------------------------------------------------------------
    
    inline hnd_asset
    asset_dictionary_file_list_add_node(
        asset_dictionary_file_list& list,
        const cchar*                name,
        const cchar*                path,
        const hnd_arena             h_arena) {

        assert(name);
        assert(path);
        assert(h_arena != INVALID_HANDLE);

        auto node_new = (asset_dictionary_file_node*)arena_push(h_arena, sizeof(asset_dictionary_file_node));
        if (!node_new) return(INVALID_HANDLE);

        const u32 length = cstr_nvar_length(name, IFB_CONFIG_FILE_PATH_SIZE);
        assert(length != 0);

        node_new->id   = hash_u32((void*)name, length);
        node_new->name = name;
        node_new->path = path;
        node_new->next = NULL;

        if (list.first == NULL) {
            assert(list.last == NULL);

            list.first       = node_new;
            list.last        = node_new;
        }
        else {
            assert(list.last != NULL);
            if (list.first->next == NULL) {
                assert(list.first == list.last);
                list.first->next = node_new;
            }

            list.last->next = node_new;
            list.last       = node_new;
        }

        return(node_new->id);
    }

    inline bool
    asset_dictionary_file_list_remove_node(
        asset_dictionary_file_list& list,
        const u32                   id) {

        asset_dictionary_file_node* node_prev = NULL;

        for (
            asset_dictionary_file_node* node = list.first;
            node != NULL;
            node = node->next) {

            if (node->id == id) {

                if (node_prev)          node_prev->next = node->next;
                if (node == list.first) list.first      = node->next;
                if (node == list.last)  list.last       = node_prev;

                // the node memory stays in the arena
                node->next = NULL;
                return(true);
            }

            node_prev = node;
        }

        return(false);
    }

    inline bool
    asset_dictionary_file_list_load_from_json(
        asset_dictionary_file_list& list,
        const json_arr*             arr,
        const hnd_arena             h_arena) {

        // a missing array is an empty list
        if (!arr) return(true);

        const u32 count = json_arr_get_count(arr);
        for (
            u32 index = 0;
            index < count;
            ++index) {

            const json_obj* obj = json_arr_get_obj(arr, index);
            if (!obj) return(false);

            // the name and path live in the json document, which is in the same arena
            const cchar* name = json_obj_get_cstr(obj, "name");
            const cchar* path = json_obj_get_cstr(obj, "path");
            if (!name || !path) return(false);

            const hnd_asset hnd = asset_dictionary_file_list_add_node(list, name, path, h_arena);
            if (hnd == INVALID_HANDLE) return(false);
        }

        return(true);
    }


    inline u32
    asset_dictionary_file_list_json_size(
        const asset_dictionary_file_list& list) {

        // the list name, brackets and whitespace
        u32 size = 32;

        for (
            const asset_dictionary_file_node* node = list.first;
            node != NULL;
            node = node->next) {

            // every character in the name and path could be escaped, plus the object syntax
            const u32 length_name = cstr_nvar_length(node->name, IFB_CONFIG_FILE_PATH_SIZE);
            const u32 length_path = cstr_nvar_length(node->path, IFB_CONFIG_FILE_PATH_SIZE);
            size += (length_name * 2) + (length_path * 2) + 64;
        }

        return(size);
    }

    inline u32
    asset_dictionary_json_write_cstr(
        cchar*       buffer,
        const u32    offset,
        const cchar* cstr) {

        u32 cursor = offset;
        for (
            const cchar* c = cstr;
            *c != 0;
            ++c) {

            buffer[cursor] = *c;
            ++cursor;
        }

        return(cursor);
    }

    inline u32
    asset_dictionary_json_write_path(
        cchar*       buffer,
        const u32    offset,
        const cchar* path) {

        const u32 length = cstr_nvar_length(path, IFB_CONFIG_FILE_PATH_SIZE);

        u32 cursor = offset;
        for (
            u32 index = 0;
            index < length && path[index] != 0;
            ++index) {

            const cchar c = path[index];

            // windows paths are full of backslashes
            if (c == '"' || c == '\\') {
                buffer[cursor] = '\\';
                ++cursor;
            }

            buffer[cursor] = c;
            ++cursor;
        }

        return(cursor);
    }

    inline u32
    asset_dictionary_file_list_json_write(
        const asset_dictionary_file_list& list,
        const cchar*                      name,
        const bool                        is_last,
        cchar*                            buffer,
        const u32                         offset) {

        u32 cursor = offset;

        cursor = asset_dictionary_json_write_cstr(buffer, cursor, "    \"");
        cursor = asset_dictionary_json_write_cstr(buffer, cursor, name);
        cursor = asset_dictionary_json_write_cstr(buffer, cursor, "\": [\n");

        for (
            const asset_dictionary_file_node* node = list.first;
            node != NULL;
            node = node->next) {

            cursor = asset_dictionary_json_write_cstr(buffer, cursor, "        { \"name\": \"");
            cursor = asset_dictionary_json_write_path(buffer, cursor, node->name);
            cursor = asset_dictionary_json_write_cstr(buffer, cursor, "\", \"path\": \"");
            cursor = asset_dictionary_json_write_path(buffer, cursor, node->path);
            cursor = asset_dictionary_json_write_cstr(buffer, cursor, node->next ? "\" },\n" : "\" }\n");
        }

        cursor = asset_dictionary_json_write_cstr(buffer, cursor, is_last ? "    ]\n" : "    ],\n");

        return(cursor);
    }
};
