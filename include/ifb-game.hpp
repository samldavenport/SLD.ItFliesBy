#ifndef IFB_GAME_HPP
#define IFB_GAME_HPP

#include "ifb-types.hpp"
#include "ifb-engine.hpp"

namespace ifb {

    struct game_player_rig {
        entity_id  connor_id;
        entity_id  jig_id;
        entity_id  jig_anchor_id;
        hnd_map    starting_map_hnd;
        hnd_map    current_map_hnd;
        vec3       camera_focus;
        f32        jig_offset_x;
        f32        jig_bob_time_s;
        f32        connor_prev_x;
        bool       jig_faces_right;
    };

    struct game_entities {
        entity_id static_entity;   
    };

    struct game_map {
        hnd_map eng_hnd;
    };

    struct game_sprites {
        hnd_sprite_atlas h_atlas;
        u32              i_sprite_connor;
        u32              i_sprite_jig;
    };

    struct game_context {
        hnd_arena        global_arena;
        game_player_rig* player_rig; 
        game_map*        map;
        game_entities*   entities;
        game_sprites*    sprites;
    };

    IFB_INTERNAL game_context* game_context_create_and_init      (void);
    IFB_INTERNAL void          game_context_update_and_render    (game_context* ctx);

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

    IFB_INTERNAL void          game_map_init                     (game_map* map);
    IFB_INTERNAL void          game_map_update_and_render        (game_map* map);

};

#endif //IFB_GAME_HPP
