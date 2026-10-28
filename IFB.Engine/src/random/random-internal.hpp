#ifndef RANDOM_INTERNAL_HPP
#define RANDOM_INTERNAL_HPP

#include "ifb-random.hpp"

namespace ifb {

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

#endif //RANDOM_INTERNAL_HPP
