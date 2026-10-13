#pragma once

#include "asset.hpp"
#include "ifb-types.hpp"

namespace ifb {

    struct asset_database {
        hnd_file     file_handle;
        u32          size_total;
        u32          size_header;
        u32          asset_count;
        hnd_asset*   array_handles;
        asset_name*  array_names;
        asset_index* array_indexes;
    };

    struct asset_index {
        asset_type_u32 type;
        u32            offset;
        u32            size;
    };

    IFB_INTERNAL asset_dictionary*
    asset_dictionary_create(
        const hnd_arena arena) {

        assert(arena != INVALID_HANDLE);


    }
    
    IFB_INTERNAL void
    asset_dictionary_file_load(
        asset_database* db,
        const cchar*    path,
        const hnd_arena h_arena) {

    }

    IFB_INTERNAL void
    asset_dictionary_file_build(
        asset_database*         db,
        const cchar*            path,
        const asset_dictionary* dict,
        const hnd_arena         h_arena) {

    }
};
