#pragma once

#include "asset-dictionary.cpp"
#include "asset.hpp"
#include "file-rw.cpp"
#include "ifb-types.hpp"
#include "memory.hpp"

namespace ifb {
    
    static constexpr cchar ASSET_DB_VERIFICATION_CSTR[] = "IFBASSETDB";

    struct asset_database {
        hnd_file          file_handle;
        u32               size_total;
        u32               size_header;
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
        u32 bytes_written = 0;

        file_write(db->file_handle, sizeof(ASSET_DB_VERIFICATION_CSTR), (void*)ASSET_DB_VERIFICATION_CSTR);
    }

};
