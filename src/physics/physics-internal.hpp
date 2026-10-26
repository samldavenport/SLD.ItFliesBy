#ifndef PHYSICS_INTERNAL_HPP
#define PHYSICS_INTERNAL_HPP

#include "ifb-types.hpp"
#include "physics.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------
    
    struct phys_frc_accmltr; // force accumulator
    struct phys_frc_intgrtr; // force integrator
    struct phys_frc_sprcalc; // spring force calculator
    struct phys_cllsn_dtctr;  // collision detector
    struct phys_bounding_box;
    struct phys_potential_contact;
    struct phys_potential_contact_array;
    struct phys_bvh_node; // bounding volume hierarchy node

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

    IFB_INTERNAL phys_cllsn_dtctr* phys_cllsn_dtctr_create   (void);
    IFB_INTERNAL void              phys_cllsn_dtctr_run      (phys_cllsn_dtctr* dtctr);

    IFB_INTERNAL phys_bvh_node*                phys_bvh_alloc                            (const hnd_arena h_arena);
    IFB_INTERNAL bool                          phys_bvh_node_is_leaf                     (const phys_bvh_node* node); 
    IFB_INTERNAL phys_potential_contact_array* phys_bvh_node_get_potential_contacts      (const phys_bvh_node* node,   const f32 limit, const hnd_arena h_arena);
    IFB_INTERNAL bool                          phys_bvh_node_aabb_check                  (const phys_bvh_node* node_a, const phys_bvh_node* node_b);   
    IFB_INTERNAL phys_potential_contact_array* phys_bvh_node_get_potential_contacts_with (const phys_bvh_node* node_a, const phys_bvh_node* node_b, const f32 limit, const hnd_arena h_arena);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
    struct phys_bounding_box {
        vec3 center;
        vec3 half_extents;
    };

    struct phys_potiential_contact {
        entity_id a;
        entity_id b;
    };

    struct phys_potential_contact_array {
        phys_potiential_contact* contacts;
        u32                      count;
    };

    struct phys_bvh_node {
        phys_bvh_node*    child_a;
        phys_bvh_node*    child_b;
        phys_bounding_box bounding_box;
        entity_id         entity;
    };
};

#endif //PHYSICS_INTERNAL_HPP
