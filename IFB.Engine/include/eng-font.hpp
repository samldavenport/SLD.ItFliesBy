#ifndef ENG_FONT_HPP
#define ENG_FONT_HPP

#include "ifb.hpp"

namespace ifb {

    // the largest atlas a font can be baked into
    static constexpr u32 FONT_ATLAS_SIZE_MAX = 4096;

    // space on the right and bottom of every glyph in the atlas, so filtering doesn't bleed into its neighbors
    static constexpr u32 FONT_ATLAS_PADDING  = 1;

    // stb_truetype's STBTT_MAX_OVERSAMPLE, which is only defined in its implementation
    static constexpr u32 FONT_OVERSAMPLE_MAX = 8;

    //--------------------------------------------------------------------
    // FONT FILE FORMAT (.ifbfont)
    //--------------------------------------------------------------------
    // - verification string | 12 bytes, FONT_FILE_VERIFICATION_CSTR without its terminator
    // - font blob           | the rest of the file, see struct font
    //--------------------------------------------------------------------

    static constexpr cchar FONT_FILE_VERIFICATION_CSTR[] = "IFBFONTATLAS";
    static constexpr u32   FONT_FILE_VERIFICATION_SIZE   = sizeof(FONT_FILE_VERIFICATION_CSTR) - 1;
    static constexpr cchar FONT_FILE_EXTENSION[]         = ".ifbfont";

    IFB_INTERNAL font* font_load_to_arena      (const hnd_file h_font_file, const hnd_arena h_arena, const font_config& cfg);
    IFB_INTERNAL bool  font_blob_is_valid      (const font* fnt, const u32 size);
    IFB_INTERNAL bool  font_file_save          (const font* fnt, const cchar* path);
    IFB_INTERNAL font* font_file_load_to_arena (const cchar* path, const hnd_arena h_arena);

};

#endif //ENG_FONT_HPP
