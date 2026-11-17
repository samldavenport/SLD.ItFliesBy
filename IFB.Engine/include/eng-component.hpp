#ifndef ENG_COMPONENT_HPP
#define ENG_COMPONENT_HPP

#include "ifb.hpp"
#include "sld.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct cmpnt_mngr;
    struct cmpnt_table;
    
    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL cmpnt_mngr* cmpnt_mngr_create             (void);
    IFB_INTERNAL void        cmpnt_mngr_startup            (reservation* res);
    
    IFB_INTERNAL void        cmpnt_lookup_position         (const u32 sparse_index, cmpnt_position&         position);
    IFB_INTERNAL void        cmpnt_lookup_color            (const u32 sparse_index, cmpnt_color&            color);
    IFB_INTERNAL void        cmpnt_lookup_quad             (const u32 sparse_index, cmpnt_quad&             quad);
    IFB_INTERNAL void        cmpnt_lookup_velocity         (const u32 sparse_index, cmpnt_velocity&         velocity);
    IFB_INTERNAL void        cmpnt_lookup_acceleration     (const u32 sparse_index, cmpnt_acceleration&     acceleration);
    IFB_INTERNAL void        cmpnt_lookup_inv_mass         (const u32 sparse_index, cmpnt_inv_mass&         inv_mass);
    IFB_INTERNAL void        cmpnt_lookup_drag             (const u32 sparse_index, cmpnt_drag&             drag);
    IFB_INTERNAL void        cmpnt_lookup_term_velocity    (const u32 sparse_index, cmpnt_term_velocity&    term_velocity);
    IFB_INTERNAL void        cmpnt_lookup_spring           (const u32 sparse_index, cmpnt_spring&           spring);
    IFB_INTERNAL void        cmpnt_lookup_map_coords       (const u32 sparse_index, cmpnt_map_coords&       map_coords);    
    IFB_INTERNAL void        cmpnt_lookup_particle_emitter (const u32 sparse_index, cmpnt_particle_emitter& particle_emitter);    
    IFB_INTERNAL void        cmpnt_lookup_sprite           (const u32 sparse_index, cmpnt_sprite&           sprite);
    IFB_INTERNAL void        cmpnt_lookup_bounds           (const u32 sparse_index, cmpnt_bounds&           bounds);
    IFB_INTERNAL void        cmpnt_lookup_collider         (const u32 sparse_index, cmpnt_collider&         collider);

    IFB_INTERNAL void        cmpnt_update_position         (const u32 sparse_index, const cmpnt_position&         position);
    IFB_INTERNAL void        cmpnt_update_color            (const u32 sparse_index, const cmpnt_color&            color);
    IFB_INTERNAL void        cmpnt_update_quad             (const u32 sparse_index, const cmpnt_quad&             quad);
    IFB_INTERNAL void        cmpnt_update_velocity         (const u32 sparse_index, const cmpnt_velocity&         velocity);
    IFB_INTERNAL void        cmpnt_update_acceleration     (const u32 sparse_index, const cmpnt_acceleration&     acceleration);
    IFB_INTERNAL void        cmpnt_update_inv_mass         (const u32 sparse_index, const cmpnt_inv_mass&         inv_mass);
    IFB_INTERNAL void        cmpnt_update_drag             (const u32 sparse_index, const cmpnt_drag&             drag);
    IFB_INTERNAL void        cmpnt_update_term_velocity    (const u32 sparse_index, const cmpnt_term_velocity&    term_velocity);
    IFB_INTERNAL void        cmpnt_update_spring           (const u32 sparse_index, const cmpnt_spring&           spring);
    IFB_INTERNAL void        cmpnt_update_map_coords       (const u32 sparse_index, const cmpnt_map_coords&       map_coords);
    IFB_INTERNAL void        cmpnt_update_particle_emitter (const u32 sparse_index, const cmpnt_particle_emitter& particle_emitter);
    IFB_INTERNAL void        cmpnt_update_sprite           (const u32 sparse_index, const cmpnt_sprite&           sprite);
    IFB_INTERNAL void        cmpnt_update_bounds           (const u32 sparse_index, const cmpnt_bounds&           bounds);
    IFB_INTERNAL void        cmpnt_update_collider         (const u32 sparse_index, const cmpnt_collider&         collider);

    // the positions at the start of the last simulation step, so a frame can be drawn between two steps
    IFB_INTERNAL void        cmpnt_positions_begin_step       (void);
    IFB_INTERNAL void        cmpnt_positions_set_render_alpha (const f32 alpha);
    IFB_INTERNAL void        cmpnt_position_snap              (const u32 sparse_index);
    IFB_INTERNAL void        cmpnt_lookup_position_render     (const u32 sparse_index, cmpnt_position& position);

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static cmpnt_mngr* _cmpnt_mngr;
    
    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct cmpnt_mngr {
        reservation* res;
        cmpnt_table* cmpnt_tbl;
        u32          capacity;
        f32          render_alpha; // how far the frame being drawn is from the last step's start to its end, 0 to 1
    };
    
    struct cmpnt_table {
        cmpnt_position*         array_position;
        cmpnt_position*         array_position_prev;       // where each entity was when the last step started
        bool*                   array_position_prev_valid; // false for an entity created or snapped since then, it's drawn where it is
        cmpnt_color*            array_color;
        cmpnt_quad*             array_quad; 
        cmpnt_velocity*         array_velocity; 
        cmpnt_acceleration*     array_acceleration;
        cmpnt_inv_mass*         array_inv_mass;
        cmpnt_drag*             array_drag;
        cmpnt_term_velocity*    array_term_velocity; 
        cmpnt_spring*           array_spring; 
        cmpnt_map_coords*       array_map_coords;
        cmpnt_particle_emitter* array_particle_emitter;
        cmpnt_sprite*           array_sprite;
        cmpnt_bounds*           array_bounds;
        cmpnt_collider*         array_collider;
    };
};
#endif //ENG_COMPONENT_HPP
