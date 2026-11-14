#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-scene-dev.cpp"
#include <cassert>

namespace ifb {

    // indexed by game_scene_id_e, none doesn't have any
    static const game_scene_procs GAME_SCENE_PROCS[game_scene_id_e_count] = {
        { NULL,                 NULL,                             NULL                },
        { game_scene_dev_enter, game_scene_dev_update_and_render, game_scene_dev_exit },
    };

    // the engine scene's name
    static const cchar* const GAME_SCENE_NAMES[game_scene_id_e_count] = {
        "",
        GAME_SCENE_DEV_NAME,
    };

    // the game context's, so the collision procs can request a switch without it
    static game_scene_state* _scene;

    inline void game_scene_begin_switch  (void);
    inline void game_scene_finish_switch (game_context* ctx);

    // nothing is running yet, so the first scene enters right away
    IFB_INTERNAL void
    game_scene_init(
        game_context*         ctx,
        const game_scene_id_e first) {

        assert(ctx);
        assert(first > game_scene_id_e_none && first < game_scene_id_e_count);

        _scene = &ctx->scene;
        _scene->active         = game_scene_id_e_none;
        _scene->next           = game_scene_id_e_none;
        _scene->phase          = game_scene_phase_e_running;
        _scene->h_active       = INVALID_HANDLE;
        _scene->h_next         = INVALID_HANDLE;
        _scene->npc_keep_count = 0;

        const bool did_request = game_scene_request(first);
        assert(did_request);
        (void)did_request;

        game_scene_begin_switch  ();
        game_scene_finish_switch (ctx);
    }

    // finishes a switch from last frame, updates the active scene, then starts a switch if one was requested
    // starting it after the update lets the old scene draw its last frame,
    // and its collision procs still run in this frame's physics step
    IFB_INTERNAL void
    game_scene_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(_scene);

        if (_scene->phase == game_scene_phase_e_switching) {
            game_scene_finish_switch(ctx);
        }

        assert(_scene->active.val > game_scene_id_e_none && _scene->active.val < game_scene_id_e_count);
        GAME_SCENE_PROCS[_scene->active.val].update_and_render(ctx);

        if (_scene->next != game_scene_id_e_none) {
            game_scene_begin_switch();
        }
    }

    // the scene to switch to at the end of the next game update
    // the npcs in npc_keep are remembered, so the next scene can place them again, the rest are forgotten
    // false while another switch is requested or underway
    IFB_INTERNAL bool
    game_scene_request(
        const game_scene_id_e id,
        const entity_id*      npc_keep,
        const u32             npc_keep_count) {

        assert(_scene);
        assert(id > game_scene_id_e_none && id < game_scene_id_e_count);
        assert(npc_keep != NULL || npc_keep_count == 0);

        if (game_scene_is_switching())                return(false);
        if (npc_keep_count > GAME_NPC_MNGR_CAPACITY) return(false);

        _scene->next           = id;
        _scene->npc_keep_count = npc_keep_count;
        for (
            u32 i_keep = 0;
                i_keep < npc_keep_count;
              ++i_keep) {

            _scene->npc_keep[i_keep] = npc_keep[i_keep];
        }

        return(true);
    }

    // a switch is requested or underway
    IFB_INTERNAL bool
    game_scene_is_switching(
        void) {

        assert(_scene);
        return(
            _scene->phase == game_scene_phase_e_switching ||
            _scene->next  != game_scene_id_e_none
        );
    }

    // the engine destroys the old scene at the start of the next frame, and runs the new one from then
    // the new scene is bound from here on, nothing should be created until it enters
    inline void
    game_scene_begin_switch(
        void) {

        assert(_scene);
        assert(_scene->phase == game_scene_phase_e_running);
        assert(_scene->next.val > game_scene_id_e_none && _scene->next.val < game_scene_id_e_count);

        const hnd_scene h_next = eng_scene_create(GAME_SCENE_NAMES[_scene->next.val]);
        assert(h_next != INVALID_HANDLE);

        const bool did_transition = eng_scene_transition(h_next, scene_flag_e_none);
        assert(did_transition);
        (void)did_transition;

        _scene->h_next = h_next;
        _scene->phase  = game_scene_phase_e_switching;
    }

    // the old scene's entities and memory are gone, so its names are free for the new scene to use
    inline void
    game_scene_finish_switch(
        game_context* ctx) {

        assert(ctx);
        assert(_scene);
        assert(_scene->phase  == game_scene_phase_e_switching);
        assert(_scene->h_next != INVALID_HANDLE);

        // what pointed into the old scene's memory
        ctx->player_rig = NULL;
        ctx->map        = NULL;

        if (_scene->active != game_scene_id_e_none) {
            GAME_SCENE_PROCS[_scene->active.val].exit(ctx);
        }

        // a dialog or a press from the old scene doesn't carry over
        game_npc_mngr_reset_all_except(_scene->npc_keep, _scene->npc_keep_count);
        game_dialog_reset();
        ctx->is_action_pressed = false;

        _scene->active         = _scene->next;
        _scene->h_active       = _scene->h_next;
        _scene->next           = game_scene_id_e_none;
        _scene->h_next         = INVALID_HANDLE;
        _scene->npc_keep_count = 0;
        _scene->phase          = game_scene_phase_e_running;

        // the transition bound it, but something else could have been bound since
        const bool did_bind = eng_scene_bind(_scene->h_active);
        assert(did_bind);
        (void)did_bind;

        GAME_SCENE_PROCS[_scene->active.val].enter(ctx, _scene->h_active);
    }
};
