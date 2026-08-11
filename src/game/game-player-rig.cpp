#pragma once

#include "ifb-component.hpp"
#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "ifb-entity.hpp"
#include <cassert>

namespace ifb {

    IFB_INTERNAL void
    player_rig_validate(
        player_rig* rig) {

        assert(rig);
        assert(rig->connor_id != ENTITY_ID_INVALID);
        assert(rig->jig_id    != ENTITY_ID_INVALID);
    }
    
    IFB_INTERNAL void 
    player_rig_init(
        player_rig* rig) {

        assert(rig); 
    
        rig->connor_id     = eng_entity_create("CONNOR");
        rig->jig_id        = eng_entity_create("JIG");
        rig->jig_anchor_id = eng_entity_create("JIG-ANCHOR");

        assert(rig->connor_id     != ENTITY_ID_INVALID);
        assert(rig->jig_id        != ENTITY_ID_INVALID);
        assert(rig->jig_anchor_id != ENTITY_ID_INVALID);

        quad_archetype quad_connor = {0};
        quad_connor.color.hex         = 0xB8BB26FF;     
        quad_connor.dimensions.width  = 0.2;
        quad_connor.dimensions.height = 0.2;
        quad_connor.position          = {0};

        quad_archetype quad_jig    = {0};
        quad_jig.color.hex         =  0x458588FF;     
        quad_jig.dimensions.width  =  0.1;
        quad_jig.dimensions.height =  0.1;
        quad_jig.position.x        = -0.175f;
        quad_jig.position.y        =  0.1f;
        quad_jig.position.z        =  0.0f;

        term_velocity_3d tv;
        tv.x = 1.00f;
        tv.y = 1.00f;
        tv.z = 1.00f;
      
        spring jig_spring;
        jig_spring.id          = rig->jig_id;
        jig_spring.anchor      = rig->jig_anchor_id;
        jig_spring.stiffness   = 40.0f;
        jig_spring.damping     = 5.0f; 
        jig_spring.rest_length = 0.001f;

        position_3d anchor_pos;
        anchor_pos.x = -0.175f;
        anchor_pos.y =  0.1f;
        anchor_pos.z =  0.0f;
                    
        const f32 inv_mass = 0.50f;
        const f32 drag     = 0.01f;
        
        eng_entity_add_components       (rig->connor_id, ENTITY_ARCHETYPE_PHYSICS_QUAD);
        eng_entity_update_quad          (rig->connor_id, quad_connor);
        eng_entity_update_inv_mass      (rig->connor_id, inv_mass);
        eng_entity_update_drag          (rig->connor_id, drag); 
        eng_entity_update_term_velocity (rig->connor_id, tv);

        eng_entity_add_components       (rig->jig_id, ENTITY_ARCHETYPE_PHYSICS_QUAD);
        eng_entity_add_components       (rig->jig_id, cmpnt_type_e_spring);
        eng_entity_update_quad          (rig->jig_id, quad_jig);
        eng_entity_update_inv_mass      (rig->jig_id, inv_mass);
        eng_entity_update_drag          (rig->jig_id, drag); 
        eng_entity_update_term_velocity (rig->jig_id, tv);
        eng_entity_update_spring        (rig->jig_id, jig_spring);

        eng_entity_add_components       (rig->jig_anchor_id, cmpnt_type_e_position);
        eng_entity_update_position      (rig->jig_anchor_id, anchor_pos);
    }
    
    IFB_INTERNAL void
    player_rig_update_and_render(
        player_rig* rig) {

        player_rig_validate(rig);     

        // get plaer input
        const bool move_left  = eng_input_is_key_down (input_keycode_e_a);
        const bool move_right = eng_input_is_key_down (input_keycode_e_d);
        const bool move_up    = eng_input_is_key_down (input_keycode_e_w);
        const bool move_down  = eng_input_is_key_down (input_keycode_e_s);

        // add movement forces to connor
        vec3 connor_force = {0};
        if (move_left)  connor_force.x -= 10.0f;
        if (move_right) connor_force.x += 10.0f;
        if (move_up)    connor_force.z -= 10.0f;
        if (move_down)  connor_force.z += 10.0f;
        eng_entity_add_force (rig->connor_id, connor_force);

        // move jig's anchor point to follow connor
        position_3d pos_anchor;
        position_3d pos_connor;
        assert(eng_entity_lookup_position(rig->jig_anchor_id, pos_anchor));
        assert(eng_entity_lookup_position(rig->connor_id,     pos_connor)); 
        pos_anchor.x = pos_connor.x - 0.175f;
        pos_anchor.y = pos_connor.y + 0.100f;
        pos_anchor.z = pos_connor.z;
        assert(eng_entity_update_position(rig->jig_anchor_id, pos_anchor));

        // render quads
        eng_entity_render(rig->connor_id);
        eng_entity_render(rig->jig_id);
    }
};
