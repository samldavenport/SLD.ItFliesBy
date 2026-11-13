#pragma once

#include "ifb-game.hpp"
#include "ifb.hpp"
#include "ifb-engine.hpp"
#include "game-entity.cpp"

namespace ifb {

    static constexpr u32 GAME_NPC_MNGR_CAPACITY = 64;

    // the npcs are packed at the front of the arrays, removing one moves the last one into its slot
    // while the npcs are updating, despawns are only marked, and removed once the loop is done,
    // so a proc can't move an npc out from under the loop
    struct game_npc_mngr {
        u32              count;
        bool             is_updating;
        entity_id        array_id          [GAME_NPC_MNGR_CAPACITY];
        game_npc_proc    array_proc        [GAME_NPC_MNGR_CAPACITY];
        game_npc_sprites array_sprites     [GAME_NPC_MNGR_CAPACITY];
        cmpnt_map_coords array_spawn       [GAME_NPC_MNGR_CAPACITY];  // the center of the wander radius
        bool             array_is_despawned[GAME_NPC_MNGR_CAPACITY];
    } static * _npc_mngr;

    inline bool game_npc_mngr_insert     (const entity_id id, const game_npc_config& cfg);
    inline void game_npc_mngr_remove_at  (const u32 i_npc);
    inline void game_npc_mngr_flush      (void);
    inline u32  game_npc_mngr_index_of   (const entity_id id);
    inline bool game_npc_mngr_does_exist (const entity_id id);

    IFB_INTERNAL game_npc_mngr*
    game_npc_mngr_create(
        const hnd_scene h_scene) {

        assert(h_scene != INVALID_HANDLE);

        auto* mngr = (game_npc_mngr*)eng_scene_alloc(h_scene, sizeof(game_npc_mngr));
        assert(mngr);

        mngr->count       = 0;
        mngr->is_updating = false;

        _npc_mngr = mngr;
        return(mngr);
    }

    IFB_INTERNAL void
    game_npc_mngr_update_and_render(
        void) {

        assert(_npc_mngr);

        // npcs spawned by a proc land past the count, and wait for the next frame
        const u32 count = _npc_mngr->count;

        _npc_mngr->is_updating = true;
        for (
            u32 i_npc = 0;
                i_npc < count;
              ++i_npc) {

            if (_npc_mngr->array_is_despawned[i_npc]) continue;

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

            eng_entity_render(_npc_mngr->array_id[i_npc]);
        }
    }

    IFB_INTERNAL entity_id
    game_npc_spawn(
        const game_npc_config& cfg) {

        assert(_npc_mngr);
        assert(cfg.cstr_name);
        assert(cfg.sprites.cstr_idle);
        assert(cfg.sprites.cstr_walk_0);
        assert(cfg.sprites.cstr_walk_1);
        assert(cfg.sprites.cstr_walk_2);
        assert(cfg.sprites.cstr_walk_3);
        assert(cfg.h_map != INVALID_HANDLE);

        if (_npc_mngr->count >= GAME_NPC_MNGR_CAPACITY) return(ENTITY_ID_INVALID);

        const entity_id id = game_entity_spawn_core_npc_sprite(
            cfg.cstr_name,
            cfg.sprites.cstr_idle,
            cfg.h_map,
            cfg.chunk_index,
            cfg.chunk_x,
            cfg.chunk_y
        );
        if (id == ENTITY_ID_INVALID) return(ENTITY_ID_INVALID);

        // npcs can't be pushed, they still walk on their own forces, the tag only stops collisions from moving them
        const bool did_tag = eng_entity_add_components(id, cmpnt_type_e_immovable);
        if (!did_tag) {
            eng_entity_destroy_by_id(id);
            return(ENTITY_ID_INVALID);
        }

        const bool did_insert = game_npc_mngr_insert(id, cfg);
        if (!did_insert) {
            eng_entity_destroy_by_id(id);
            return(ENTITY_ID_INVALID);
        }

        return(id);
    }

    IFB_INTERNAL bool
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

    inline bool
    game_npc_mngr_insert(
        const entity_id        id,
        const game_npc_config& cfg) {

        assert(_npc_mngr);
        assert(id != ENTITY_ID_INVALID);
        assert(!game_npc_mngr_does_exist(id));

        if (_npc_mngr->count >= GAME_NPC_MNGR_CAPACITY) return(false);

        cmpnt_map_coords spawn;
        const bool did_get_spawn = eng_map_get_entity_coords(cfg.h_map, id, spawn);
        if (!did_get_spawn) return(false);

        const u32 index = _npc_mngr->count++;

        _npc_mngr->array_id           [index] = id;
        _npc_mngr->array_proc         [index] = cfg.proc;
        _npc_mngr->array_sprites      [index] = cfg.sprites;
        _npc_mngr->array_spawn        [index] = spawn;
        _npc_mngr->array_is_despawned [index] = false;

        return(true);
    }

    // destroys the entity, and moves the last npc into its slot
    inline void
    game_npc_mngr_remove_at(
        const u32 i_npc) {

        assert(_npc_mngr);
        assert(!_npc_mngr->is_updating);
        assert(i_npc < _npc_mngr->count);

        const bool did_destroy = eng_entity_destroy_by_id(_npc_mngr->array_id[i_npc]);
        assert(did_destroy);
        (void)did_destroy;

        const u32 i_npc_last = _npc_mngr->count - 1;

        if (i_npc != i_npc_last) {
            _npc_mngr->array_id           [i_npc] = _npc_mngr->array_id           [i_npc_last];
            _npc_mngr->array_proc         [i_npc] = _npc_mngr->array_proc         [i_npc_last];
            _npc_mngr->array_sprites      [i_npc] = _npc_mngr->array_sprites      [i_npc_last];
            _npc_mngr->array_spawn        [i_npc] = _npc_mngr->array_spawn        [i_npc_last];
            _npc_mngr->array_is_despawned [i_npc] = _npc_mngr->array_is_despawned [i_npc_last];
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
};
