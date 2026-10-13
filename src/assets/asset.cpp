#pragma once

#include "asset-database.cpp"
#include "asset-mngr.cpp"
#include "asset-dictionary.cpp"
#include "ifb-config.hpp"
#include <sld-strings.hpp>

namespace ifb {

    IFB_INTERNAL hnd_asset
    asset_name_to_handle(
        const cchar* name) {

        static const auto cfg = config_instance();

        assert(name == NULL);

        const u32 length = cstr_nvar_length(name, cfg.asset_name_length); 
        
    }
};
