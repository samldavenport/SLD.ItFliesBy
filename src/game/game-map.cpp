#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-types.hpp"
#include "sld.hpp"

namespace ifb {

    IFB_INTERNAL void
    game_map_init(
        game_map* map) {


        map->eng_hnd = eng_map_create("dev-map", 25, 25);
        assert(map->eng_hnd != INVALID_HANDLE);

        const u32 chunk_index_0 = eng_map_chunk_create(map->eng_hnd, 10,  10,  0, 0, map_color_e_none);
        const u32 chunk_index_1 = eng_map_chunk_create(map->eng_hnd,  5,   5, 5, 10, map_color_e_none);
        const u32 chunk_index_2 = eng_map_chunk_create(map->eng_hnd,  10, 10, 10, 5, map_color_e_gray_dark);
        
        
        assert(chunk_index_0 != INVALID_INDEX);
        assert(chunk_index_1 != INVALID_INDEX);
        assert(chunk_index_2 != INVALID_INDEX);

        bool did_set_sprites = true;
        did_set_sprites &= eng_map_chunk_set_sprite(map->eng_hnd, chunk_index_0, "CORE-SPRITES", "GRASS"); 
        did_set_sprites &= eng_map_chunk_set_sprite(map->eng_hnd, chunk_index_1, "CORE-SPRITES", "WOOD"); 
        did_set_sprites &= eng_map_set_tile_sprite(map->eng_hnd, 0, 0, "CORE-SPRITES", "WOOD");
        assert(did_set_sprites);

        bool did_set_walls = true;
        for (u32 col = 0; col < 10; ++col) did_set_walls &= eng_map_wall_set(map->eng_hnd, 0,   col, map_wall_orientation_e_row,    map_color_e_gray_dark);
        for (u32 row = 0; row < 10; ++row) did_set_walls &= eng_map_wall_set(map->eng_hnd, row, 0,   map_wall_orientation_e_column, map_color_e_gray_light);
        for (u32 row = 0; row < 7;  ++row) did_set_walls &= eng_map_wall_set(map->eng_hnd, row, 10,  map_wall_orientation_e_column, map_color_e_gray_light);
        for (u32 col = 10; col < 15; ++col) did_set_walls &= eng_map_wall_set(map->eng_hnd, 5,   col, map_wall_orientation_e_row,    map_color_e_gray_dark);
        assert(did_set_walls);

        map->chunk_count = 3;
    }

    IFB_INTERNAL void
    game_map_update_and_render(
        game_map* map) {

        assert(map);
        const bool did_render = eng_map_render(map->eng_hnd);
        assert(did_render);
    } 
};
