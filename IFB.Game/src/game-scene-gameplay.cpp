#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-entity.cpp"
#include "game-map.cpp"
#include "game-player-rig.cpp"

namespace ifb {

    // the level: the map and its parallax, the player rig, and the boxes
    // all of it belongs to the gameplay scene, so destroying the scene cleans it up
    IFB_INTERNAL void
    game_scene_gameplay_init(
        game_context* ctx) {

        assert(ctx);

        // pushing binds the scene, so everything created after this belongs to it
        // it starts running on the first frame
        const hnd_scene h_scene = eng_scene_create(GAME_SCENE_GAMEPLAY_NAME);
        assert(h_scene != INVALID_HANDLE);

        const bool did_push = eng_scene_push(h_scene, scene_flag_e_none);
        assert(did_push);
        (void)did_push;

        // the level's state lives as long as the scene does
        auto rig      = (game_player_rig*)eng_scene_alloc(h_scene, sizeof(game_player_rig));
        auto map      =        (game_map*)eng_scene_alloc(h_scene, sizeof(game_map));
        auto entities =   (game_entities*)eng_scene_alloc(h_scene, sizeof(game_entities));
        assert(rig      != NULL);
        assert(map      != NULL);
        assert(entities != NULL);

        ctx->h_scene_gameplay = h_scene;
        ctx->player_rig       = rig;
        ctx->map              = map;
        ctx->entities         = entities;
        ctx->npc_mngr         = game_npc_mngr_create(h_scene);

        // the map goes first, the player rig spawns on it
        game_map_init(map);

        rig->starting_map_hnd = map->eng_hnd;
        rig->current_map_hnd  = map->eng_hnd;
        game_player_rig_init(rig);

        entities->static_entity_0 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-0","WOOD",map->eng_hnd,0,3,3);
        entities->static_entity_1 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-1","WOOD",map->eng_hnd,0,3,7);
        entities->static_entity_2 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-2","WOOD",map->eng_hnd,0,7,3);
        entities->static_entity_3 = game_entity_spawn_static_core_sprite("STATIC-SPRITE-3","WOOD",map->eng_hnd,0,7,7);


        game_npc_config rose_config;
        rose_config.cstr_name           = "ROSE";
        rose_config.sprites.cstr_left   = "ROSE-LEFT";
        rose_config.sprites.cstr_right  = "ROSE-RIGHT";
        rose_config.sprites.cstr_walk_0 = "ROSE-WLK-0";
        rose_config.sprites.cstr_walk_1 = "ROSE-WLK-1";
        rose_config.sprites.cstr_walk_2 = "ROSE-WLK-2";
        rose_config.sprites.cstr_walk_3 = "ROSE-WLK-3";
        rose_config.sprites.cstr_walk_4 = "ROSE-WLK-4";
        rose_config.sprites.cstr_walk_5 = "ROSE-WLK-5";
        rose_config.sprites.cstr_walk_6 = "ROSE-WLK-6";
        rose_config.sprites.cstr_walk_7 = "ROSE-WLK-7";
        rose_config.h_map               = map->eng_hnd;
        rose_config.chunk_index         = 1;
        rose_config.chunk_x             = 2;
        rose_config.chunk_y             = 2;
        rose_config.proc                = NULL;
        rose_config.tile_roam_x         = 1;
        rose_config.tile_roam_y         = 1;

        game_npc_spawn(rose_config);

        assert(entities->static_entity_0 != ENTITY_ID_INVALID);
        assert(entities->static_entity_1 != ENTITY_ID_INVALID);
        assert(entities->static_entity_2 != ENTITY_ID_INVALID);
        assert(entities->static_entity_3 != ENTITY_ID_INVALID);
    }

    // the engine skips the scene's entities while it isn't updating or rendering,
    // so this only has to keep the game's own logic in step
    IFB_INTERNAL void
    game_scene_gameplay_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(ctx->h_scene_gameplay != INVALID_HANDLE);

        if (!eng_scene_is_updating(ctx->h_scene_gameplay) &&
            !eng_scene_is_rendering(ctx->h_scene_gameplay)) {
            return;
        }

        auto entities = ctx->entities;

        game_player_rig_update_and_render (ctx->player_rig);
        game_player_rig_update_camera     (ctx->player_rig);
        game_map_update_and_render        (ctx->map);
        game_npc_mngr_update_and_render   ();

        eng_entity_render(entities->static_entity_0);
        eng_entity_render(entities->static_entity_1);
        eng_entity_render(entities->static_entity_2);
        eng_entity_render(entities->static_entity_3);
    }
};
