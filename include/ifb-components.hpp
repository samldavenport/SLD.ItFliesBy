#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {


    enum component_type_e {
        component_type_e_none     = 0,
        component_type_e_position = 1,
        component_type_e_color    = 2
    };

    struct component_type {
        u32 val;

        component_type () = default;
        component_type (const u32 val) : val(val) { }

        inline bool operator== (const u32 other)            { return(val == other);     }
        inline bool operator!= (const u32 other)            { return(val != other);     }
        inline bool operator== (const component_type other) { return(val == other.val); }
        inline bool operator!= (const component_type other) { return(val != other.val); }
    };

};

#endif //IFB_COMPONENTS_HPP