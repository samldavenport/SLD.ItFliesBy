#ifndef IFB_GAME_HPP
#define IFB_GAME_HPP

#include "ifb.hpp"

namespace ifb {

    IFB_U32(game_connor_state_u32);
    enum game_connor_state_e {
        game_connor_state_e_idle_left    = 0,
        game_connor_state_e_idle_right   = 1,
        game_connor_state_e_walk_left_1  = 2,
        game_connor_state_e_walk_left_2  = 3,
        game_connor_state_e_walk_left_3  = 4,
        game_connor_state_e_walk_left_4  = 5,
        game_connor_state_e_walk_right_1 = 6,
        game_connor_state_e_walk_right_2 = 7,
        game_connor_state_e_walk_right_3 = 8,
        game_connor_state_e_walk_right_4 = 9
    };
      
    IFB_U32(game_connor_direction_u32);
    enum game_connor_direction_e {
        game_connor_direction_e_left  = 0,
        game_connor_direction_e_right = 1,
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

    IFB_U32(game_jig_direction_u32);
    enum game_jig_direction_e {
        game_jig_direction_e_left  = 0,
        game_jig_direction_e_right = 1
    }; 

    struct game_player_rig {
        entity_id                 connor_id;
        entity_id                 jig_id;
        entity_id                 jig_anchor_id;
        hnd_map                   starting_map_hnd;
        hnd_map                   current_map_hnd;
        vec3                      camera_focus;
        f32                       jig_offset_x;
        f32                       jig_bob_time_s;
        f32                       connor_prev_x;
        f32                       walk_timer;
        game_connor_state_u32     connor_state;
        game_connor_direction_u32 connor_direction;
        f32                       jig_fly_progress;
        f32                       jig_fly_blend;
        game_jig_state_u32        jig_state;
        game_jig_direction_u32    jig_direction;
        bool                      walk_cycle_change;
        bool                      is_input_locked;   // the movement keys are ignored, like while a dialog is open
    };

    struct game_entities {
        entity_id static_entity_0;
        entity_id static_entity_1;
        entity_id static_entity_2;
        entity_id static_entity_3;
    };

    struct game_map {
        hnd_map eng_hnd;
    };

    struct game_sprites {
        hnd_sprite_atlas h_atlas;
        u32              i_sprite_connor;
        u32              i_sprite_jig;
    };

    // collision events keep coming every physics step while connor touches a box,
    // so a bump is a contact after GAME_BOOP_CONTACT_GAP_S without one
    static constexpr f64 GAME_BOOP_CONTACT_GAP_S = 0.1;
    static constexpr f32 GAME_BOOP_VOLUME        = 0.25f;
    static constexpr f32 GAME_MUSIC_VOLUME       = 0.3f;

    struct game_audio {
        hnd_audio_voice h_music;            // INVALID_HANDLE if it couldn't start, like without the sound bank
        hnd_audio_voice h_boop;             // stale once the boop finishes
        f64             time_s;             // since the game started
        f64             box_contact_time_s; // the last collision event between connor and a box
    };

    static constexpr const cchar* GAME_SCENE_GAMEPLAY_NAME = "GAMEPLAY";

    //--------------------------------------------------------------------
    // DIALOG
    //--------------------------------------------------------------------
    // a box along the bottom of the screen that types its text out a character at a time
    // the text is wrapped when it opens, then shown a page of lines at a time

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

    struct game_dialog {
        const cchar*          text;        // not copied, it has to outlive the dialog
        u32                   text_len;
        font_line             lines[GAME_DIALOG_LINE_CAPACITY];
        u32                   line_count;
        u32                   page_line;   // the first line on the page
        f32                   reveal;      // characters shown on the page, the fraction is the next one coming in
        f32                   blink_s;
        game_dialog_state_u32 state;
    };

    struct game_npc_config {
        const cchar*  name;
        const cchar*  sprite_idle;
        const cchar*  sprite_walk_0;
        const cchar*  sprite_walk_1;
        const cchar*  sprite_walk_2;
        const cchar*  sprite_walk_3;
        const hnd_map h_map;
        const u32     chunk_index;
        const u32     chunk_x;
        const u32     chunk_y;
    };
    
    // the global arena holds what lasts the whole game
    // the player rig, map, and entities belong to the gameplay scene, and live in its arena
    struct game_context {
        hnd_arena        global_arena;
        hnd_scene        h_scene_gameplay;
        game_player_rig* player_rig;
        game_map*        map;
        game_entities*   entities;
        game_sprites*    sprites;
        game_audio*      audio;
        const font*      fnt;   // the ui font, NULL if it didn't load
        game_dialog*     dialog;
        bool             talk_was_down;
    } static * _game_ctx;

    IFB_INTERNAL game_context* game_context_create_and_init      (void);
    IFB_INTERNAL void          game_context_update_and_render    (game_context* ctx);

    IFB_INTERNAL void          game_dialog_init       (game_dialog* dialog);
    IFB_INTERNAL bool          game_dialog_open       (game_dialog* dialog, const font* fnt, const cchar* text, const u32 text_len);
    IFB_INTERNAL bool          game_dialog_is_open    (const game_dialog* dialog);
    IFB_INTERNAL void          game_dialog_advance    (game_dialog* dialog);
    IFB_INTERNAL void          game_dialog_update     (game_dialog* dialog, const f32 dt);
    IFB_INTERNAL void          game_dialog_render     (const game_dialog* dialog, const font* fnt);
    IFB_INTERNAL void          game_dialog_get_layout (const font* fnt, rect& box, rect& text_area);

    IFB_INTERNAL void          game_scene_gameplay_init              (game_context* ctx);
    IFB_INTERNAL void          game_scene_gameplay_update_and_render (game_context* ctx);

    IFB_INTERNAL void          game_player_rig_validate          (game_player_rig* player_rig);
    IFB_INTERNAL void          game_player_rig_init              (game_player_rig* player_rig);
    IFB_INTERNAL void          game_player_rig_update_and_render (game_player_rig* player_rig);
    IFB_INTERNAL void          game_player_rig_update_camera     (game_player_rig* player_rig);
    
    IFB_INTERNAL entity_id
    game_entity_spawn_static_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  atlas_name_cstr, 
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y
    );

    IFB_INTERNAL entity_id
    game_entity_spawn_static_core_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y
    );

    IFB_INTERNAL entity_id
    game_entity_spawn_core_npc_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y
    );


    IFB_INTERNAL void          game_map_init                     (game_map* map);
    IFB_INTERNAL void          game_map_update_and_render        (game_map* map);

    IFB_INTERNAL entity_id     game_npc_spawn(const game_npc_config& cfg);

};

#endif //IFB_GAME_HPP
