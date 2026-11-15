#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-entity.cpp"
#include "game-player-rig.cpp"
#include <cassert>

namespace ifb {

    // the dev scene's state, in its scene memory, NULL while it isn't the active scene
    struct game_scene_dev {
        hnd_scene       h_scene;
        game_player_rig rig;
        game_map        map;
        game_entities   entities;
    } static * _scene_dev;

    IFB_INTERNAL void
    game_scene_dev_box_proc(
        const phys_cllsn_contact& contact) {

        assert(_game_ctx);
        assert(_scene_dev);

        auto player_rig = &_scene_dev->rig;
        auto entities   = &_scene_dev->entities;

        if (contact.other != player_rig->connor_id)  return;
        if (contact.phase == phys_cllsn_phase_e_end) return;
        if (!_game_ctx->is_action_pressed)           return;

        _game_ctx->is_action_pressed = false;

        static constexpr cchar text_box_1[] = "This is box 1";
        static constexpr cchar text_box_2[] = "This is box 2";
        static constexpr cchar text_box_3[] = "This is box 3";
        static constexpr cchar text_box_4[] = "This is box 4";

        const cchar* text = "";
        if      (contact.self == entities->static_entity_0) text = text_box_1;
        else if (contact.self == entities->static_entity_1) text = text_box_2;
        else if (contact.self == entities->static_entity_2) text = text_box_3;
        else if (contact.self == entities->static_entity_3) text = text_box_4;
        const u32 text_size = sizeof(text_box_1)-1;

        auto audio = _game_ctx->audio;
        audio_voice_info info;
        const bool is_boop_playing = eng_audio_voice_get_info(audio->h_boop, info);
        if (!is_boop_playing) {
            audio->h_boop = eng_audio_sound_play("BOOP", audio_group_e_sfx, GAME_BOOP_VOLUME, false);
        };

        const bool did_open = game_dialog_open(text,text_size);
        assert(did_open);
    }

    IFB_INTERNAL void
    game_scene_dev_rose_collision_proc(
        const phys_cllsn_contact& contact) {

        assert(_game_ctx);
        assert(_scene_dev);

        auto player_rig = &_scene_dev->rig;

        // only connor matters, she can bump into the boxes too
        if (contact.other != player_rig->connor_id) return;

        // he has to be touching her, end means he just walked away
        if (contact.phase == phys_cllsn_phase_e_end) return;

        // the game callback reads e before the physics step, and leaves the press here if nothing used it
        if (!_game_ctx->is_action_pressed) return;

        // he has to be facing her, he only faces left or right, so it's which side of him she's on
        // the positions, not the normal, the normal is along z when he touches her from above or below
        cmpnt_position pos_rose;
        cmpnt_position pos_connor;
        if (!eng_cmpnt_lookup_position(contact.self,  pos_rose))   return;
        if (!eng_cmpnt_lookup_position(contact.other, pos_connor)) return;

        const f32  rose_dx         = pos_rose.x - pos_connor.x;
        const bool is_facing_left  = player_rig->connor_direction == game_character_direction_e_left;
        const bool is_facing_right = player_rig->connor_direction == game_character_direction_e_right;
        const bool is_facing_rose  = (is_facing_left && rose_dx <= 0.0f) || (is_facing_right && rose_dx >= 0.0f);

        // the press isn't used, so something else he's touching and facing can still take it
        if (!is_facing_rose) return;

        // used up, so nothing else connor is touching reacts to the same press
        _game_ctx->is_action_pressed = false;

        static constexpr cchar rose_text_cstr[] = "Rose: \"Hi Connor!\"";
        static constexpr u32   rose_text_size   = sizeof(rose_text_cstr)-1;
        const bool did_open = game_dialog_open(rose_text_cstr, rose_text_size);

        // she turns to connor, and stays put until the dialog closes
        if (did_open) {
            (void)game_npc_talk_to(contact.self, contact.other);
        }
    }

    // the map and its parallax, the player rig, the boxes, and rose
    // the scene is bound, so everything created here belongs to it, the map too
    IFB_INTERNAL void
    game_scene_dev_enter(
        game_context*   ctx,
        const hnd_scene h_scene) {

        assert(ctx);
        assert(h_scene != INVALID_HANDLE);
        assert(_scene_dev == NULL);

        auto dev = (game_scene_dev*)eng_scene_alloc(h_scene, sizeof(game_scene_dev));
        assert(dev != NULL);

        _scene_dev      = dev;
        dev->h_scene    = h_scene;
        ctx->player_rig = &dev->rig;
        ctx->map        = &dev->map;

        auto rig      = &dev->rig;
        auto map      = &dev->map;
        auto entities = &dev->entities;

        // the map goes first, the player rig spawns on it
        // DEV-MAP is assets/maps/dev-map.json, a map asset in the asset database
        // it uses CORE-SPRITES, so the atlas has to be loaded first
        map->eng_hnd = eng_map_load("DEV-MAP");
        assert(map->eng_hnd != INVALID_HANDLE);

        const bool did_set_plx = eng_parallax_set(
            "PARALLAX-FRONT",
            "PARALLAX-MID",
            "PARALLAX-BACK"
        );

        assert(did_set_plx);
        (void)did_set_plx;

        // connor starts in the first chunk, between the four boxes, unless a trigger says otherwise
        const game_scene_arrival spawn   = { 0, 5, 5 };
        const game_scene_arrival arrival = game_scene_get_arrival(spawn);

        game_player_rig_init(rig);
        const bool did_place_rig = game_player_rig_place(rig, map->eng_hnd, arrival.chunk_index, arrival.chunk_x, arrival.chunk_y);
        assert(did_place_rig);
        (void)did_place_rig;

        // the wall along the top of the first chunk goes to cliff town, straight up from where connor starts
        // he arrives on the west end of its road, just inside its door back here
        const game_scene_arrival cliff_town_arrival = { 0, 1, 12 };

        game_scene_trigger_def to_cliff_town;
        to_cliff_town.cstr_name   = "TRG-CLIFF-TOWN";
        to_cliff_town.h_map       = map->eng_hnd;
        to_cliff_town.chunk_index = 0;
        to_cliff_town.chunk_x     = 5;
        to_cliff_town.chunk_y     = 0;
        to_cliff_town.side        = game_scene_trigger_side_e_y_min;
        to_cliff_town.target      = game_scene_id_e_cliff_town;
        to_cliff_town.arrival     = &cliff_town_arrival;

        entities->trigger_cliff_town = game_scene_trigger_spawn(to_cliff_town);

        entities->static_entity_0 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-0","WOOD",map->eng_hnd,0,3,3);
        entities->static_entity_1 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-1","WOOD",map->eng_hnd,0,3,7);
        entities->static_entity_2 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-2","WOOD",map->eng_hnd,0,7,3);
        entities->static_entity_3 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-3","WOOD",map->eng_hnd,0,7,7);

        cmpnt_collider box_collider;
        box_collider.proc               = game_scene_dev_box_proc;
        box_collider.mask_category      = PHYS_CLLSN_MASK_ALL;
        box_collider.mask_collides_with = PHYS_CLLSN_MASK_ALL;
        box_collider.is_trigger         = false;


        bool did_add_colliders = true;
        did_add_colliders &= eng_entity_add_components (entities->static_entity_0, cmpnt_type_e_collider);
        did_add_colliders &= eng_cmpnt_update_collider (entities->static_entity_0, box_collider);
        did_add_colliders &= eng_entity_add_components (entities->static_entity_1, cmpnt_type_e_collider);
        did_add_colliders &= eng_cmpnt_update_collider (entities->static_entity_1, box_collider);
        did_add_colliders &= eng_entity_add_components (entities->static_entity_2, cmpnt_type_e_collider);
        did_add_colliders &= eng_cmpnt_update_collider (entities->static_entity_2, box_collider);
        did_add_colliders &= eng_entity_add_components (entities->static_entity_3, cmpnt_type_e_collider);
        did_add_colliders &= eng_cmpnt_update_collider (entities->static_entity_3, box_collider);
        assert(did_add_colliders);

        game_npc_def rose_def;
        rose_def.cstr_name           = "ROSE";
        rose_def.sprites.cstr_left   = "ROSE-LEFT";
        rose_def.sprites.cstr_right  = "ROSE-RIGHT";
        rose_def.sprites.cstr_walk_0 = "ROSE-WLK-0";
        rose_def.sprites.cstr_walk_1 = "ROSE-WLK-1";
        rose_def.sprites.cstr_walk_2 = "ROSE-WLK-2";
        rose_def.sprites.cstr_walk_3 = "ROSE-WLK-3";
        rose_def.sprites.cstr_walk_4 = "ROSE-WLK-4";
        rose_def.sprites.cstr_walk_5 = "ROSE-WLK-5";
        rose_def.sprites.cstr_walk_6 = "ROSE-WLK-6";
        rose_def.sprites.cstr_walk_7 = "ROSE-WLK-7";

        game_npc_placement rose_at;
        rose_at.h_map              = map->eng_hnd;
        rose_at.chunk_index        = 1;
        rose_at.chunk_x            = 2;
        rose_at.chunk_y            = 2;
        rose_at.proc               = NULL;
        rose_at.tile_roam_x        = 1;
        rose_at.tile_roam_y        = 1;
        rose_at.mask_category      = PHYS_CLLSN_MASK_ALL;
        rose_at.mask_collides_with = PHYS_CLLSN_MASK_ALL;
        rose_at.cllsn_proc         = game_scene_dev_rose_collision_proc;

        // the same whether the npc manager kept her from the last scene or not
        entities->rose = game_npc_spawn(rose_def, rose_at);

        assert(entities->trigger_cliff_town != ENTITY_ID_INVALID);
        assert(entities->rose               != ENTITY_ID_INVALID);
        assert(entities->static_entity_0 != ENTITY_ID_INVALID);
        assert(entities->static_entity_1 != ENTITY_ID_INVALID);
        assert(entities->static_entity_2 != ENTITY_ID_INVALID);
        assert(entities->static_entity_3 != ENTITY_ID_INVALID);
    }

    // the engine skips the scene's entities while it isn't updating or rendering,
    // so this only has to keep the game's own logic in step
    IFB_INTERNAL void
    game_scene_dev_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(_scene_dev);

        const hnd_scene h_scene = _scene_dev->h_scene;
        if (!eng_scene_is_updating(h_scene) &&
            !eng_scene_is_rendering(h_scene)) {
            return;
        }

        auto dev = _scene_dev;

        // connor stands still while he's being talked to, and while the scene fades
        dev->rig.is_input_locked = game_dialog_is_open() || game_scene_is_switching();

        game_player_rig_update_and_render (&dev->rig);
        game_player_rig_update_camera     (&dev->rig, ctx->camera_focus);
        const bool did_render = eng_map_render(dev->map.eng_hnd);
        game_npc_mngr_update_and_render       ();

        eng_entity_render(dev->entities.static_entity_0);
        eng_entity_render(dev->entities.static_entity_1);
        eng_entity_render(dev->entities.static_entity_2);
        eng_entity_render(dev->entities.static_entity_3);
    }

    // the engine already released everything the scene owned: its entities, its map, and its memory
    IFB_INTERNAL void
    game_scene_dev_exit(
        game_context* ctx) {

        assert(ctx);
        _scene_dev = NULL;
    }
};
