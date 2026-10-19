#pragma once

#include "asset-database.cpp"
#include "asset-mngr.cpp"
#include "asset-dictionary.cpp"
#include "asset-memory.cpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include <sld-strings.hpp>

namespace ifb {

    IFB_INTERNAL hnd_asset
    asset_name_cstr_to_handle(
        const cchar* name_cstr) {

        // same handle the database and manager use (upper case, zero padded)
        return(asset_database_handle_of(name_cstr));
    }
};
