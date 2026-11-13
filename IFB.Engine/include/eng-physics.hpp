#ifndef ENG_PHYSICS_HPP
#define ENG_PHYSICS_HPP

#include "ifb.hpp"
#include "sld-math-types.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPE DECLARATIONS
    //--------------------------------------------------------------------

    struct phys_mngr;         // manager
    struct phys_frc_accmltr;  // force accumulator
    struct phys_frc_intgrtr;  // force integrator
    struct phys_frc_sprcalc;  // spring force calculator
    struct phys_cllsn_dtctr;  // collision detector
    struct phys_cllsn_rslvr;  // collision resolver
    struct phys_cllsn_list;   // collision list
    struct phys_cllsn_pair;   // collision pair 
    struct phys_cllsn_mask;   // collision mask
    struct phys_bvh;          // bounding volume hierarchy
    struct phys_bvh_node;     // bounding volume hierarchy node
    struct phys_aabb;         // aabb bounding box
    struct phys_cntct_list;   // contact list
    struct phys_cntct;        // contact

    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------

    static phys_mngr* _phys_mngr;

    // boxes this close count as touching, even though they don't overlap
    // resolution pushes a pair back out to exactly touching, so without it a resting contact would end
    // it's only for the contacts, nothing is pushed until the boxes actually overlap
    // about a pixel and a half on a 0.2 wide character quad
    static constexpr f32 PHYS_CNTCT_SKIN = 0.005f;

    //--------------------------------------------------------------------
    // METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL phys_mngr*        phys_mngr_create             (void);
    IFB_INTERNAL void              phys_mngr_validate           (void);
    IFB_INTERNAL void              phys_mngr_startup            (reservation* res);
    IFB_INTERNAL void              phys_mngr_shutdown           (void);
    IFB_INTERNAL void              phys_mngr_simulate           (const hnd_map map, const f32 dt);

    IFB_INTERNAL void              phys_entity_add_force        (const u32 sparse_index, const vec3& force);
    IFB_INTERNAL bool              phys_entity_remove_all_force (const entity_id id);
    IFB_INTERNAL void              phys_entity_clear_force      (const u32 sparse_index);
    IFB_INTERNAL void              phys_entity_set_mass         (const entity_id id, const f32 mass);
    IFB_INTERNAL void              phys_entity_set_inv_mass     (const entity_id id, const f32 inv_mass);
    
    IFB_INTERNAL void*             phys_mngr_res_alloc         (const u32 size_min);
    IFB_INTERNAL phys_frc_accmltr* phys_mngr_get_frc_accmltr   (void);

    IFB_INTERNAL phys_frc_accmltr* phys_frc_accmltr_create     (void);
    IFB_INTERNAL void              phys_frc_accmltr_validate   (const phys_frc_accmltr* const accum);
    IFB_INTERNAL void              phys_frc_accmltr_add        (phys_frc_accmltr* const accum, const u32 sparse_index, const vec3& v);
    IFB_INTERNAL const vec3&       phys_frc_accmltr_get        (const phys_frc_accmltr* const accum, const u32 sparse_index);
    IFB_INTERNAL void              phys_frc_accmltr_clear      (phys_frc_accmltr* const accum, const u32 sparse_index);
    IFB_INTERNAL void              phys_frc_accmltr_reset      (phys_frc_accmltr* const accum);
   
    IFB_INTERNAL phys_frc_intgrtr* phys_frc_intgrtr_create     (void); 
    IFB_INTERNAL void              phys_frc_intgrtr_run        (phys_frc_intgrtr* integrator, const phys_frc_accmltr* accum, const f32 dt);
    IFB_INTERNAL phys_frc_sprcalc* phys_frc_sprcalc_create     (void);
    IFB_INTERNAL void              phys_frc_sprcalc_run        (phys_frc_sprcalc* calc);

    IFB_INTERNAL phys_cllsn_dtctr* phys_cllsn_dtctr_create     (void);
    IFB_INTERNAL void              phys_cllsn_dtctr_run        (phys_cllsn_dtctr* dtctr, const f32 dt);
    IFB_INTERNAL phys_cllsn_list*  phys_cllsn_dtctr_get_list   (const phys_cllsn_dtctr* dtctr);

    IFB_INTERNAL phys_cllsn_rslvr* phys_cllsn_rslvr_create     (void);
    IFB_INTERNAL void              phys_cllsn_rslvr_run        (phys_cllsn_rslvr* rslvr, const phys_cllsn_list* cllsn_list, phys_cntct_list* cntct_list, const f32 dt);

    IFB_INTERNAL phys_cllsn_list*  phys_cllsn_list_create      (void); 
    IFB_INTERNAL u32               phys_cllsn_list_get_count   (const phys_cllsn_list* list);
    IFB_INTERNAL phys_cllsn_pair   phys_cllsn_list_get_pair    (const phys_cllsn_list* list, const u32 index);
    IFB_INTERNAL bool              phys_cllsn_list_insert_pair (phys_cllsn_list*       list, const phys_cllsn_pair& pair);
    IFB_INTERNAL void              phys_cllsn_list_remove_pair (phys_cllsn_list*       list, const u32 index);
    IFB_INTERNAL void              phys_cllsn_list_reset       (phys_cllsn_list*       list);

    IFB_INTERNAL phys_cntct_list*  phys_cntct_list_create      (void);
    IFB_INTERNAL u32               phys_cntct_list_get_count   (const phys_cntct_list* list);
    IFB_INTERNAL phys_cntct        phys_cntct_list_get         (const phys_cntct_list* list, const u32 index);
    IFB_INTERNAL u32               phys_cntct_list_find        (const phys_cntct_list* list, const entity_id a, const entity_id b);
    IFB_INTERNAL bool              phys_cntct_list_insert      (phys_cntct_list*       list, const phys_cntct& cntct);
    IFB_INTERNAL void              phys_cntct_list_reset       (phys_cntct_list*       list);

    // called once for every contact that has a phase this step
    using fptr_phys_cntct_phase_proc = void (*) (const phys_cntct& cntct, const phys_cllsn_phase_u32 phase);
    IFB_INTERNAL u32               phys_cntct_list_walk_phases (const phys_cntct_list* curr, const phys_cntct_list* prev, const fptr_phys_cntct_phase_proc proc);

    IFB_INTERNAL void              phys_cntct_dispatch         (const phys_cntct& cntct, const phys_cllsn_phase_u32 phase);

    IFB_INTERNAL phys_bvh*         phys_bvh_create             (const u32 entity_count, const hnd_arena h_arena);
    IFB_INTERNAL void              phys_bvh_build              (phys_bvh* bvh, const entity_id* ids, const phys_aabb* aabbs, const phys_cllsn_mask* masks, const u32 count);
    IFB_INTERNAL u32               phys_bvh_query_pairs        (const phys_bvh* bvh, phys_cllsn_list* list);

    IFB_INTERNAL void              phys_aabb_reset             (phys_aabb& aabb); 
    IFB_INTERNAL void              phys_aabb_grow              (phys_aabb& aabb_curr, const phys_aabb& aabb_new); 
    IFB_INTERNAL void              phys_aabb_grow_pt           (phys_aabb& aabb_curr, const vec3& pt); 
    IFB_INTERNAL bool              phys_aabb_check             (const phys_aabb& aabb_a, const phys_aabb& aabb_b);
    IFB_INTERNAL f32               phys_aabb_area              (const phys_aabb& aabb);
    IFB_INTERNAL vec3              phys_aabb_centroid          (const phys_aabb& aabb);
    IFB_INTERNAL void              phys_aabb_from_center       (phys_aabb& aabb, const vec3& center, const vec3& half_extent);
    IFB_INTERNAL vec3              phys_aabb_half_extent_quad  (const cmpnt_quad& quad);
    IFB_INTERNAL bool              phys_aabb_get_body_box      (const entity& e, vec3& offset, vec3& half_extent);

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS
    //--------------------------------------------------------------------
    
    struct phys_aabb {
        vec3 min;
        vec3 max;
    };

    // interior node: count == 0, children are at child_left_or_first and child_left_or_first + 1
    // leaf node:     count >  0, entities are at array_entity_ids[child_left_or_first .. child_left_or_first + count)
    struct phys_bvh_node {
        phys_aabb bnd_box;
        u32       child_left_or_first;
        u32       count;
    };

    struct phys_bvh {
        phys_bvh_node*   array_nodes;
        entity_id*       array_entity_ids;
        phys_aabb*       array_entity_aabbs;
        phys_cllsn_mask* array_cllsn_masks;
        u32              count_nodes;
        u32              count_entities;
        u32              capacity_nodes;
        u32              capacity_entities;
    };

    struct phys_cllsn_pair {
        entity_id a;
        entity_id b;
    };


    struct phys_cllsn_mask {
        u32 category;
        u32 collides_with;
    };

    // the pairs that actually overlap after integration
    // a is always the lower id, and the normal points from a to b
    struct phys_cntct_list {
        entity_id* array_a;
        entity_id* array_b;
        vec3*      array_normal;
        f32*       array_depth;
        u32        count;
    };

    // one entry of the contact list
    struct phys_cntct {
        entity_id a;
        entity_id b;
        vec3      normal;
        f32       depth;
    };
};

#endif //ENG_PHYSICS_HPP
