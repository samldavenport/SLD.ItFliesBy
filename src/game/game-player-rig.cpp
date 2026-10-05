#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "ifb-entity.hpp"
#include <cassert>

namespace ifb {

    // how close connor can get to the edge of the map, in tiles,
    // before the camera stops following him
    static constexpr f32 GAME_PLAYER_RIG_CAMERA_MARGIN_COLS = 2.5f;
    static constexpr f32 GAME_PLAYER_RIG_CAMERA_MARGIN_ROWS = 2.0f;

    inline f32
    game_player_rig_clamp_camera_focus(
        const f32 coord,
        const f32 count,
        const f32 margin) {

        const f32 coord_min = margin;
        const f32 coord_max = count - margin;

        // the map is too small to follow anything, stay on the center
        if (coord_min > coord_max) return(count * 0.5f);

        if (coord < coord_min) return(coord_min);
        if (coord > coord_max) return(coord_max);
        return(coord);
    }

    IFB_INTERNAL void
    game_player_rig_validate(
        game_player_rig* player_rig) {

        assert(player_rig);
        assert(player_rig->connor_id != ENTITY_ID_INVALID);
        assert(player_rig->jig_id    != ENTITY_ID_INVALID);
    }
    
    IFB_INTERNAL void 
    game_player_rig_init(
        game_player_rig* player_rig) {

        assert(player_rig); 
    
        player_rig->connor_id     = eng_entity_create("CONNOR");
        player_rig->jig_id        = eng_entity_create("JIG");
        player_rig->jig_anchor_id = eng_entity_create("JIG-ANCHOR");

        assert(player_rig->connor_id     != ENTITY_ID_INVALID);
        assert(player_rig->jig_id        != ENTITY_ID_INVALID);
        assert(player_rig->jig_anchor_id != ENTITY_ID_INVALID);

        // spawn in the center of the starting map
        u32 map_count_rows = 0;
        u32 map_count_cols = 0;
        const bool did_get_dims = eng_map_get_dimensions(
            player_rig->starting_map_hnd,
            map_count_rows,
            map_count_cols
        );
        assert(did_get_dims);

        cmpnt_map_coords spawn_coords;
        spawn_coords.h_map = player_rig->starting_map_hnd;
        spawn_coords.col_x = (f32)map_count_cols * 0.5f;
        spawn_coords.lvl_y = 0.0f;
        spawn_coords.row_z = (f32)map_count_rows * 0.5f;

        cmpnt_position spawn_pos;
        const bool did_get_spawn = eng_map_get_pos_from_coords(spawn_coords, spawn_pos);
        assert(did_get_spawn);

        // the default camera is framed around the world origin
        player_rig->camera_focus = {0};

        atype_quad quad_connor  = {0};
        quad_connor.color.hex   = 0xB8BB26FF;
        quad_connor.quad.width  = 0.2;
        quad_connor.quad.height = 0.2;
        quad_connor.position.x  = spawn_pos.x;
        quad_connor.position.y  = spawn_pos.y + 0.1f;
        quad_connor.position.z  = spawn_pos.z;

        atype_quad quad_jig  = {0};
        quad_jig.color.hex   =  0x458588FF;
        quad_jig.quad.width  =  0.1;
        quad_jig.quad.height =  0.1;
        quad_jig.position.x  = spawn_pos.x - 0.175f;
        quad_jig.position.y  = spawn_pos.y + 0.2f;
        quad_jig.position.z  = spawn_pos.z;

        cmpnt_term_velocity tv;
        tv.x = 1.00f;
        tv.y = 1.00f;
        tv.z = 1.00f;
      
        cmpnt_spring jig_spring;
        jig_spring.id          = player_rig->jig_id;
        jig_spring.anchor      = player_rig->jig_anchor_id;
        jig_spring.stiffness   = 40.0f;
        jig_spring.damping     = 5.0f; 
        jig_spring.rest_length = 0.001f;

        cmpnt_position anchor_pos;
        anchor_pos.x = spawn_pos.x - 0.175f;
        anchor_pos.y = spawn_pos.y + 0.1f;
        anchor_pos.z = spawn_pos.z;
                    
        const f32 inv_mass = 0.50f;
        const f32 drag     = 0.01f;
        
        // connor belongs to the map, so he can't walk off of it
        eng_entity_add_components      (player_rig->connor_id, ENTITY_ARCHETYPE_PHYSICS_QUAD);
        eng_entity_add_components      (player_rig->connor_id, cmpnt_type_e_map_coords);
        eng_cmpnt_update_map_coords    (player_rig->connor_id, spawn_coords);
        eng_cmpnt_update_quad          (player_rig->connor_id, quad_connor);
        eng_cmpnt_update_inv_mass      (player_rig->connor_id, inv_mass);
        eng_cmpnt_update_drag          (player_rig->connor_id, drag); 
        eng_cmpnt_update_term_velocity (player_rig->connor_id, tv);

        eng_entity_add_components      (player_rig->jig_id, ENTITY_ARCHETYPE_PHYSICS_QUAD);
        eng_entity_add_components      (player_rig->jig_id, cmpnt_type_e_spring);
        eng_cmpnt_update_quad          (player_rig->jig_id, quad_jig);
        eng_cmpnt_update_inv_mass      (player_rig->jig_id, inv_mass);
        eng_cmpnt_update_drag          (player_rig->jig_id, drag); 
        eng_cmpnt_update_term_velocity (player_rig->jig_id, tv);
        eng_cmpnt_update_spring        (player_rig->jig_id, jig_spring);

        eng_entity_add_components      (player_rig->jig_anchor_id, cmpnt_type_e_position);
        eng_cmpnt_update_position      (player_rig->jig_anchor_id, anchor_pos);
    }
    
    IFB_INTERNAL void
    game_player_rig_update_and_render(
        game_player_rig* player_rig) {

        game_player_rig_validate(player_rig);     

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
        eng_entity_add_force (player_rig->connor_id, connor_force);

        // move jig's anchor point to follow connor
        cmpnt_position pos_anchor;
        cmpnt_position pos_connor;
        assert(eng_cmpnt_lookup_position(player_rig->jig_anchor_id, pos_anchor));
        assert(eng_cmpnt_lookup_position(player_rig->connor_id,     pos_connor)); 
        pos_anchor.x = pos_connor.x - 0.175f;
        pos_anchor.y = pos_connor.y + 0.100f;
        pos_anchor.z = pos_connor.z;
        assert(eng_cmpnt_update_position(player_rig->jig_anchor_id, pos_anchor));

        // render quads
        eng_entity_render(player_rig->connor_id);
        eng_entity_render(player_rig->jig_id);
    }

    IFB_INTERNAL void
    game_player_rig_update_camera(
        game_player_rig* player_rig) {

        game_player_rig_validate(player_rig);

        // find connor on the map
        cmpnt_map_coords focus_coords;
        const bool did_get_coords = eng_map_get_entity_coords(
            player_rig->current_map_hnd,
            player_rig->connor_id,
            focus_coords
        );

        u32 map_count_rows = 0;
        u32 map_count_cols = 0;
        const bool did_get_dims = eng_map_get_dimensions(
            player_rig->current_map_hnd,
            map_count_rows,
            map_count_cols
        );

        if (!did_get_coords || !did_get_dims) return;

        // the camera focuses on connor until he gets close to an edge of the map,
        // then the focus stays put and he moves around on screen
        focus_coords.col_x = game_player_rig_clamp_camera_focus(focus_coords.col_x, (f32)map_count_cols, GAME_PLAYER_RIG_CAMERA_MARGIN_COLS);
        focus_coords.row_z = game_player_rig_clamp_camera_focus(focus_coords.row_z, (f32)map_count_rows, GAME_PLAYER_RIG_CAMERA_MARGIN_ROWS);

        cmpnt_position focus_pos;
        if (!eng_map_get_pos_from_coords(focus_coords, focus_pos)) return;

        // move the camera as far as the focus moved
        // the origin and target move together, so the view angle doesn't change
        const f32 focus_delta_x = focus_pos.x - player_rig->camera_focus.x;
        const f32 focus_delta_z = focus_pos.z - player_rig->camera_focus.z;

        vec3 camera_origin;
        vec3 camera_target;
        eng_camera_get_origin(camera_origin);
        eng_camera_get_target(camera_target);
        camera_origin.x += focus_delta_x;
        camera_origin.z += focus_delta_z;
        camera_target.x += focus_delta_x;
        camera_target.z += focus_delta_z;
        eng_camera_set_origin(camera_origin);
        eng_camera_set_target(camera_target);

        player_rig->camera_focus.x = focus_pos.x;
        player_rig->camera_focus.z = focus_pos.z;
    }
};
