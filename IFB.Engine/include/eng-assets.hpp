#ifndef ENG_ASSETS_HPP
#define ENG_ASSETS_HPP

#include "ifb-assets.hpp"
#include "ifb-config.hpp"
#include <sld-strings.hpp>

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct asset_name {
        cchar cstr [IFB_CONFIG_ASSET_NAME_SIZE];
        
        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            (void)cstr_nvar_copy(
                cstr,
                IFB_CONFIG_ASSET_NAME_SIZE,
                this->cstr,
                IFB_CONFIG_ASSET_NAME_SIZE
            );
        }

        inline hnd_asset
        to_handle(void) {

            const hnd_asset hnd = hash_u32((void*)cstr, IFB_CONFIG_ASSET_NAME_SIZE);
            assert(hnd != INVALID_HANDLE);

            return(hnd);
        }
    };
};

#endif //ENG_ASSETS_HPP
