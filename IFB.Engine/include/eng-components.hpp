#ifndef ENG_COMPONENTS_HPP
#define ENG_COMPONENTS_HPP

#include "ifb-components.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    using  cmpnt_acceleration  = vec3;

    struct cmpnt_inv_mass {
        f32 normal_val;
    };

    struct cmpnt_drag {
        f32 normal_val;
    };
};

#endif //ENG_COMPONENTS_HPP
