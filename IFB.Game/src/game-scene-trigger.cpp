#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb.hpp"

#include <cassert>

namespace ifb {

    // the trigger box is as tall as connor's quad, centered where his is, so he's always inside it vertically
    static constexpr f32 GAME_SCENE_TRIGGER_HEIGHT   = 0.2f;
    static constexpr f32 GAME_SCENE_TRIGGER_OFFSET_Y = 0.1f;

    // an edge trigger's slab, in tiles
    // thicker than a wall (MAP_WALL_HALF_THICKNESS is 0.05 a side), so connor reaches it while the wall stops him
    static constexpr f32 GAME_SCENE_TRIGGER_EDGE_THICKNESS = 0.2f;

    // the slab is nudged this far into its tile, in tiles,
    // so its center is on the tile's chunk, where connor is, even on the chunk's far edge
    static constexpr f32 GAME_SCENE_TRIGGER_EDGE_NUDGE = 0.01f;

    // the active scene's triggers, the shared collision proc finds where to go by the trigger's entity id
    // the entities belong to the scene, so the table is emptied whenever the scene switches
    struct game_scene_trigger_table {
        entity_id          id          [GAME_SCENE_TRIGGER_CAPACITY];
        game_scene_id_e    target      [GAME_SCENE_TRIGGER_CAPACITY];
        game_scene_arrival arrival     [GAME_SCENE_TRIGGER_CAPACITY];
        bool               has_arrival [GAME_SCENE_TRIGGER_CAPACITY];
        u32                count;
    } static _scene_triggers;

    IFB_INTERNAL void
    game_scene_trigger_reset(
        void) {

        _scene_triggers.count = 0;
    }

    // connor walking onto a trigger requests its scene
    // only begin, so standing on it doesn't keep asking, and a switch that's underway refuses anyway
    IFB_INTERNAL void
    game_scene_trigger_proc(
        const phys_cllsn_contact& contact) {

        assert(_game_ctx);

        if (contact.phase != phys_cllsn_phase_e_begin) return;

        const game_player_rig* rig = _game_ctx->player_rig;
        if (rig == NULL)                     return;
        if (contact.other != rig->connor_id) return;

        for (
            u32 index = 0;
                index < _scene_triggers.count;
              ++index) {

            if (_scene_triggers.id[index] != contact.self) continue;

            const game_scene_arrival* arrival = _scene_triggers.has_arrival[index]
                ? &_scene_triggers.arrival[index]
                : NULL;

            (void)game_scene_request(_scene_triggers.target[index], arrival);
            return;
        }
    }

    // an invisible trigger covering one tile of a chunk, or one edge of it, in the bound scene
    // the same tile addressing as the npcs and the static sprites
    // ENTITY_ID_INVALID if the tile isn't on the map, or the table is full
    IFB_INTERNAL entity_id
    game_scene_trigger_spawn(
        const game_scene_trigger_def& def) {

        assert(def.cstr_name);
        assert(strnlen_s(def.cstr_name, TAG_SIZE) < TAG_SIZE); // an entity tag holds TAG_SIZE - 1 characters
        assert(def.h_map != INVALID_HANDLE);
        assert(def.target > game_scene_id_e_none && def.target < game_scene_id_e_count);

        if (_scene_triggers.count >= GAME_SCENE_TRIGGER_CAPACITY) return(ENTITY_ID_INVALID);

        u32 chunk_count_rows = 0;
        u32 chunk_count_cols = 0;
        u32 chunk_origin_row = 0;
        u32 chunk_origin_col = 0;
        const bool did_get_chunk = eng_map_chunk_get_dimensions(
            def.h_map,
            def.chunk_index,
            chunk_count_rows,
            chunk_count_cols,
            chunk_origin_row,
            chunk_origin_col
        );
        if (!did_get_chunk) return(ENTITY_ID_INVALID);

        const bool is_in_chunk = (def.chunk_x < chunk_count_cols && def.chunk_y < chunk_count_rows);
        if (!is_in_chunk) return(ENTITY_ID_INVALID);

        // the center of the tile
        cmpnt_map_coords coords;
        coords.h_map = def.h_map;
        coords.col_x = (f32)(chunk_origin_col + def.chunk_x) + 0.5f;
        coords.lvl_y = 0.0f;
        coords.row_z = (f32)(chunk_origin_row + def.chunk_y) + 0.5f;

        // the next tile over, to measure a tile in world units
        cmpnt_map_coords coords_next = coords;
        coords_next.col_x += 1.0f;

        cmpnt_position pos;
        cmpnt_position pos_next;
        if (!eng_map_get_pos_from_coords(coords,      pos))      return(ENTITY_ID_INVALID);
        if (!eng_map_get_pos_from_coords(coords_next, pos_next)) return(ENTITY_ID_INVALID);

        const f32 tile_size = pos_next.x - pos.x;
        assert(tile_size > 0.0f);

        pos.y += GAME_SCENE_TRIGGER_OFFSET_Y;

        // the whole tile, or a slab along one of its edges, centered just inside the tile
        // no quad or sprite, so it's never drawn
        const f32 half_tile = tile_size * 0.5f;
        const f32 thickness = tile_size * GAME_SCENE_TRIGGER_EDGE_THICKNESS;
        const f32 to_edge   = half_tile - (tile_size * GAME_SCENE_TRIGGER_EDGE_NUDGE);

        cmpnt_bounds bounds;
        bounds.x      = 0.0f;
        bounds.y      = 0.0f;
        bounds.width  = tile_size;
        bounds.height = GAME_SCENE_TRIGGER_HEIGHT;
        bounds.depth  = tile_size;

        switch (def.side) {
            case game_scene_trigger_side_e_tile:                                            break;
            case game_scene_trigger_side_e_x_min: pos.x -= to_edge; bounds.width = thickness; break;
            case game_scene_trigger_side_e_x_max: pos.x += to_edge; bounds.width = thickness; break;
            case game_scene_trigger_side_e_y_min: pos.z -= to_edge; bounds.depth = thickness; break;
            case game_scene_trigger_side_e_y_max: pos.z += to_edge; bounds.depth = thickness; break;
            default: assert(false); return(ENTITY_ID_INVALID);
        }

        cmpnt_collider collider;
        collider.proc               = game_scene_trigger_proc;
        collider.mask_category      = PHYS_CLLSN_MASK_ALL;
        collider.mask_collides_with = PHYS_CLLSN_MASK_ALL;
        collider.is_trigger         = true;

        const entity_id id = eng_entity_create(def.cstr_name);
        if (id == ENTITY_ID_INVALID) return(ENTITY_ID_INVALID);

        bool did_spawn = true;
        did_spawn &= eng_entity_add_components   (id,
            cmpnt_type_e_position   |
            cmpnt_type_e_map_coords |
            cmpnt_type_e_bounds     |
            cmpnt_type_e_collider
        );
        // the map coords first, setting them moves the entity to the tile's center on the floor
        did_spawn &= eng_cmpnt_update_map_coords (id, coords);
        did_spawn &= eng_cmpnt_update_position   (id, pos);
        did_spawn &= eng_cmpnt_update_bounds     (id, bounds);
        did_spawn &= eng_cmpnt_update_collider   (id, collider);

        if (!did_spawn) {
            eng_entity_destroy_by_id(id);
            return(ENTITY_ID_INVALID);
        }

        const u32 index = _scene_triggers.count++;
        _scene_triggers.id          [index] = id;
        _scene_triggers.target      [index] = def.target;
        _scene_triggers.has_arrival [index] = (def.arrival != NULL);
        if (def.arrival) {
            _scene_triggers.arrival [index] = *def.arrival;
        }

        return(id);
    }
};
