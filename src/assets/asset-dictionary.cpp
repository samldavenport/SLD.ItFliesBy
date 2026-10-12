#pragma once

#include "asset.hpp"

namespace ifb {

    struct asset_dictionary {

    };

    IFB_INTERNAL asset_dictionary* asset_dictionary_create             (const hnd_arena arena);
    IFB_INTERNAL asset_dictionary* asset_dictionary_load_from_json     (const cchar* path, const hnd_arena arena);
    IFB_INTERNAL bool              asset_dictionary_save_to_json       (asset_dictionary* dict, const cchar* path);
    IFB_INTERNAL hnd_asset         asset_dictionary_add_text_file      (asset_dictionary* dict, const cchar* path);
    IFB_INTERNAL hnd_asset         asset_dictionary_add_image_file     (asset_dictionary* dict, const cchar* path);       
    IFB_INTERNAL bool              asset_dictionary_remove_file        (asset_dictionary* dict, const cchar* path);

};
