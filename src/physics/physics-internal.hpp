#ifndef PHYSICS_INTERNAL_HPP
#define PHYSICS_INTERNAL_HPP

#include "physics.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------
    
    struct phys_frc_accmltr; // force accumulator
    struct phys_frc_intgrtr; // force integrator
    struct phys_frc_sprcalc; // spring force calculator

    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------

    static phys_mngr* _phys_mngr;

    //--------------------------------------------------------------------
    // METHOD DECLARATIONS
    //--------------------------------------------------------------------
   
    IFB_INTERNAL void*             phys_mngr_res_alloc       (const u32 size_min);
    IFB_INTERNAL phys_frc_accmltr* phys_mngr_get_frc_accmltr (void);

    IFB_INTERNAL phys_frc_accmltr* phys_frc_accmltr_create   (void);
    IFB_INTERNAL void              phys_frc_accmltr_validate (phys_frc_accmltr* const accum);
    IFB_INTERNAL void              phys_frc_accmltr_add      (phys_frc_accmltr* const accum, const entity_id id, const vec3& v);
    IFB_INTERNAL bool              phys_frc_accmltr_lookup   (phys_frc_accmltr* const accum, const entity_id id, vec3& v);
    IFB_INTERNAL bool              phys_frc_accmltr_remove   (phys_frc_accmltr* const accum, const entity_id id);
    IFB_INTERNAL void              phys_frc_accmltr_reset    (phys_frc_accmltr* const accum);
   
    IFB_INTERNAL phys_frc_intgrtr* phys_frc_intgrtr_create   (void); 
    IFB_INTERNAL void              phys_frc_intgrtr_run      (phys_frc_intgrtr* integrator, const phys_frc_accmltr* accum, const f32 dt);

    IFB_INTERNAL phys_frc_sprcalc* phys_frc_sprcalc_create   (void);
    IFB_INTERNAL void              phys_frc_sprcalc_run      (phys_frc_sprcalc* calc);
   

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
};

#endif //PHYSICS_INTERNAL_HPP
