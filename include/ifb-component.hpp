#ifndef IFB_COMPONENTS_HPP
#define IFB_COMPONENTS_HPP

#include <sld.hpp>

#include "ifb-types.hpp"

using namespace sld;

namespace ifb {

    enum cmpnt_type_e {
        cmpnt_type_e_none          = bit_value(0),
        cmpnt_type_e_position      = bit_value(1),
        cmpnt_type_e_color         = bit_value(2),
        cmpnt_type_e_quad          = bit_value(3),
        cmpnt_type_e_rigid_body    = bit_value(4),
        cmpnt_type_e_velocity      = bit_value(5),
        cmpnt_type_e_acceleration  = bit_value(6),
        cmpnt_type_e_inv_mass      = bit_value(7),
        cmpnt_type_e_drag          = bit_value(8),
        cmpnt_type_e_term_velocity = bit_value(9),
        cmpnt_type_e_spring        = bit_value(10)
    };

};

#endif //IFB_COMPONENTS_HPP
