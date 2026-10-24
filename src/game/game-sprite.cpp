#pragma once

#include "ifb-game.hpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"

namespace ifb {

    IFB_INTERNAL entity_id 
    game_entity_spawn_static_sprite(
        const cchar*  entity_name_cstr,
        const cchar*  atlas_name_cstr, 
        const cchar*  sprite_name_cstr,
        const hnd_map h_map,
        const u32     chunk_index,
        const u32     chunk_x,
        const u32     chunk_y) {

        assert(entity_name_cstr);
        assert(atlas_name_cstr);
        assert(sprite_name_cstr);
        assert(h_map != INVALID_HANDLE); 

        // chunk coordinates are relative to the chunk origin
        u32 chunk_count_rows = 0;
        u32 chunk_count_cols = 0;
        u32 chunk_origin_row = 0;
        u32 chunk_origin_col = 0;
        const bool did_get_chunk = eng_map_chunk_get_dimensions(
            h_map,
            chunk_index,
            chunk_count_rows,
            chunk_count_cols,
            chunk_origin_row,
            chunk_origin_col
        );
        if (!did_get_chunk) return(ENTITY_ID_INVALID);

        const bool is_in_chunk = (chunk_x < chunk_count_cols && chunk_y < chunk_count_rows);
        if (!is_in_chunk) return(ENTITY_ID_INVALID);

        // place the sprite in the center of the tile
        atype_sprite sprite = {0};
        sprite.map_coords.h_map = h_map;
        sprite.map_coords.col_x = (f32)(chunk_origin_col + chunk_x) + 0.5f;
        sprite.map_coords.lvl_y = 0.0f;
        sprite.map_coords.row_z = (f32)(chunk_origin_row + chunk_y) + 0.5f;

        const bool did_get_pos = eng_map_get_pos_from_coords(sprite.map_coords, sprite.position);
        if (!did_get_pos) return(ENTITY_ID_INVALID);

        sprite.color.hex   = 0xFFFFFFFF;
        sprite.quad.width  = 0.2f;
        sprite.quad.height = 0.2f;

        // quads are centered on their position, lift by half the height so it sits on the floor
        sprite.position.y += sprite.quad.height * 0.5f;

        const entity_id id = eng_entity_create(entity_name_cstr);
        if (id == ENTITY_ID_INVALID) return(ENTITY_ID_INVALID);

        bool did_spawn = true;
        did_spawn &= eng_entity_add_components (id, ENTITY_ARCHETYPE_SPRITE);
        did_spawn &= eng_atype_update_sprite   (id, sprite);
        did_spawn &= eng_entity_set_sprite     (id, atlas_name_cstr, sprite_name_cstr);

        if (!did_spawn) {
            eng_entity_destroy_by_id(id);
            return(ENTITY_ID_INVALID);
        }

        return(id);
    }
};
