#ifndef IFB_RANDOM_HPP
#define IFB_RANDOM_HPP

#include "ifb-types.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DECLARATIONS
    //--------------------------------------------------------------------

    struct random_table;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL random_table* random_table_create  (void);
    IFB_INTERNAL void          random_table_refresh (void);
    IFB_INTERNAL void          random_table_update  (void); // once per frame

    IFB_INTERNAL u32           random_u32           (void);
    IFB_INTERNAL f32           random_f32           (void); // [0.0, 1.0)
    IFB_INTERNAL f32           random_f32_range     (const f32 min, const f32 max);
};

#endif //IFB_RANDOM_HPP
