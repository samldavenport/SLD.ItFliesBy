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

    // the trigger back to the dev map, the map's edge at the west end of the road
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_TO_DEV_CHUNK = 0;
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_TO_DEV_X     = 0;
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_TO_DEV_Y     = 12;

    // where connor arrives in the dev map, just inside its wall trigger to cliff town on the top of tile 5,0
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_DEV_ARRIVAL_CHUNK = 0;
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_DEV_ARRIVAL_X     = 5;
    static constexpr u32 GAME_SCENE_CLIFF_TOWN_DEV_ARRIVAL_Y     = 1;

    // the cliff town scene's state, in its scene memory, NULL while it isn't the active scene
    struct game_scene_cliff_town {
        hnd_scene       h_scene;
        game_player_rig rig;
        game_map        map;
        entity_id       trigger_dev;
    } static * _scene_cliff_town;

    // the map and its parallax, the player rig, and the trigger back to the dev map
    // the scene is bound, so everything created here belongs to it, the map too
    IFB_GAME void
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

        // wherever the trigger he came through says, or the road
        const game_scene_arrival spawn = {
            GAME_SCENE_CLIFF_TOWN_SPAWN_CHUNK,
            GAME_SCENE_CLIFF_TOWN_SPAWN_X,
            GAME_SCENE_CLIFF_TOWN_SPAWN_Y
        };
        const game_scene_arrival arrival = game_scene_get_arrival(spawn);

        game_player_rig_init(rig);
        const bool did_place_rig = game_player_rig_place(
            rig,
            map->eng_hnd,
            arrival.chunk_index,
            arrival.chunk_x,
            arrival.chunk_y
        );
        assert(did_place_rig);
        (void)did_place_rig;

        // walking off the west end of the road goes back to the dev map,
        // connor arrives just inside its cliff town trigger, not on it
        const game_scene_arrival dev_arrival = {
            GAME_SCENE_CLIFF_TOWN_DEV_ARRIVAL_CHUNK,
            GAME_SCENE_CLIFF_TOWN_DEV_ARRIVAL_X,
            GAME_SCENE_CLIFF_TOWN_DEV_ARRIVAL_Y
        };

        game_scene_trigger_def to_dev;
        to_dev.cstr_name   = "TRG-DEV";
        to_dev.h_map       = map->eng_hnd;
        to_dev.chunk_index = GAME_SCENE_CLIFF_TOWN_TO_DEV_CHUNK;
        to_dev.chunk_x     = GAME_SCENE_CLIFF_TOWN_TO_DEV_X;
        to_dev.chunk_y     = GAME_SCENE_CLIFF_TOWN_TO_DEV_Y;
        to_dev.side        = game_scene_trigger_side_e_x_min;
        to_dev.target      = game_scene_id_e_dev;
        to_dev.arrival     = &dev_arrival;

        town->trigger_dev = game_scene_trigger_spawn(to_dev);
        assert(town->trigger_dev != ENTITY_ID_INVALID);
    }

    // the engine skips the scene's entities while it isn't updating or rendering,
    // so this only has to keep the game's own logic in step
    IFB_GAME void
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

        // connor stands still while a dialog is open, and while the scene fades
        town->rig.is_input_locked = game_dialog_is_open() || game_scene_is_switching();

        game_player_rig_update_and_render (&town->rig);
        game_player_rig_update_camera     (&town->rig, ctx->camera_focus);
        (void)eng_map_render              (town->map.eng_hnd);

        // npcs kept from the last scene aren't placed until this scene places them
        game_npc_mngr_update_and_render();
    }

    // the engine already released everything the scene owned: its entities, its map, and its memory
    IFB_GAME void
    game_scene_cliff_town_exit(
        game_context* ctx) {

        assert(ctx);
        _scene_cliff_town = NULL;
    }
};
