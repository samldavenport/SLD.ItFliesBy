#ifndef ENG_FONT_HPP
#define ENG_FONT_HPP

#include "ifb.hpp"

namespace ifb {

    IFB_INTERNAL font* font_load_to_arena (const hnd_file h_font_file, const hnd_arena h_arena); 

};

#endif //ENG_FONT_HPP
