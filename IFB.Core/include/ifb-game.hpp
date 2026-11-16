#ifndef IFB_GAME_HPP
#define IFB_GAME_HPP

#include "ifb.hpp"

#define IFB_GAME static

namespace ifb {



    IFB_U32(game_character_state_u32);
    enum game_character_state_e {
        game_character_state_e_idle_left    = 0,
        game_character_state_e_idle_right   = 1,
        game_character_state_e_walk_left_1  = 2,
        game_character_state_e_walk_left_2  = 3,
        game_character_state_e_walk_left_3  = 4,
        game_character_state_e_walk_left_4  = 5,
        game_character_state_e_walk_right_1 = 6,
        game_character_state_e_walk_right_2 = 7,
        game_character_state_e_walk_right_3 = 8,
        game_character_state_e_walk_right_4 = 9
    };
      
    IFB_U32(game_character_direction_u32);
    enum game_character_direction_e {
        game_character_direction_e_left  = 0,
        game_character_direction_e_right = 1,
    };

    IFB_U32(game_jig_state_u32);
    enum game_jig_state_e {
        game_jig_state_e_left_0  = 0,
        game_jig_state_e_left_1  = 1,
        game_jig_state_e_left_2  = 2,
        game_jig_state_e_left_3  = 3,
        game_jig_state_e_right_0 = 4,
        game_jig_state_e_right_1 = 5,
        game_jig_state_e_right_2 = 6,
        game_jig_state_e_right_3 = 7
    };

    struct game_player_rig {
        entity_id                    connor_id;
        entity_id                    jig_id;
        entity_id                    jig_anchor_id;
        hnd_map                      current_map_hnd;   // INVALID_HANDLE until game_player_rig_place
        f32                          jig_offset_x;
        f32                          jig_bob_time_s;
        f32                          connor_prev_x;
        f32                          walk_timer;
        f32                          jig_fly_progress;
        f32                          jig_fly_blend;
        game_character_state_u32     connor_state;
        game_character_direction_u32 connor_direction;
        game_jig_state_u32           jig_state;
        game_character_direction_u32 jig_direction;
        bool                         walk_cycle_change;
        bool                         is_input_locked;   // the movement keys are ignored, like while a dialog is open
    };

    struct game_entities {
        entity_id static_entity_0;
        entity_id static_entity_1;
        entity_id static_entity_2;
        entity_id static_entity_3;
        entity_id rose;
        entity_id trigger_cliff_town;
    };

    struct game_map {
        hnd_map eng_hnd;
    };

    struct game_sprites {
        hnd_sprite_atlas h_atlas;
        u32              i_sprite_connor;
        u32              i_sprite_jig;
    };

    static constexpr f32 GAME_BOOP_VOLUME  = 0.25f;
    static constexpr f32 GAME_MUSIC_VOLUME = 0.3f;

    struct game_audio {
        hnd_audio_voice h_music; // INVALID_HANDLE if it couldn't start, like without the sound bank
        hnd_audio_voice h_boop;  // stale once the boop finishes
    };

    static constexpr const cchar* GAME_SCENE_DEV_NAME        = "DEV";
    static constexpr const cchar* GAME_SCENE_CLIFF_TOWN_NAME = "CLIFF-TOWN";
    static constexpr const cchar* GAME_SCENE_PHYSICS_NAME    = "PHYSICS";

    struct game_npc_mngr;

    //--------------------------------------------------------------------
    // DIALOG
    //--------------------------------------------------------------------
    // a box along the bottom of the screen that types its text out a character at a time
    // the text is wrapped when it opens, then shown a page of lines at a time
    struct game_dialog;

    static constexpr u32 GAME_DIALOG_LINE_CAPACITY    = 32;     // lines past this are cut off
    static constexpr u32 GAME_DIALOG_LINES_PER_PAGE   = 2;
    static constexpr f32 GAME_DIALOG_CHARS_PER_SECOND = 40.0f;
    static constexpr f32 GAME_DIALOG_MARGIN           = 16.0f;  // screen pixels around the box
    static constexpr f32 GAME_DIALOG_BORDER           = 4.0f;
    static constexpr f32 GAME_DIALOG_PADDING          = 16.0f;  // inside the border, around the text
    static constexpr f32 GAME_DIALOG_LINE_SPACING     = 8.0f;   // on top of the font's line height
    static constexpr f32 GAME_DIALOG_ARROW_SIZE       = 8.0f;   // the "more" marker in the bottom right corner
    static constexpr f32 GAME_DIALOG_ARROW_BLINK_S    = 0.4f;   // how long the marker is shown, then hidden
    static constexpr u32 GAME_DIALOG_COLOR_BORDER     = 0x303040FF;
    static constexpr u32 GAME_DIALOG_COLOR_FILL       = 0xF8F8F8FF;
    static constexpr u32 GAME_DIALOG_COLOR_TEXT       = 0x303040FF;

    IFB_U32(game_dialog_state_u32);
    enum game_dialog_state_e {
        game_dialog_state_e_closed = 0,
        game_dialog_state_e_typing = 1, // the page is still being revealed
        game_dialog_state_e_paused = 2  // the page is all shown, waiting for the player
    };


    //--------------------------------------------------------------------
    // NPCS
    //--------------------------------------------------------------------
    // npcs are built like connor, without the input, and with their own sprites
    // the manager updates and renders every one placed in the current scene, and runs its proc, if it has one
    //
    // the manager belongs to the game context, and outlives the scenes
    // an npc's entity belongs to the scene it was placed in, and is gone with it
    // so the manager keeps who the npc is (game_npc_def) across scenes,
    // and each scene places it again with where it stands and how it behaves there (game_npc_placement)
    // its entity id is the hash of its name, so it's the same in every scene
    static constexpr u32 GAME_NPC_MNGR_CAPACITY = 64;

    // called once a frame for the npc, NULL if it doesn't have one
    // it can despawn its own npc, or any other
    using game_npc_proc = void (*) (const entity_id id);

    // the names aren't copied, they have to outlive the npc
    struct game_npc_sprites {
        const cchar* cstr_left;
        const cchar* cstr_right;
        const cchar* cstr_walk_0;
        const cchar* cstr_walk_1;
        const cchar* cstr_walk_2;
        const cchar* cstr_walk_3;
        const cchar* cstr_walk_4;
        const cchar* cstr_walk_5;
        const cchar* cstr_walk_6;
        const cchar* cstr_walk_7;
    };

    // who the npc is, kept across scenes
    // the name isn't copied, it has to outlive the npc
    struct game_npc_def {
        const cchar*         cstr_name;
        game_npc_sprites     sprites;
    };

    // where the npc stands in the scene it's placed in, and how it behaves there
    struct game_npc_placement {
        game_npc_proc        proc;
        hnd_map              h_map;
        u32                  chunk_index;
        u32                  chunk_x;
        u32                  chunk_y;
        u32                  tile_roam_x; // the distance the character can roam from their spawn tile
        u32                  tile_roam_y;
        u32                  mask_category;
        u32                  mask_collides_with;
        fptr_phys_cllsn_proc cllsn_proc;  // what it does when connor talks to it depends on where it is
    };

    //--------------------------------------------------------------------
    // SCENES
    //--------------------------------------------------------------------
    // the game runs one scene at a time, each one is an engine scene that owns its entities and memory
    // a switch is requested, and applied over two frames:
    //
    //   the frame it's applied: the old scene updates one last time, then the engine transition is requested
    //   the next frame:         the engine has released the old scene, so the old scene exits,
    //                           the npcs that aren't kept are forgotten, and the new scene enters and updates
    //
    // the new scene enters a frame later so it can create entities with the same names as the old scene's,
    // like connor, which the engine only destroys at the start of the frame

    IFB_U32(game_scene_id_u32);
    enum game_scene_id_e {
        game_scene_id_e_none       = 0,
        game_scene_id_e_dev        = 1,
        game_scene_id_e_cliff_town = 2,
        game_scene_id_e_physics    = 3, // the physics stress test, from game > testing > physics
        game_scene_id_e_count      = 4
    };

    // a switch fades the screen to black, switches while it's black, then fades the new scene in
    //
    //   running   -> fade_out   a switch was requested, the old scene keeps running under the fade
    //   fade_out  -> switching  the screen is black, the engine transition is requested
    //   switching -> fade_in    the old scene is released and the new one enters
    //   fade_in   -> running    the new scene runs under the fade, after a beat of black
    //
    // connor's movement keys are locked, and presses are dropped, from the request until the fade in is done
    IFB_U32(game_scene_phase_u32);
    enum game_scene_phase_e {
        game_scene_phase_e_running   = 0, // the active scene updates, and a switch can be requested
        game_scene_phase_e_fade_out  = 1, // the old scene fades to black
        game_scene_phase_e_switching = 2, // the old scene is being released, the next one enters at the start of the next update
        game_scene_phase_e_fade_in   = 3  // the new scene fades in from black
    };

    static constexpr f32 GAME_SCENE_FADE_OUT_S  = 0.3f;
    static constexpr f32 GAME_SCENE_FADE_HOLD_S = 0.1f;        // black, before the fade in, it covers the new scene's first frames too
    static constexpr f32 GAME_SCENE_FADE_IN_S   = 0.3f;
    static constexpr f32 GAME_SCENE_FADE_MAX_DT = 1.0f / 30.0f; // a long frame, like the one that loads the map, doesn't skip the fade

    struct game_context;

    struct game_scene_procs {
        // the scene is bound, load its map, allocate its state with eng_scene_alloc, and spawn or place its entities and npcs
        void (*enter)             (game_context* ctx, const hnd_scene h_scene);
        void (*update_and_render) (game_context* ctx);
        // runs after the engine released the scene, so its entities, maps, and eng_scene_alloc memory are already gone
        // it clears what pointed into them, and releases anything the engine scene doesn't own
        void (*exit)              (game_context* ctx);
    };

    // the tile connor is placed on when a scene enters, like the door he came through
    struct game_scene_arrival {
        u32 chunk_index;
        u32 chunk_x;
        u32 chunk_y;
    };

    struct game_scene_state {
        game_scene_id_u32    active;
        game_scene_id_u32    next;            // none when no switch is requested
        game_scene_phase_u32 phase;
        f32                  fade_s;          // how long the fade out or fade in has run
        hnd_scene            h_active;
        hnd_scene            h_next;          // created and transitioned to while switching
        game_scene_arrival   arrival;         // where connor goes in the next scene, if has_arrival
        bool                 has_arrival;     // false, and the next scene uses its own spawn
        u32                  npc_keep_count;  // the npcs that are remembered through the switch
        entity_id            npc_keep[GAME_NPC_MNGR_CAPACITY];
    };

    //--------------------------------------------------------------------
    // SCENE TRIGGERS
    //--------------------------------------------------------------------
    // a tile, or one edge of a tile, that switches scenes when connor touches it, like a door
    // each one is an invisible trigger entity in the scene that spawned it, and is gone with it
    // a map can have as many as the table holds, each going to its own scene and arrival
    //
    // an arrival is never on a trigger, or connor walks straight back out
    static constexpr u32 GAME_SCENE_TRIGGER_CAPACITY = 32;

    // what part of the tile the trigger covers
    // an edge is a thin slab along that side of the tile, so connor touches it walking into a wall there,
    // into the edge of the map, or through a gap in a wall
    // in the map file, a row wall at (row, col) is the y_min side of tile (col, row),
    // and a column wall at (row, col) is the x_min side of tile (col, row)
    // physics only pairs entities in the same chunk, so an edge between two chunks
    // only triggers from the tile's side
    IFB_U32(game_scene_trigger_side_u32);
    enum game_scene_trigger_side_e {
        game_scene_trigger_side_e_tile  = 0, // the whole tile, connor walks onto it
        game_scene_trigger_side_e_x_min = 1,
        game_scene_trigger_side_e_x_max = 2,
        game_scene_trigger_side_e_y_min = 3,
        game_scene_trigger_side_e_y_max = 4
    };

    // the name is the entity's tag, so it has to be unique in the scene, and fit in TAG_SIZE - 1 characters
    struct game_scene_trigger_def {
        const cchar*              cstr_name;
        hnd_map                   h_map;
        u32                       chunk_index;
        u32                       chunk_x;
        u32                       chunk_y;
        game_scene_trigger_side_e side;
        game_scene_id_e           target;
        const game_scene_arrival* arrival;  // NULL for the target scene's own spawn
    };

    //--------------------------------------------------------------------
    // PHYSICS TEST
    //--------------------------------------------------------------------
    // a walled arena full of quads that fly off in random directions and ricochet off of each other and the walls
    // the gui's physics test window changes the settings, and shows the readouts
    // the quads are respawned with the new count, size, and speeds when the window asks,
    // the restitution and mass apply to the quads right away

    static constexpr u32 GAME_PHYS_TEST_QUAD_CAPACITY = 1000;  // under the engine's entity capacity, with room for the walls
    static constexpr u32 GAME_PHYS_TEST_ARENA_COLS    = 32;    // tiles, the outer ring is the walls
    static constexpr u32 GAME_PHYS_TEST_ARENA_ROWS    = 20;

    struct game_phys_test_settings {
        u32  quad_count;
        f32  quad_size;      // world units, a tile is 0.2
        f32  speed_min;      // world units per second
        f32  speed_max;
        f32  restitution;    // 1 bounces forever, 0 stops dead
        f32  mass_min;       // each quad gets a mass between these
        f32  mass_max;
        bool is_respawn_requested;
    };

    // filled in by the scene every frame
    struct game_phys_test_readout {
        u32 quad_count;         // the quads that actually spawned
        u32 quad_count_moving;  // the ones with any speed left
        f32 speed_avg;
        f32 speed_max;
        f32 kinetic_energy;     // sum of half m v squared, it stays flat with perfect bounces
        f32 kinetic_energy_start;
        u32 escaped_count;      // quads that got outside of the walls
    };

    struct game_phys_test {
        game_phys_test_settings settings;
        game_phys_test_readout  readout;
    };

    // the global arena holds what lasts the whole game
    // the player rig and map are the active scene's, NULL when it doesn't have one
    // phys_test is the physics test scene's, NULL when it isn't the active scene
    struct game_context {
        hnd_arena        global_arena;
        game_scene_state scene;
        game_player_rig* player_rig;
        game_map*        map;
        game_phys_test*  phys_test;
        game_npc_mngr*   npc_mngr;
        game_sprites*    sprites;
        game_audio*      audio;
        const font*      fnt;   // the ui font, NULL if it didn't load
        game_dialog*     dialog;
        vec3             camera_focus;  // where the engine camera is framed, it outlives the scenes like the camera does
        bool             talk_was_down;
        bool             is_action_pressed; // e was pressed this pass, and nothing has used it yet
    } static * _game_ctx;

    IFB_GAME game_context* game_context_create_and_init      (void);
    IFB_GAME void          game_context_update_and_render    (game_context* ctx);

    IFB_GAME game_dialog*  game_dialog_create     (const hnd_arena h_arena, const font* fnt);
    IFB_GAME void          game_dialog_reset      (void);
    IFB_GAME bool          game_dialog_open       (const cchar* text, const u32 text_len);
    IFB_GAME bool          game_dialog_is_open    (void);
    IFB_GAME void          game_dialog_advance    (void);
    IFB_GAME void          game_dialog_update     (game_context* ctx);
    IFB_GAME void          game_dialog_render     (void);
    IFB_GAME void          game_dialog_get_layout (rect& box, rect& text_area);

    // scenes
    IFB_GAME void          game_scene_init              (game_context* ctx, const game_scene_id_e first);
    IFB_GAME void          game_scene_update_and_render (game_context* ctx);
    IFB_GAME bool          game_scene_request           (const game_scene_id_e id, const game_scene_arrival* arrival = NULL, const entity_id* npc_keep = NULL, const u32 npc_keep_count = 0);
    IFB_GAME bool          game_scene_is_switching      (void);
    IFB_GAME void          game_scene_render_fade       (void);
    IFB_GAME game_scene_arrival game_scene_get_arrival  (const game_scene_arrival& spawn);

    // scene triggers
    IFB_GAME void          game_scene_trigger_reset     (void);
    IFB_GAME entity_id     game_scene_trigger_spawn     (const game_scene_trigger_def& def);

    // the dev scene: the dev map, the player rig, the boxes, and rose
    IFB_GAME void          game_scene_dev_enter              (game_context* ctx, const hnd_scene h_scene);
    IFB_GAME void          game_scene_dev_update_and_render  (game_context* ctx);
    IFB_GAME void          game_scene_dev_exit               (game_context* ctx);

    // cliff town: the cliff town map, the player rig, and the trigger back to the dev map
    IFB_GAME void          game_scene_cliff_town_enter           (game_context* ctx, const hnd_scene h_scene);
    IFB_GAME void          game_scene_cliff_town_update_and_render (game_context* ctx);
    IFB_GAME void          game_scene_cliff_town_exit              (game_context* ctx);

    // the physics test: a walled arena of quads ricocheting off of each other
    IFB_GAME void          game_scene_physics_enter              (game_context* ctx, const hnd_scene h_scene);
    IFB_GAME void          game_scene_physics_update_and_render  (game_context* ctx);
    IFB_GAME void          game_scene_physics_exit               (game_context* ctx);

    IFB_GAME void          game_player_rig_validate          (game_player_rig* player_rig);
    IFB_GAME void          game_player_rig_init              (game_player_rig* player_rig);
    IFB_GAME bool          game_player_rig_place             (game_player_rig* player_rig, const hnd_map h_map, const u32 chunk_index, const u32 chunk_x, const u32 chunk_y);
    IFB_GAME void          game_player_rig_update_and_render (game_player_rig* player_rig);
    IFB_GAME void          game_player_rig_update_camera     (game_player_rig* player_rig, vec3& camera_focus);
    
    IFB_GAME entity_id
    game_entity_spawn_static_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  atlas_name_cstr, 
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y
    );

    IFB_GAME entity_id
    game_entity_spawn_static_core_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y
    );

    IFB_GAME entity_id
    game_entity_spawn_core_npc_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y
    );

    // the collision rect connor and the npcs share, sized to their quad
    IFB_GAME cmpnt_bounds
    game_entity_character_bounds(
        const f32 quad_width
    );


    IFB_GAME void game_map_update_and_render        (game_map* map);


    // the manager lives on the game context's arena
    IFB_GAME game_npc_mngr* game_npc_mngr_create              (const hnd_arena h_arena);
    IFB_GAME void           game_npc_mngr_update_and_render   (void);
    IFB_GAME void           game_npc_mngr_reset_all_except    (const entity_id* keep, const u32 keep_count);
    IFB_GAME entity_id      game_npc_spawn                    (const game_npc_def& def, const game_npc_placement& at);
    IFB_GAME bool           game_npc_despawn                  (const entity_id id);
    IFB_GAME bool           game_npc_is_placed                (const entity_id id);
    IFB_GAME bool           game_npc_talk_to                  (const entity_id id, const entity_id other_id);
};

#endif //IFB_GAME_HPP
