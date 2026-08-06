#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

namespace ifb {

    enum cmpnt_type_e {
        cmpnt_type_e_none          = 0,
        cmpnt_type_e_position      = 1,
        cmpnt_type_e_color         = 2,
        cmpnt_type_e_quad          = 3,
        cmpnt_type_e_rigid_body    = 4,
        cmpnt_type_e_velocity      = 5,
        cmpnt_type_e_acceleration  = 6,
        cmpnt_type_e_inv_mass      = 7,
        cmpnt_type_e_drag          = 8,
        cmpnt_type_e_term_velocity = 9
    };

};

#endif //IFB_COMPONENTS_HPP
