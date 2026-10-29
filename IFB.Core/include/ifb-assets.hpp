#ifndef IFB_ASSETS_HPP
#define IFB_ASSETS_HPP

#include "ifb-config.hpp"
#include "ifb.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
   
    enum asset_type_e {
        asset_type_e_text          = 0,
        asset_type_e_image         = 1,
        asset_type_e_sprite_atlas  = 2,
        asset_type_e_count         = 3 
    };

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct asset_database_info {
        cchar     name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_asset hnd;
        u32       type;
        u32       offset;
        u32       size;
    };

    struct sprite_atlas_builder_info {
        cchar     name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_asset hnd;
        u32       width;
        u32       height;
    };
    
    struct asset_dictionary_file_info {
        const cchar* name;
        const cchar* path;
        u32          id;
        u32          type;
    };

    struct asset_preview {
        u32          type;
        const cchar* text;
        b32          is_text_truncated; // the file is bigger than the file buffer
        u32          texture;
        u32          width;
        u32          height;
    };
    
};

#endif //IFB_ASSETS_HPP
