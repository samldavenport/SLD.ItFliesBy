#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "ifb-entity.hpp"
#include <cassert>
#include <math.h>

namespace ifb {

    // how close connor can get to the edge of the map, in tiles,
    // before the camera stops following him
    static constexpr f32 GAME_PLAYER_RIG_CAMERA_MARGIN_COLS = 2.5f;
    static constexpr f32 GAME_PLAYER_RIG_CAMERA_MARGIN_ROWS = 2.0f;

    // jig follows behind connor, this far to his left or his right
    // when connor moves towards jig, jig's anchor holds its ground and connor walks past it,
    // which leaves jig behind him on the other side
    static constexpr f32 GAME_PLAYER_RIG_JIG_OFFSET_X     = 0.175f;

    // on the way across, the anchor swings out this far towards the camera
    // so jig arcs around connor instead of passing through him
    static constexpr f32 GAME_PLAYER_RIG_JIG_ARC_DEPTH    = 0.3f;

    // how fast the anchor settles on the nearest side when connor stops part way
    static constexpr f32 GAME_PLAYER_RIG_JIG_SETTLE_SPEED = 0.25f;

    // jig is always flying, so his anchor bobs up and down
    static constexpr f32 GAME_PLAYER_RIG_JIG_BOB_HEIGHT   = 0.02f;
    static constexpr f32 GAME_PLAYER_RIG_JIG_BOB_PERIOD_S = 1.6f;

    inline f32
    game_player_rig_clamp_camera_focus(
        const f32 coord,
        const f32 bounds_min,
        const f32 bounds_max,
        const f32 margin) {

        const f32 coord_min = bounds_min + margin;
        const f32 coord_max = bounds_max - margin;

        // the map is too small to follow anything, stay on the center
        if (coord_min > coord_max) return((bounds_min + bounds_max) * 0.5f);

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

        // spawn in the center of the starting map's first chunk,
        // the center of the map itself isn't always navigable
        u32 chunk_count_rows = 0;
        u32 chunk_count_cols = 0;
        u32 chunk_origin_row = 0;
        u32 chunk_origin_col = 0;
        const bool did_get_chunk = eng_map_chunk_get_dimensions(
            player_rig->starting_map_hnd,
            0,
            chunk_count_rows,
            chunk_count_cols,
            chunk_origin_row,
            chunk_origin_col
        );
        assert(did_get_chunk);

        cmpnt_map_coords spawn_coords;
        spawn_coords.h_map = player_rig->starting_map_hnd;
        spawn_coords.col_x = (f32)chunk_origin_col + ((f32)chunk_count_cols * 0.5f);
        spawn_coords.lvl_y = 0.0f;
        spawn_coords.row_z = (f32)chunk_origin_row + ((f32)chunk_count_rows * 0.5f);

        cmpnt_position spawn_pos;
        const bool did_get_spawn = eng_map_get_pos_from_coords(spawn_coords, spawn_pos);
        assert(did_get_spawn);

        // the default camera is framed around the world origin
        player_rig->camera_focus = {0};

        // jig starts on connor's left
        player_rig->jig_offset_x   = -GAME_PLAYER_RIG_JIG_OFFSET_X;
        player_rig->jig_bob_time_s = 0.0f;
        player_rig->connor_prev_x  = spawn_pos.x;
        const f32 jig_offset_x    = player_rig->jig_offset_x;

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
        quad_jig.position.x  = spawn_pos.x + jig_offset_x;
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
        anchor_pos.x = spawn_pos.x + jig_offset_x;
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

        // the anchor holds its ground while connor moves,
        // so it slides across to whichever side is behind him
        const f32 connor_delta_x   = pos_connor.x - player_rig->connor_prev_x;
        player_rig->connor_prev_x  = pos_connor.x;
        player_rig->jig_offset_x  -= connor_delta_x;

        // the first frame doesn't have a usable delta time
        f32 dt_s = eng_system_get_delta_time_s();
        if (!(dt_s >= 0.0f && dt_s < 1.0f)) dt_s = 0.0f;

        // settle on the nearest side
        const f32 settle_step = GAME_PLAYER_RIG_JIG_SETTLE_SPEED * dt_s;
        if (player_rig->jig_offset_x < 0.0f) player_rig->jig_offset_x -= settle_step;
        else                                 player_rig->jig_offset_x += settle_step;

        // the anchor never gets further away than either side
        if (player_rig->jig_offset_x < -GAME_PLAYER_RIG_JIG_OFFSET_X) player_rig->jig_offset_x = -GAME_PLAYER_RIG_JIG_OFFSET_X;
        if (player_rig->jig_offset_x >  GAME_PLAYER_RIG_JIG_OFFSET_X) player_rig->jig_offset_x =  GAME_PLAYER_RIG_JIG_OFFSET_X;

        // swing the anchor out along an arc on the way across
        // it is furthest out when it is level with connor and back in line at either side
        const f32 arc_x     = player_rig->jig_offset_x / GAME_PLAYER_RIG_JIG_OFFSET_X;
        const f32 arc_depth = sqrtf(1.0f - (arc_x * arc_x)) * GAME_PLAYER_RIG_JIG_ARC_DEPTH;

        // bob the anchor up and down, the spring carries jig along with it
        // the time wraps every period so it stays small
        player_rig->jig_bob_time_s = fmodf(player_rig->jig_bob_time_s + dt_s, GAME_PLAYER_RIG_JIG_BOB_PERIOD_S);
        const f32 bob_angle        = (player_rig->jig_bob_time_s / GAME_PLAYER_RIG_JIG_BOB_PERIOD_S) * 6.28318531f;
        const f32 bob_height       = sinf(bob_angle) * GAME_PLAYER_RIG_JIG_BOB_HEIGHT;

        pos_anchor.x = pos_connor.x + player_rig->jig_offset_x;
        pos_anchor.y = pos_connor.y + 0.100f + bob_height;
        pos_anchor.z = pos_connor.z + arc_depth;
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

        // the edges of the map are the edges of the area its chunks cover
        u32 map_row_min = 0;
        u32 map_col_min = 0;
        u32 map_row_max = 0;
        u32 map_col_max = 0;
        const bool did_get_bounds = eng_map_get_navigable_bounds(
            player_rig->current_map_hnd,
            map_row_min,
            map_col_min,
            map_row_max,
            map_col_max
        );

        if (!did_get_coords || !did_get_bounds) return;

        // the camera focuses on connor until he gets close to an edge of the map,
        // then the focus stays put and he moves around on screen
        focus_coords.col_x = game_player_rig_clamp_camera_focus(focus_coords.col_x, (f32)map_col_min, (f32)map_col_max, GAME_PLAYER_RIG_CAMERA_MARGIN_COLS);
        focus_coords.row_z = game_player_rig_clamp_camera_focus(focus_coords.row_z, (f32)map_row_min, (f32)map_row_max, GAME_PLAYER_RIG_CAMERA_MARGIN_ROWS);

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
