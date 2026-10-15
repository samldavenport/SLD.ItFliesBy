#pragma once

#include "asset-dictionary.cpp"
#include "asset.hpp"
#include "file-rw.cpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "memory.hpp"
#include "files.hpp"

namespace ifb {
    
    //--------------------------------------------------------------------
    // HEADER FORMAT 
    //--------------------------------------------------------------------
    // - verification string | 10 bytes
    // - asset count (n)     | 4 bytes
    // - handles             | n * 4 bytes
    // - names               | n * IFB_CONFIG_ASSET_NAME_SIZE bytes
    // - indexes             | n * 12 bytes 
    //--------------------------------------------------------------------
    // TEXT FORMAT
    //--------------------------------------------------------------------
    // - size (n)            | 4 bytes
    // - text                | n bytes 
    //--------------------------------------------------------------------
    // IMAGE FORMAT
    //--------------------------------------------------------------------
    // - pixel width (w)     | 4 bytes
    // - pixel height (h)    | 4 bytes
    // - pixels (rgba)       | w * h * 4 bytes
    //--------------------------------------------------------------------

    static constexpr cchar ASSET_DB_VERIFICATION_CSTR[] = "IFBASSETDB";

    struct asset_database {
        hnd_file          file_handle;
        u32               asset_count;
        hnd_asset*        array_handles;
        asset_name*       array_names;
        asset_index*      array_indexes;
    };

    struct asset_index {
        asset_type_u32 type;
        u32            offset;
        u32            size;
    };


    IFB_INTERNAL bool
    asset_database_read_at(
        const hnd_file h_file,
        const u32      offset,
        const u32      size,
        void*          data) {

        // like writing, the cursor doesn't move on its own
        file_set_cursor(h_file, offset);
        const u32  bytes_read = file_read_to_buffer(h_file, size, (byte*)data);
        const bool did_read   = (bytes_read == size);

        return(did_read);
    }

    IFB_INTERNAL asset_database*
    asset_database_load(
        const cchar*    path,
        const hnd_arena h_arena) {

        assert(path);
        assert(h_arena != INVALID_HANDLE);

        // open the file, the database keeps it open to read assets later
        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == INVALID_HANDLE) {
            return(NULL);
        }

        // the file needs to be at least big enough for the verification string and count
        const u32 size_file         = file_get_size(h_file);
        const u32 size_verification = sizeof(ASSET_DB_VERIFICATION_CSTR) - 1;
        const u32 offset_count      = size_verification;
        const u32 offset_handles    = offset_count + sizeof(u32);
        if (size_file < offset_handles) {
            file_close(h_file);
            return(NULL);
        }

        // check the verification string
        cchar verification[sizeof(ASSET_DB_VERIFICATION_CSTR) - 1];
        bool is_valid = asset_database_read_at(h_file, 0, size_verification, verification);
        is_valid     &= (memcmp(verification, ASSET_DB_VERIFICATION_CSTR, size_verification) == 0);

        // read the number of assets
        u32 asset_count = 0;
        is_valid &= asset_database_read_at(h_file, offset_count, sizeof(u32), &asset_count);
        is_valid &= (asset_count != 0);
        if (!is_valid) {
            file_close(h_file);
            return(NULL);
        }

        // header offsets, 64 bits so a garbage count can't overflow
        const u64 offset_names   = offset_handles + ((u64)sizeof(hnd_asset)   * asset_count); 
        const u64 offset_indexes = offset_names   + ((u64)sizeof(asset_name)  * asset_count); 
        const u64 offset_data    = offset_indexes + ((u64)sizeof(asset_index) * asset_count); 
        if (offset_data > size_file) {
            file_close(h_file);
            return(NULL);
        }

        // save the position prior to our work here
        const u32 init_save = arena_save(h_arena);

        // create the database 
        auto db = ifb_arena_push_struct(h_arena, asset_database); 
        if (!db) {
            file_close(h_file);
            return(NULL);
        }
        db->file_handle = h_file;
        db->asset_count = asset_count;

        // allocate space for the arrays 
        db->array_handles = ifb_arena_push_array(h_arena, asset_count, hnd_asset);   
        db->array_names   = ifb_arena_push_array(h_arena, asset_count, asset_name);   
        db->array_indexes = ifb_arena_push_array(h_arena, asset_count, asset_index);   
        const bool did_alloc_arrays = (
            db->array_handles != NULL &&
            db->array_names   != NULL &&
            db->array_indexes != NULL
        );
        if (!did_alloc_arrays) {
            file_close(h_file);
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        // read the arrays
        is_valid &= asset_database_read_at(h_file, offset_handles,      sizeof(hnd_asset)   * asset_count, db->array_handles);
        is_valid &= asset_database_read_at(h_file, (u32)offset_names,   sizeof(asset_name)  * asset_count, db->array_names);
        is_valid &= asset_database_read_at(h_file, (u32)offset_indexes, sizeof(asset_index) * asset_count, db->array_indexes);

        // make sure every index points at data inside the file
        for (
            u32 index = 0;
                index < asset_count && is_valid;
              ++index) {

            const asset_index& idx = db->array_indexes[index];
            const u64          end = (u64)idx.offset + idx.size;

            is_valid &= (idx.type.val < asset_type_e_count);
            is_valid &= (idx.offset   >= offset_data);
            is_valid &= (end          <= size_file);
        }

        if (!is_valid) {
            file_close(h_file);
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        return(db);
    }

    IFB_INTERNAL u32
    asset_database_write_at(
        const hnd_file h_file,
        const u32      offset,
        const u32      size,
        const void*    data) {

        // file_write doesn't move the cursor, so we always set it first
        file_set_cursor(h_file, offset);
        const u32 bytes_written = file_write(h_file, size, (const byte*)data);
        assert(bytes_written == size);

        return(bytes_written);
    }

    IFB_INTERNAL void
    asset_database_set_index(
        asset_database*                   db,
        const u32                         index,
        const asset_dictionary_file_node* node,
        const asset_type_u32              type,
        const u32                         offset,
        const u32                         size) {

        assert(index < db->asset_count);

        db->array_handles[index] = node->id;
        db->array_names  [index].init(node->name);
        db->array_indexes[index].type   = type;
        db->array_indexes[index].offset = offset;
        db->array_indexes[index].size   = size;
    }

    IFB_INTERNAL asset_database* 
    asset_database_build(
        const cchar*            path, 
        const asset_dictionary* dict,
        const hnd_arena         h_arena) {

        assert(path);
        assert(dict);
        assert(h_arena != INVALID_HANDLE);

        // count the number of assets
        const u32 asset_count = asset_dictionary_count_assets(dict); 
        if (asset_count == 0) {
            return(NULL);
        } 

        // save the position prior to our work here
        const u32 init_save = arena_save(h_arena);

        // create the database 
        auto db = ifb_arena_push_struct(h_arena, asset_database); 
        if (!db) return(NULL);
        db->asset_count = asset_count;

        // allocate space for the arrays 
        db->array_handles = ifb_arena_push_array(h_arena, asset_count, hnd_asset);   
        db->array_names   = ifb_arena_push_array(h_arena, asset_count, asset_name);   
        db->array_indexes = ifb_arena_push_array(h_arena, asset_count, asset_index);   
        const bool did_alloc_arrays = (
            db->array_handles != NULL &&
            db->array_names   != NULL &&
            db->array_indexes != NULL
        );
        if (!did_alloc_arrays) {
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        // open the file
        db->file_handle = file_rw_open_always(path);
        if (db->file_handle == INVALID_HANDLE) {
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        // header offsets, the verification string is written without its null terminator
        const u32 size_verification = sizeof(ASSET_DB_VERIFICATION_CSTR) - 1;
        const u32 offset_count      = size_verification;
        const u32 offset_handles    = offset_count   + sizeof(u32);
        const u32 offset_names      = offset_handles + (sizeof(hnd_asset)   * asset_count); 
        const u32 offset_indexes    = offset_names   + (sizeof(asset_name)  * asset_count); 
        const u32 offset_data       = offset_indexes + (sizeof(asset_index) * asset_count); 

        // the asset data goes after the header, we write the header once we know the indexes
        u32 offset_cur  = offset_data;
        u32 index_cur   = 0;

        // save our place
        const u32 write_asset_save = arena_save(h_arena);

        // write the text files to the database
        for (
            auto node = dict->text_files.first;
                 node != NULL;
                 node = node->next) {

            // open the next file, it should always exist
            const hnd_file h_file = file_ro_open_existing(node->path);
            assert(h_file != INVALID_HANDLE);

            // load the file contents 
            const text* txt = eng_text_file_load_to_arena(h_file, h_arena); 
            file_close(h_file);
            assert(txt != NULL);
     
            // write the size and text to the file
            const u32 offset_asset = offset_cur;
            offset_cur += asset_database_write_at(db->file_handle, offset_cur, sizeof(u32), &txt->size);
            if (txt->size > 0) {
                offset_cur += asset_database_write_at(db->file_handle, offset_cur, txt->size, txt->cstr);
            }

            // update the index
            asset_database_set_index(db, index_cur, node, asset_type_e_text, offset_asset, offset_cur - offset_asset);
            ++index_cur;

            // revert the arena
            arena_revert(h_arena, write_asset_save);
        }

        // write the image files to the database
        for (
            auto node = dict->image_files.first;
                 node != NULL;
                 node = node->next) {

            // open the next file, it should always exist
            const hnd_file h_file = file_ro_open_existing(node->path);
            assert(h_file != INVALID_HANDLE);

            // load the image, always rgba
            const image* img = eng_image_load_to_arena(h_file, h_arena);
            file_close(h_file);
            assert(img != NULL);

            // write the dimensions and pixels to the file
            const u32 offset_asset = offset_cur;
            offset_cur += asset_database_write_at(db->file_handle, offset_cur, sizeof(u32), &img->pixel_width);
            offset_cur += asset_database_write_at(db->file_handle, offset_cur, sizeof(u32), &img->pixel_height);
            if (img->data.size > 0) {
                offset_cur += asset_database_write_at(db->file_handle, offset_cur, img->data.size, img->data.vptr);
            }

            // update the index
            asset_database_set_index(db, index_cur, node, asset_type_e_image, offset_asset, offset_cur - offset_asset);
            ++index_cur;

            // revert the arena
            arena_revert(h_arena, write_asset_save);
        }
        assert(index_cur == asset_count);

        // now that the indexes are filled in, write the header
        (void)asset_database_write_at(db->file_handle, 0,              size_verification,                   ASSET_DB_VERIFICATION_CSTR);
        (void)asset_database_write_at(db->file_handle, offset_count,   sizeof(u32),                         &db->asset_count);
        (void)asset_database_write_at(db->file_handle, offset_handles, sizeof(hnd_asset)   * asset_count,   db->array_handles);
        (void)asset_database_write_at(db->file_handle, offset_names,   sizeof(asset_name)  * asset_count,   db->array_names);
        (void)asset_database_write_at(db->file_handle, offset_indexes, sizeof(asset_index) * asset_count,   db->array_indexes);

        return(db);
    }

};
