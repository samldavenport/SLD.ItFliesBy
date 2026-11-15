#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-player-rig.cpp"
#include <cassert>

namespace ifb {

    // where connor arrives in cliff town, on the road across the middle of the map
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_SPAWN_CHUNK = 0;
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_SPAWN_X     = 12;
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_SPAWN_Y     = 12;

    // the cliff town scene's state, in its scene memory, NULL while it isn't the active scene
    struct game_scene_cliff_town {
        hnd_scene       h_scene;
        game_player_rig rig;
        game_map        map;
    } static * _scene_cliff_town;

    // the map and its parallax, and the player rig
    // the scene is bound, so everything created here belongs to it, the map too
    IFB_INTERNAL void
    game_scene_cliff_town_enter(
        game_context*   ctx,
        const hnd_scene h_scene) {

        assert(ctx);
        assert(h_scene != INVALID_HANDLE);
        assert(_scene_cliff_town == NULL);

        auto town = (game_scene_cliff_town*)eng_scene_alloc(h_scene, sizeof(game_scene_cliff_town));
        assert(town != NULL);

        _scene_cliff_town = town;
        town->h_scene     = h_scene;
        ctx->player_rig   = &town->rig;
        ctx->map          = &town->map;

        auto rig = &town->rig;
        auto map = &town->map;

        // the map goes first, the player rig is placed on it
        // CLIFF-TOWN is assets/maps/cliff-town.json, a map asset in the asset database
        // it uses CORE-SPRITES, so the atlas has to be loaded first
        map->eng_hnd = eng_map_load("CLIFF-TOWN");
        assert(map->eng_hnd != INVALID_HANDLE);

        const bool did_set_plx = eng_parallax_set(
            "PARALLAX-CLIFFS-FRONT",
            "PARALLAX-CLIFFS-MID",
            "PARALLAX-CLIFFS-BACK"
        );
        assert(did_set_plx);
        (void)did_set_plx;

        game_player_rig_init(rig);
        const bool did_place_rig = game_player_rig_place(
            rig,
            map->eng_hnd,
            GAME_SCENE_CLIFF_TOWN_SPAWN_CHUNK,
            GAME_SCENE_CLIFF_TOWN_SPAWN_X,
            GAME_SCENE_CLIFF_TOWN_SPAWN_Y
        );
        assert(did_place_rig);
        (void)did_place_rig;
    }

    // the engine skips the scene's entities while it isn't updating or rendering,
    // so this only has to keep the game's own logic in step
    IFB_INTERNAL void
    game_scene_cliff_town_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(_scene_cliff_town);

        const hnd_scene h_scene = _scene_cliff_town->h_scene;
        if (!eng_scene_is_updating(h_scene) &&
            !eng_scene_is_rendering(h_scene)) {
            return;
        }

        auto town = _scene_cliff_town;

        // connor stands still while a dialog is open
        town->rig.is_input_locked = game_dialog_is_open();

        game_player_rig_update_and_render (&town->rig);
        game_player_rig_update_camera     (&town->rig, ctx->camera_focus);
        (void)eng_map_render              (town->map.eng_hnd);

        // npcs kept from the last scene aren't placed until this scene places them
        game_npc_mngr_update_and_render();
    }

    // the engine already released everything the scene owned: its entities, its map, and its memory
    IFB_INTERNAL void
    game_scene_cliff_town_exit(
        game_context* ctx) {

        assert(ctx);
        _scene_cliff_town = NULL;
    }
};
