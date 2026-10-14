#pragma once

#include "asset-dictionary.cpp"
#include "asset.hpp"
#include "file-rw.cpp"
#include "ifb-types.hpp"
#include "memory.hpp"
#include "files.hpp"

namespace ifb {
    
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


    IFB_INTERNAL asset_database*
    asset_database_load(
        const cchar*    path,
        const hnd_arena h_arena) {

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
        }

        // open the file
        db->file_handle = file_rw_open_always(path);
        assert(db->file_handle != INVALID_HANDLE);

        // keep track of how much we write to the file
        u32 bytes_written  = 0;
        u32 assets_written = 0; 

        // write the string
        bytes_written += file_write(
            db->file_handle,
            sizeof(ASSET_DB_VERIFICATION_CSTR),
            (byte*)ASSET_DB_VERIFICATION_CSTR
        );
  
        // write the number of assets
        bytes_written += file_write(db->file_handle, sizeof(u32), (byte*)&bytes_written);

        // write the arrays, as temporary data
        bytes_written += file_write(db->file_handle, asset_count * sizeof(hnd_asset),   (byte*)db->array_handles);
        bytes_written += file_write(db->file_handle, asset_count * sizeof(asset_name),  (byte*)db->array_names);
        bytes_written += file_write(db->file_handle, asset_count * sizeof(asset_index), (byte*)db->array_indexes);
    
        // write the text files to the database
        for (
            auto node = dict->text_files.first;
                 node != NULL;
                 node = node->next) {

            
        }
    }

};
