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

    IFB_INTERNAL font* font_load_to_arena (const hnd_file h_font_file, const hnd_arena h_arena, const font_config& cfg);

};

#endif //ENG_FONT_HPP
