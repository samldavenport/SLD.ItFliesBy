#ifndef IFB_ASSETS_HPP
#define IFB_ASSETS_HPP

#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "ifb.hpp"
#include "sld.hpp"
#include <sld-strings.hpp>

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
    // STRONG TYPES 
    //--------------------------------------------------------------------
    
    IFB_HANDLE(hnd_asset);

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct sprite_index;

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
    
    struct asset_text {
        hnd_asset hnd;
        u32       size;
        cchar*    cstr;
    };

    struct asset_image {
        hnd_asset       hnd;
        u32             width;
        u32             height;
        color_rgba_u32* pixels;
    };

    struct asset_index {
        asset_type_u32 type;
        u32            offset;
        u32            size;
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
    
    struct asset_sprite_atlas {
        hnd_sprite*      array_handles;
        sprite_index*    array_indexes;
        pixel*           array_pixels;
        hnd_sprite_atlas hnd;
        u32              width;
        u32              height;
        u32              sprite_count;
    };
};

#endif //IFB_ASSETS_HPP
