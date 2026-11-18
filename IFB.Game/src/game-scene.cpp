#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include "game-scene-trigger.cpp"
#include "game-scene-dev.cpp"
#include "game-scene-cliff-town.cpp"
#include "game-scene-physics.cpp"
#include <cassert>

namespace ifb {

    // indexed by game_scene_id_e, none doesn't have any
    static const game_scene_procs GAME_SCENE_PROCS[game_scene_id_e_count] = {
        { NULL,                        NULL,                                    NULL                       },
        { game_scene_dev_enter,        game_scene_dev_update_and_render,        game_scene_dev_exit        },
        { game_scene_cliff_town_enter, game_scene_cliff_town_update_and_render, game_scene_cliff_town_exit },
        { game_scene_physics_enter,    game_scene_physics_update_and_render,    game_scene_physics_exit    },
    };

    // the engine scene's name
    static const cchar* const GAME_SCENE_NAMES[game_scene_id_e_count] = {
        "",
        GAME_SCENE_DEV_NAME,
        GAME_SCENE_CLIFF_TOWN_NAME,
        GAME_SCENE_PHYSICS_NAME,
    };

    // the game context's, so the collision procs can request a switch without it
    static game_scene_state* _scene;

    inline void game_scene_begin_switch  (void);
    inline void game_scene_finish_switch (game_context* ctx);

    // nothing is running yet, so the first scene enters right away, and fades in from black
    IFB_GAME void
    game_scene_init(
        game_context*         ctx,
        const game_scene_id_e first) {

        assert(ctx);
        assert(first > game_scene_id_e_none && first < game_scene_id_e_count);

        _scene = &ctx->scene;
        _scene->active         = game_scene_id_e_none;
        _scene->next           = game_scene_id_e_none;
        _scene->phase          = game_scene_phase_e_running;
        _scene->fade_s         = 0.0f;
        _scene->h_active       = INVALID_HANDLE;
        _scene->h_next         = INVALID_HANDLE;
        _scene->has_arrival    = false;
        _scene->npc_keep_count = 0;

        const bool did_request = game_scene_request(first);
        assert(did_request);
        (void)did_request;

        game_scene_begin_switch  ();
        game_scene_finish_switch (ctx);
    
    }

    // finishes a switch from last frame, updates the active scene, then moves the fade along
    // the switch starts after the update, once the screen is black,
    // so the old scene draws its last frame, and its collision procs still run in this frame's physics step
    IFB_GAME void
    game_scene_update_and_render(
        game_context* ctx) {

        assert(ctx);
        assert(_scene);

        if (_scene->phase == game_scene_phase_e_switching) {
            game_scene_finish_switch(ctx);
        }

        assert(_scene->active.val > game_scene_id_e_none && _scene->active.val < game_scene_id_e_count);
        GAME_SCENE_PROCS[_scene->active.val].update_and_render(ctx);

        f32 dt_s = eng_system_get_delta_time_s();
        if (dt_s < 0.0f)                   dt_s = 0.0f;
        if (dt_s > GAME_SCENE_FADE_MAX_DT) dt_s = GAME_SCENE_FADE_MAX_DT;

        switch (_scene->phase.val) {

            case game_scene_phase_e_running: {
                if (_scene->next != game_scene_id_e_none) {
                    _scene->phase  = game_scene_phase_e_fade_out;
                    _scene->fade_s = 0.0f;
                }
            } break;

            case game_scene_phase_e_fade_out: {
                _scene->fade_s += dt_s;
                if (_scene->fade_s >= GAME_SCENE_FADE_OUT_S) {
                    game_scene_begin_switch();
                }
            } break;

            case game_scene_phase_e_fade_in: {
                _scene->fade_s += dt_s;
                if (_scene->fade_s >= GAME_SCENE_FADE_HOLD_S + GAME_SCENE_FADE_IN_S) {
                    _scene->phase  = game_scene_phase_e_running;
                    _scene->fade_s = 0.0f;
                }
            } break;

            default: break;
        }
    }

    // black over the whole screen, as dark as the fade is
    // it goes last, so it covers the scene and the dialog, only the gui is drawn over it
    IFB_GAME void
    game_scene_render_fade(
        void) {

        assert(_scene);

        f32 alpha = 0.0f;
        switch (_scene->phase.val) {
            case game_scene_phase_e_fade_out:  alpha = _scene->fade_s / GAME_SCENE_FADE_OUT_S;                             break;
            case game_scene_phase_e_switching: alpha = 1.0f;                                                               break;
            case game_scene_phase_e_fade_in:   alpha = 1.0f - ((_scene->fade_s - GAME_SCENE_FADE_HOLD_S) / GAME_SCENE_FADE_IN_S); break;
            default:                                                                                                       break;
        }
        if (alpha <= 0.0f) return;
        if (alpha >  1.0f) alpha = 1.0f;

        // eased, so it starts and ends gently
        alpha = alpha * alpha * (3.0f - (2.0f * alpha));

        u32 screen_width  = 0;
        u32 screen_height = 0;
        eng_draw_get_screen_size(screen_width, screen_height);

        rect screen;
        screen.x      = 0.0f;
        screen.y      = 0.0f;
        screen.width  = (f32)screen_width;
        screen.height = (f32)screen_height;

        color_rgba_f32 black;
        black.r = 0.0f;
        black.g = 0.0f;
        black.b = 0.0f;
        black.a = alpha;

        (void)eng_draw_rect_screen(screen, black);
    }

    // the scene to switch to at the end of the next game update
    // connor arrives at arrival, or the scene's own spawn if it's NULL
    // the npcs in npc_keep are remembered, so the next scene can place them again, the rest are forgotten
    // false while another switch is requested or underway
    IFB_GAME bool
    game_scene_request(
        const game_scene_id_e     id,
        const game_scene_arrival* arrival,
        const entity_id*          npc_keep,
        const u32                 npc_keep_count) {

        assert(_scene);
        assert(id > game_scene_id_e_none && id < game_scene_id_e_count);
        assert(npc_keep != NULL || npc_keep_count == 0);

        if (game_scene_is_switching())                return(false);
        if (npc_keep_count > GAME_NPC_MNGR_CAPACITY) return(false);

        _scene->next        = id;
        _scene->has_arrival = (arrival != NULL);
        if (arrival) {
            _scene->arrival = *arrival;
        }

        _scene->npc_keep_count = npc_keep_count;
        for (
            u32 i_keep = 0;
                i_keep < npc_keep_count;
              ++i_keep) {

            _scene->npc_keep[i_keep] = npc_keep[i_keep];
        }

        return(true);
    }

    // a switch is requested or underway, from the request until the new scene has faded in
    IFB_GAME bool
    game_scene_is_switching(
        void) {

        assert(_scene);
        return(
            _scene->phase != game_scene_phase_e_running ||
            _scene->next  != game_scene_id_e_none
        );
    }

    // where connor goes in the scene that's entering, the requested arrival, or the scene's own spawn
    // only meaningful in a scene's enter proc
    IFB_GAME game_scene_arrival
    game_scene_get_arrival(
        const game_scene_arrival& spawn) {

        assert(_scene);
        return(_scene->has_arrival ? _scene->arrival : spawn);
    }

    // the engine destroys the old scene at the start of the next frame, and runs the new one from then
    // the new scene is bound from here on, nothing should be created until it enters
    // the screen is black by now, or nothing was running yet
    inline void
    game_scene_begin_switch(
        void) {

        assert(_scene);
        assert(_scene->phase == game_scene_phase_e_fade_out || _scene->active == game_scene_id_e_none);
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
        ctx->phys_test  = NULL;

        if (_scene->active != game_scene_id_e_none) {
            GAME_SCENE_PROCS[_scene->active.val].exit(ctx);
        }

        // a dialog, a press, or a trigger from the old scene doesn't carry over
        game_npc_mngr_reset_all_except(_scene->npc_keep, _scene->npc_keep_count);
        game_dialog_reset();
        game_scene_trigger_reset();
        ctx->is_action_pressed = false;

        _scene->active         = _scene->next;
        _scene->h_active       = _scene->h_next;
        _scene->next           = game_scene_id_e_none;
        _scene->h_next         = INVALID_HANDLE;
        _scene->npc_keep_count = 0;
        _scene->phase          = game_scene_phase_e_fade_in;
        _scene->fade_s         = 0.0f;

        // the transition bound it, but something else could have been bound since
        const bool did_bind = eng_scene_bind(_scene->h_active);
        assert(did_bind);
        (void)did_bind;

        GAME_SCENE_PROCS[_scene->active.val].enter(ctx, _scene->h_active);

        // used up by enter
        _scene->has_arrival = false;
    }
};
