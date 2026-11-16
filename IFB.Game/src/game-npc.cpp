#pragma once

#include "ifb-game.hpp"
#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "game-entity.cpp"
#include <math.h>

namespace ifb {

    // an idle npc rolls its roam chance every tick, if it doesn't roam the chance grows
    // when it does, it walks to a random tile within its roam range of its spawn tile, and the chance starts over
    static constexpr f32 GAME_NPC_ROAM_TICK_S        = 1.0f;
    static constexpr f32 GAME_NPC_ROAM_CHANCE_STEP   = 0.1f;   // added every tick it doesn't roam, up to 1
    static constexpr f32 GAME_NPC_ROAM_FORCE         = 6.0f;   // a stroll, connor walks on 10 an axis
    static constexpr f32 GAME_NPC_ROAM_ARRIVE_TILES  = 0.1f;   // this close to the target tile's center is there
    static constexpr f32 GAME_NPC_ROAM_GIVE_UP_S     = 6.0f;   // it stops, if something is in the way
    static constexpr u32 GAME_NPC_ROAM_PICK_ATTEMPTS = 8;      // tries for a target tile that's on a chunk
    static constexpr f32 GAME_NPC_WALK_FRAME_S       = 0.1f;   // the same walk cycle as connor's

    // the npcs are packed at the front of the arrays, removing one moves the last one into its slot
    // while the npcs are updating, despawns are only marked, and removed once the loop is done,
    // so a proc can't move an npc out from under the loop
    //
    // an npc the manager knows isn't always in the current scene
    // it's placed when a scene spawns it, and stops being placed when the scene is switched,
    // until the next scene spawns it again
    struct game_npc_mngr {
        u32                          count;
        bool                         is_updating;

        // who the npc is, kept across scenes
        entity_id                    array_id           [GAME_NPC_MNGR_CAPACITY];  // the hash of its name, the same in every scene
        const cchar*                 array_name         [GAME_NPC_MNGR_CAPACITY];
        game_npc_sprites             array_sprites      [GAME_NPC_MNGR_CAPACITY];
        game_character_direction_u32 array_direction    [GAME_NPC_MNGR_CAPACITY];
        bool                         array_is_placed    [GAME_NPC_MNGR_CAPACITY];  // its entity is in the current scene
        bool                         array_is_despawned [GAME_NPC_MNGR_CAPACITY];

        // where it is in the scene it's placed in, set again every time it's placed
        game_npc_proc                array_proc         [GAME_NPC_MNGR_CAPACITY];
        cmpnt_map_coords             array_spawn        [GAME_NPC_MNGR_CAPACITY];  // the center of the wander radius
        game_character_state_u32     array_state        [GAME_NPC_MNGR_CAPACITY];
        f32                          array_walk_timer_s [GAME_NPC_MNGR_CAPACITY];  // time on the current walk frame
        bool                         array_walk_cycle_change [GAME_NPC_MNGR_CAPACITY];  // the next walk frame is due
        u32                          array_tile_roam_x  [GAME_NPC_MNGR_CAPACITY];
        u32                          array_tile_roam_y  [GAME_NPC_MNGR_CAPACITY];
        f32                          array_roam_chance  [GAME_NPC_MNGR_CAPACITY];  // 0 to 1
        f32                          array_roam_tick_s  [GAME_NPC_MNGR_CAPACITY];  // time toward the next roll
        f32                          array_roam_time_s  [GAME_NPC_MNGR_CAPACITY];  // time spent walking to the target
        cmpnt_map_coords             array_roam_target  [GAME_NPC_MNGR_CAPACITY];  // the center of the target tile
        bool                         array_is_roaming   [GAME_NPC_MNGR_CAPACITY];
        bool                         array_is_talking   [GAME_NPC_MNGR_CAPACITY];  // stands still facing who it's talking to, until the dialog closes
    } static * _npc_mngr;

    inline u32          game_npc_mngr_insert     (const entity_id id, const game_npc_def& def);
    inline bool         game_npc_mngr_place_at   (const u32 i_npc, const game_npc_placement& at);
    inline void         game_npc_mngr_unplace_at (const u32 i_npc);
    inline void         game_npc_mngr_remove_at  (const u32 i_npc);
    inline void         game_npc_mngr_flush      (void);
    inline u32          game_npc_mngr_index_of   (const entity_id id);
    inline bool         game_npc_mngr_does_exist (const entity_id id);
    inline const cchar* game_npc_mngr_get_sprite (const u32 npc_index);
    inline vec3         game_npc_roam            (const u32 i_npc, const f32 dt_s);
    inline bool         game_npc_roam_pick       (const u32 i_npc);
    inline void         game_npc_roam_stop       (const u32 i_npc);
    inline void         game_npc_update_timers   (const u32 i_npc, const f32 dt_s);
    inline void         game_npc_update_state    (const u32 i_npc);
    inline void         game_npc_update_sprite   (const u32 i_npc);

    IFB_GAME game_npc_mngr*
    game_npc_mngr_create(
        const hnd_arena h_arena) {

        assert(h_arena != INVALID_HANDLE);

        auto* mngr = (game_npc_mngr*)eng_arena_push(h_arena, sizeof(game_npc_mngr));
        assert(mngr);

        mngr->count       = 0;
        mngr->is_updating = false;

        _npc_mngr = mngr;
        return(mngr);
    }

    IFB_GAME void
    game_npc_mngr_update_and_render(
        void) {

        assert(_npc_mngr);

        // the first frame doesn't have a usable delta time
        f32 dt_s = eng_system_get_delta_time_s();
        if (!(dt_s >= 0.0f && dt_s < 1.0f)) dt_s = 0.0f;

        // npcs spawned by a proc land past the count, and wait for the next frame
        const u32 count = _npc_mngr->count;

        _npc_mngr->is_updating = true;
        for (
            u32 i_npc = 0;
                i_npc < count;
              ++i_npc) {

            if (_npc_mngr->array_is_despawned[i_npc]) continue;
            if (!_npc_mngr->array_is_placed[i_npc])   continue;

            // talking lasts until the dialog closes, it doesn't roam until then
            bool& is_talking = _npc_mngr->array_is_talking[i_npc];
            if (is_talking && !game_dialog_is_open()) {
                is_talking = false;
            }
            const vec3 force = is_talking ? vec3{0} : game_npc_roam(i_npc, dt_s);

            // the same order as connor, the forces, then the state, then the sprite
            game_npc_update_timers (i_npc, dt_s);
            eng_entity_add_force   (_npc_mngr->array_id[i_npc], force);
            game_npc_update_state  (i_npc);
            game_npc_update_sprite (i_npc);

            const entity_id     id   = _npc_mngr->array_id   [i_npc];
            const game_npc_proc proc = _npc_mngr->array_proc [i_npc];

            if (proc != NULL) proc(id);
        }
        _npc_mngr->is_updating = false;

        // everything despawned this frame is gone before anything is drawn
        game_npc_mngr_flush();

        for (
            u32 i_npc = 0;
                i_npc < _npc_mngr->count;
              ++i_npc) {

            if (!_npc_mngr->array_is_placed[i_npc]) continue;
            eng_entity_render(_npc_mngr->array_id[i_npc]);
        }
    }

    // forgets every npc that isn't in keep, and takes the kept ones out of the scene,
    // so the next scene can place them again without the manager starting over
    // the entities are destroyed if they still exist, like when it's called in the middle of a scene
    // during a scene switch the engine has already destroyed them with the old scene
    IFB_GAME void
    game_npc_mngr_reset_all_except(
        const entity_id* keep,
        const u32        keep_count) {

        assert(_npc_mngr);
        assert(!_npc_mngr->is_updating);
        assert(keep != NULL || keep_count == 0);

        // backwards, removing one moves the last npc into its slot, and that one has already been checked
        for (
            u32 i_npc = _npc_mngr->count;
                i_npc > 0;
              --i_npc) {

            const u32       index = i_npc - 1;
            const entity_id id    = _npc_mngr->array_id[index];

            bool is_kept = false;
            for (
                u32 i_keep = 0;
                    i_keep < keep_count;
                  ++i_keep) {

                if (keep[i_keep] == id) {
                    is_kept = true;
                    break;
                }
            }

            const bool is_despawned = _npc_mngr->array_is_despawned[index];
            if (is_kept && !is_despawned) game_npc_mngr_unplace_at (index);
            else                          game_npc_mngr_remove_at  (index);
        }
    }

    // places the npc in the bound scene
    // an npc the manager doesn't know yet is added, one it kept from the last scene keeps what it was,
    // like the way it's facing, and takes the sprites in def
    // ENTITY_ID_INVALID if it's already placed, or the manager is full, or its entity couldn't be made
    IFB_GAME entity_id
    game_npc_spawn(
        const game_npc_def&       def,
        const game_npc_placement& at) {

        assert(_npc_mngr);
        assert(!_npc_mngr->is_updating);
        assert(def.cstr_name);
        assert(def.sprites.cstr_left);
        assert(def.sprites.cstr_right);
        assert(def.sprites.cstr_walk_0);
        assert(def.sprites.cstr_walk_1);
        assert(def.sprites.cstr_walk_2);
        assert(def.sprites.cstr_walk_3);
        assert(def.sprites.cstr_walk_4);
        assert(def.sprites.cstr_walk_5);
        assert(def.sprites.cstr_walk_6);
        assert(def.sprites.cstr_walk_7);
        assert(at.h_map != INVALID_HANDLE);

        // the same id the engine gives the entity
        const tag       name = def.cstr_name;
        const entity_id id   = name.to_id();

        u32        i_npc  = game_npc_mngr_index_of(id);
        const bool is_new = (i_npc == INVALID_INDEX);
        if (is_new) {
            i_npc = game_npc_mngr_insert(id, def);
            if (i_npc == INVALID_INDEX) return(ENTITY_ID_INVALID);
        }
        else {
            if (_npc_mngr->array_is_placed    [i_npc]) return(ENTITY_ID_INVALID);
            if (_npc_mngr->array_is_despawned [i_npc]) return(ENTITY_ID_INVALID);
            _npc_mngr->array_name    [i_npc] = def.cstr_name;
            _npc_mngr->array_sprites [i_npc] = def.sprites;
        }

        const bool did_place = game_npc_mngr_place_at(i_npc, at);
        if (!did_place) {
            // a new one was inserted last, so nothing has moved into its slot
            if (is_new) --_npc_mngr->count;
            return(ENTITY_ID_INVALID);
        }

        return(id);
    }

    IFB_GAME bool
    game_npc_despawn(
        const entity_id id) {

        assert(_npc_mngr);
        assert(id != ENTITY_ID_INVALID);

        const u32 i_npc = game_npc_mngr_index_of(id);
        if (i_npc == INVALID_INDEX)                    return(false);
        if (_npc_mngr->array_is_despawned[i_npc])      return(false);

        _npc_mngr->array_is_despawned[i_npc] = true;

        // outside of the update loop it can go right away
        if (!_npc_mngr->is_updating) game_npc_mngr_flush();

        return(true);
    }

    // turns the npc toward the other entity, and keeps it standing there until the dialog closes
    // call it when opening the npc's dialog
    // false if the id isn't an npc, or either position can't be found
    IFB_GAME bool
    game_npc_talk_to(
        const entity_id id,
        const entity_id other_id) {

        assert(_npc_mngr);
        assert(id       != ENTITY_ID_INVALID);
        assert(other_id != ENTITY_ID_INVALID);

        const u32 i_npc = game_npc_mngr_index_of(id);
        if (i_npc == INVALID_INDEX)               return(false);
        if (_npc_mngr->array_is_despawned[i_npc]) return(false);
        if (!_npc_mngr->array_is_placed[i_npc])   return(false);

        cmpnt_position pos_npc;
        cmpnt_position pos_other;
        if (!eng_cmpnt_lookup_position(id,       pos_npc))   return(false);
        if (!eng_cmpnt_lookup_position(other_id, pos_other)) return(false);

        // straight above or below keeps the way it was facing
        const f32 dx = pos_other.x - pos_npc.x;
        if      (dx < 0.0f) _npc_mngr->array_direction[i_npc] = game_character_direction_e_left;
        else if (dx > 0.0f) _npc_mngr->array_direction[i_npc] = game_character_direction_e_right;

        game_npc_roam_stop(i_npc);
        _npc_mngr->array_is_talking[i_npc] = true;

        // the sprite now, instead of on the next update
        game_npc_update_state  (i_npc);
        game_npc_update_sprite (i_npc);
        return(true);
    }

    // adds who the npc is, it isn't placed yet
    // its index, INVALID_INDEX if the manager is full
    inline u32
    game_npc_mngr_insert(
        const entity_id     id,
        const game_npc_def& def) {

        assert(_npc_mngr);
        assert(id != ENTITY_ID_INVALID);
        assert(!game_npc_mngr_does_exist(id));

        if (_npc_mngr->count >= GAME_NPC_MNGR_CAPACITY) return(INVALID_INDEX);

        const u32 index = _npc_mngr->count++;

        _npc_mngr->array_id           [index] = id;
        _npc_mngr->array_name         [index] = def.cstr_name;
        _npc_mngr->array_sprites      [index] = def.sprites;
        _npc_mngr->array_direction    [index] = game_character_direction_e_right;
        _npc_mngr->array_is_placed    [index] = false;
        _npc_mngr->array_is_despawned [index] = false;

        return(index);
    }

    // makes the npc's entity in the bound scene, and starts it standing still on its spawn tile
    // false if the entity couldn't be made, nothing is left behind
    inline bool
    game_npc_mngr_place_at(
        const u32                 i_npc,
        const game_npc_placement& at) {

        assert(_npc_mngr);
        assert(i_npc < _npc_mngr->count);
        assert(!_npc_mngr->array_is_placed[i_npc]);

        const bool is_facing_left = _npc_mngr->array_direction[i_npc] == game_character_direction_e_left;
        const game_npc_sprites& sprites = _npc_mngr->array_sprites[i_npc];

        const entity_id id = game_entity_spawn_core_npc_sprite(
            _npc_mngr->array_name[i_npc],
            is_facing_left ? sprites.cstr_left : sprites.cstr_right,
            at.h_map,
            at.chunk_index,
            at.chunk_x,
            at.chunk_y
        );
        if (id == ENTITY_ID_INVALID) return(false);
        assert(id == _npc_mngr->array_id[i_npc]);

        // npcs can't be pushed, they still walk on their own forces, the tag only stops collisions from moving them
        const bool did_tag = eng_entity_add_components(id, cmpnt_type_e_immovable);
        if (!did_tag) {
            eng_entity_destroy_by_id(id);
            return(false);
        }

        cmpnt_collider collider;
        collider.proc               = at.cllsn_proc;
        collider.mask_category      = at.mask_category;
        collider.mask_collides_with = at.mask_collides_with;
        collider.is_trigger         = false;
        collider.restitution        = 0.0f;

        bool did_add_collider = true;
        did_add_collider &= eng_entity_add_components (id, cmpnt_type_e_collider);
        did_add_collider &= eng_cmpnt_update_collider (id, collider);
        assert(did_add_collider);

        cmpnt_map_coords spawn;
        const bool did_get_spawn = eng_map_get_entity_coords(at.h_map, id, spawn);
        if (!did_get_spawn) {
            eng_entity_destroy_by_id(id);
            return(false);
        }

        _npc_mngr->array_proc         [i_npc] = at.proc;
        _npc_mngr->array_spawn        [i_npc] = spawn;
        _npc_mngr->array_state        [i_npc] = is_facing_left ? game_character_state_e_idle_left : game_character_state_e_idle_right;
        _npc_mngr->array_walk_timer_s [i_npc] = 0.0f;
        _npc_mngr->array_walk_cycle_change [i_npc] = false;
        _npc_mngr->array_tile_roam_x  [i_npc] = at.tile_roam_x;
        _npc_mngr->array_tile_roam_y  [i_npc] = at.tile_roam_y;
        _npc_mngr->array_roam_chance  [i_npc] = 0.0f;
        _npc_mngr->array_roam_tick_s  [i_npc] = 0.0f;
        _npc_mngr->array_roam_time_s  [i_npc] = 0.0f;
        _npc_mngr->array_roam_target  [i_npc] = spawn;
        _npc_mngr->array_is_roaming   [i_npc] = false;
        _npc_mngr->array_is_talking   [i_npc] = false;
        _npc_mngr->array_is_placed    [i_npc] = true;

        return(true);
    }

    // takes the npc out of the scene, the manager still knows it
    // its entity is destroyed if it still exists
    inline void
    game_npc_mngr_unplace_at(
        const u32 i_npc) {

        assert(_npc_mngr);
        assert(!_npc_mngr->is_updating);
        assert(i_npc < _npc_mngr->count);

        if (!_npc_mngr->array_is_placed[i_npc]) return;

        // false when the engine already destroyed it with its scene
        (void)eng_entity_destroy_by_id(_npc_mngr->array_id[i_npc]);

        _npc_mngr->array_is_placed  [i_npc] = false;
        _npc_mngr->array_is_roaming [i_npc] = false;
        _npc_mngr->array_is_talking [i_npc] = false;
    }

    // takes the npc out of the scene, and moves the last npc into its slot
    inline void
    game_npc_mngr_remove_at(
        const u32 i_npc) {

        assert(_npc_mngr);
        assert(!_npc_mngr->is_updating);
        assert(i_npc < _npc_mngr->count);

        game_npc_mngr_unplace_at(i_npc);

        const u32 i_npc_last = _npc_mngr->count - 1;

        if (i_npc != i_npc_last) {
            _npc_mngr->array_id           [i_npc] = _npc_mngr->array_id           [i_npc_last];
            _npc_mngr->array_name         [i_npc] = _npc_mngr->array_name         [i_npc_last];
            _npc_mngr->array_is_placed    [i_npc] = _npc_mngr->array_is_placed    [i_npc_last];
            _npc_mngr->array_proc         [i_npc] = _npc_mngr->array_proc         [i_npc_last];
            _npc_mngr->array_sprites      [i_npc] = _npc_mngr->array_sprites      [i_npc_last];
            _npc_mngr->array_spawn        [i_npc] = _npc_mngr->array_spawn        [i_npc_last];
            _npc_mngr->array_direction    [i_npc] = _npc_mngr->array_direction    [i_npc_last];
            _npc_mngr->array_state        [i_npc] = _npc_mngr->array_state        [i_npc_last];
            _npc_mngr->array_walk_timer_s [i_npc] = _npc_mngr->array_walk_timer_s [i_npc_last];
            _npc_mngr->array_walk_cycle_change [i_npc] = _npc_mngr->array_walk_cycle_change [i_npc_last];
            _npc_mngr->array_tile_roam_x  [i_npc] = _npc_mngr->array_tile_roam_x  [i_npc_last];
            _npc_mngr->array_tile_roam_y  [i_npc] = _npc_mngr->array_tile_roam_y  [i_npc_last];
            _npc_mngr->array_roam_chance  [i_npc] = _npc_mngr->array_roam_chance  [i_npc_last];
            _npc_mngr->array_roam_tick_s  [i_npc] = _npc_mngr->array_roam_tick_s  [i_npc_last];
            _npc_mngr->array_roam_time_s  [i_npc] = _npc_mngr->array_roam_time_s  [i_npc_last];
            _npc_mngr->array_roam_target  [i_npc] = _npc_mngr->array_roam_target  [i_npc_last];
            _npc_mngr->array_is_roaming   [i_npc] = _npc_mngr->array_is_roaming   [i_npc_last];
            _npc_mngr->array_is_despawned [i_npc] = _npc_mngr->array_is_despawned [i_npc_last];
            _npc_mngr->array_is_talking   [i_npc] = _npc_mngr->array_is_talking   [i_npc_last];
        }

        _npc_mngr->count--;
    }

    // walks backwards, so the npc moved into a removed slot has already been checked
    inline void
    game_npc_mngr_flush(
        void) {

        assert(_npc_mngr);

        for (
            u32 i_npc = _npc_mngr->count;
                i_npc > 0;
              --i_npc) {

            const u32 index = i_npc - 1;
            if (_npc_mngr->array_is_despawned[index]) {
                game_npc_mngr_remove_at(index);
            }
        }
    }

    inline u32
    game_npc_mngr_index_of(
        const entity_id id) {

        assert(id != ENTITY_ID_INVALID);
        assert(_npc_mngr);

        for (
            u32 i_npc = 0;
                i_npc < _npc_mngr->count;
              ++i_npc) {

            if (_npc_mngr->array_id[i_npc] == id) {
                return(i_npc);
            }
        }

        return(INVALID_INDEX);
    }

    inline bool
    game_npc_mngr_does_exist(
        const entity_id id) {

        const u32 i_npc = game_npc_mngr_index_of(id);
        return(i_npc != INVALID_INDEX);
    }

    // the npc is in the current scene
    IFB_GAME bool
    game_npc_is_placed(
        const entity_id id) {

        assert(_npc_mngr);

        const u32 i_npc = game_npc_mngr_index_of(id);
        if (i_npc == INVALID_INDEX) return(false);

        return(
            _npc_mngr->array_is_placed    [i_npc] &&
           !_npc_mngr->array_is_despawned [i_npc]
        );
    }
    
    inline const cchar*
    game_npc_mngr_get_sprite(
        const u32 npc_index) {

        assert(npc_index != INVALID_INDEX);
        assert(npc_index <  _npc_mngr->count);
           
        const game_character_state_u32 state   = _npc_mngr->array_state   [npc_index];
        const game_npc_sprites         sprites = _npc_mngr->array_sprites [npc_index]; 

        const cchar* sprite = NULL; 
        switch(state.val) {
            case (game_character_state_e_idle_left):    sprite = sprites.cstr_left;   break;   
            case (game_character_state_e_idle_right):   sprite = sprites.cstr_right;  break;  
            case (game_character_state_e_walk_left_1):  sprite = sprites.cstr_walk_0; break; 
            case (game_character_state_e_walk_left_2):  sprite = sprites.cstr_walk_1; break; 
            case (game_character_state_e_walk_left_3):  sprite = sprites.cstr_walk_2; break; 
            case (game_character_state_e_walk_left_4):  sprite = sprites.cstr_walk_3; break; 
            case (game_character_state_e_walk_right_1): sprite = sprites.cstr_walk_4; break;
            case (game_character_state_e_walk_right_2): sprite = sprites.cstr_walk_5; break;
            case (game_character_state_e_walk_right_3): sprite = sprites.cstr_walk_6; break;
            case (game_character_state_e_walk_right_4): sprite = sprites.cstr_walk_7; break;
            default:                                    sprite = sprites.cstr_right;  break;
        }
        assert(sprite != NULL);

        return(sprite);
    }

    // idle, it rolls the roam chance every tick, walking, it pushes toward the target tile
    // the force to add this frame, zero when it isn't walking
    inline vec3
    game_npc_roam(
        const u32 i_npc,
        const f32 dt_s) {

        auto* mngr = _npc_mngr;
        const entity_id id = mngr->array_id[i_npc];

        vec3 force = {0};

        if (!mngr->array_is_roaming[i_npc]) {

            mngr->array_roam_tick_s[i_npc] += dt_s;
            if (mngr->array_roam_tick_s[i_npc] < GAME_NPC_ROAM_TICK_S) return(force);
            mngr->array_roam_tick_s[i_npc] -= GAME_NPC_ROAM_TICK_S;

            // it only starts over once it actually goes somewhere
            const bool does_roam = eng_random_f32() < mngr->array_roam_chance[i_npc];
            if (does_roam && game_npc_roam_pick(i_npc)) {
                mngr->array_is_roaming  [i_npc] = true;
                mngr->array_roam_time_s [i_npc] = 0.0f;
                mngr->array_roam_chance [i_npc] = 0.0f;
                return(force);
            }

            f32& chance = mngr->array_roam_chance[i_npc];
            chance += GAME_NPC_ROAM_CHANCE_STEP;
            if (chance > 1.0f) chance = 1.0f;
            return(force);
        }

        const cmpnt_map_coords& target = mngr->array_roam_target[i_npc];

        cmpnt_map_coords coords;
        if (!eng_map_get_entity_coords(target.h_map, id, coords)) {
            game_npc_roam_stop(i_npc);
            return(force);
        }

        mngr->array_roam_time_s[i_npc] += dt_s;

        // the map's columns run along x, and its rows along z
        const f32 delta_col = target.col_x - coords.col_x;
        const f32 delta_row = target.row_z - coords.row_z;
        const f32 distance  = sqrtf((delta_col * delta_col) + (delta_row * delta_row));

        const bool is_there    = distance                       <= GAME_NPC_ROAM_ARRIVE_TILES;
        const bool is_given_up = mngr->array_roam_time_s[i_npc] >= GAME_NPC_ROAM_GIVE_UP_S;
        if (is_there || is_given_up) {
            game_npc_roam_stop(i_npc);
            return(force);
        }

        force.x = (delta_col / distance) * GAME_NPC_ROAM_FORCE;
        force.z = (delta_row / distance) * GAME_NPC_ROAM_FORCE;

        // walking straight up or down keeps the way it was facing
        if (fabsf(delta_col) > GAME_NPC_ROAM_ARRIVE_TILES) {
            mngr->array_direction[i_npc] = (delta_col < 0.0f)
                ? game_character_direction_e_left
                : game_character_direction_e_right;
        }

        return(force);
    }

    // a random tile within the roam range of the spawn tile, that's on a chunk, and isn't the one it's on
    // false if it didn't find one, like with no roam range
    inline bool
    game_npc_roam_pick(
        const u32 i_npc) {

        auto* mngr = _npc_mngr;

        const s32 range_cols = (s32)mngr->array_tile_roam_x[i_npc];
        const s32 range_rows = (s32)mngr->array_tile_roam_y[i_npc];
        if (range_cols == 0 && range_rows == 0) return(false);

        const cmpnt_map_coords& spawn     = mngr->array_spawn[i_npc];
        const s32               spawn_col = (s32)floorf(spawn.col_x);
        const s32               spawn_row = (s32)floorf(spawn.row_z);

        cmpnt_map_coords coords;
        if (!eng_map_get_entity_coords(spawn.h_map, mngr->array_id[i_npc], coords)) return(false);
        const s32 current_col = (s32)floorf(coords.col_x);
        const s32 current_row = (s32)floorf(coords.row_z);

        for (
            u32 attempt = 0;
                attempt < GAME_NPC_ROAM_PICK_ATTEMPTS;
              ++attempt) {

            // -range to +range, inclusive
            const s32 offset_col = (s32)(eng_random_u32() % (u32)((range_cols * 2) + 1)) - range_cols;
            const s32 offset_row = (s32)(eng_random_u32() % (u32)((range_rows * 2) + 1)) - range_rows;
            const s32 col        = spawn_col + offset_col;
            const s32 row        = spawn_row + offset_row;

            if (col < 0 || row < 0)                           continue;
            if (col == current_col && row == current_row)     continue;

            cmpnt_map_coords target;
            target.h_map = spawn.h_map;
            target.col_x = (f32)col + 0.5f;
            target.lvl_y = spawn.lvl_y;
            target.row_z = (f32)row + 0.5f;

            // in bounds means on a chunk
            if (!eng_map_is_in_bounds(target)) continue;

            mngr->array_roam_target[i_npc] = target;
            return(true);
        }

        return(false);
    }

    // it waits a whole tick before its next roll
    inline void
    game_npc_roam_stop(
        const u32 i_npc) {

        _npc_mngr->array_is_roaming  [i_npc] = false;
        _npc_mngr->array_roam_time_s [i_npc] = 0.0f;
        _npc_mngr->array_roam_tick_s [i_npc] = 0.0f;
    }

    // the same as game_player_rig_update_timers, the walk cycle moves on every GAME_NPC_WALK_FRAME_S
    inline void
    game_npc_update_timers(
        const u32 i_npc,
        const f32 dt_s) {

        _npc_mngr->array_walk_timer_s[i_npc] += dt_s;

        if (_npc_mngr->array_walk_timer_s[i_npc] >= GAME_NPC_WALK_FRAME_S) {
            _npc_mngr->array_walk_timer_s      [i_npc] = 0.0f;
            _npc_mngr->array_walk_cycle_change [i_npc] = true;
        }
    }

    // the same as game_player_rig_update_connor_state
    // stopped, it idles facing its direction, moving, it steps through that direction's walk cycle
    inline void
    game_npc_update_state(
        const u32 i_npc) {

        auto* mngr = _npc_mngr;

        const bool is_npc_moving   = eng_entity_is_moving(mngr->array_id[i_npc]);
        const bool is_facing_left  = mngr->array_direction[i_npc] == game_character_direction_e_left;
        const bool is_facing_right = mngr->array_direction[i_npc] == game_character_direction_e_right;
        const bool is_moving_left  = is_npc_moving  && is_facing_left;
        const bool is_moving_right = is_npc_moving  && is_facing_right;
        const bool is_idle_left    = !is_npc_moving && is_facing_left;
        const bool is_idle_right   = !is_npc_moving && is_facing_right;

        game_character_state_u32& state             = mngr->array_state             [i_npc];
        bool&                     walk_cycle_change = mngr->array_walk_cycle_change [i_npc];

        if      (is_idle_left)  state = game_character_state_e_idle_left;
        else if (is_idle_right) state = game_character_state_e_idle_right;

        if (is_moving_left && walk_cycle_change) {
            switch (state.val) {
                case (game_character_state_e_walk_left_1):  state = game_character_state_e_walk_left_2; break;
                case (game_character_state_e_walk_left_2):  state = game_character_state_e_walk_left_3; break;
                case (game_character_state_e_walk_left_3):  state = game_character_state_e_walk_left_4; break;
                default:                                    state = game_character_state_e_walk_left_1; break;
            }
            walk_cycle_change = false;
        }
        else if (is_moving_right && walk_cycle_change) {
            switch (state.val) {
                case (game_character_state_e_walk_right_1): state = game_character_state_e_walk_right_2; break;
                case (game_character_state_e_walk_right_2): state = game_character_state_e_walk_right_3; break;
                case (game_character_state_e_walk_right_3): state = game_character_state_e_walk_right_4; break;
                default:                                    state = game_character_state_e_walk_right_1; break;
            }
            walk_cycle_change = false;
        }
    }

    // the same as game_player_rig_update_sprites
    inline void
    game_npc_update_sprite(
        const u32 i_npc) {

        // a name that isn't in the atlas leaves the old sprite up, so catch it here
        const cchar* sprite     = game_npc_mngr_get_sprite(i_npc);
        const bool   did_set    = eng_entity_set_sprite(_npc_mngr->array_id[i_npc], "CORE-SPRITES", sprite);
        assert(did_set);
        (void)did_set;
    }
};
