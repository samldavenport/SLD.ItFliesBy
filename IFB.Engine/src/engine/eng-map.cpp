#pragma once

#include "ifb-engine.hpp"
#include "ifb.hpp"
#include "eng-component.hpp"
#include "sld.hpp"
#include "eng-entity.hpp"
#include "eng-map.hpp"
#include "eng-renderer.hpp"
#include "eng-asset.hpp"
#include "eng-file.hpp"
#include "eng-json.hpp"
#include "eng-memory.hpp"

namespace ifb {

    IFB_ENGINE_API hnd_map
    eng_map_create(
        const cchar* name,
        const u32    count_rows,
        const u32    count_cols) {

        assert(name       != NULL);
        assert(count_rows != 0);
        assert(count_cols != 0);

        const hnd_map hnd = map_create(
            name,
            count_rows,
            count_cols
        );

        return(hnd);
    }

    // builds a map from a text asset in the runtime database, see docs/map-format.md
    // the atlases the map uses have to be loaded first
    IFB_ENGINE_API hnd_map
    eng_map_load(
        const cchar* asset_name) {

        const hnd_map h_map = eng_map_load_as(asset_name, NULL);
        return(h_map);
    }

    // like eng_map_load, but the map is named map_name instead of the name in the file
    // a map's handle is the hash of its name, so this is how one file is loaded more than once
    IFB_ENGINE_API hnd_map
    eng_map_load_as(
        const cchar* asset_name,
        const cchar* map_name) {

        assert(asset_name != NULL);

        const asset_database* db = asset_mngr_get_database();
        if (db == NULL) {
            return(INVALID_HANDLE);
        }

        // the text and the parsed document only live while the map is built, the map copies what it needs
        const hnd_arena h_arena = arena_alloc();
        if (h_arena == INVALID_HANDLE) {
            return(INVALID_HANDLE);
        }

        hnd_map h_map = INVALID_HANDLE;

        const asset_text* txt = asset_database_load_text_by_name_to_arena(db, asset_name, h_arena);
        if (txt != NULL && txt->size != 0) {
            h_map = map_parse_from_json(txt->cstr, txt->size, h_arena, map_name);
        }

        arena_free(h_arena);
        return(h_map);
    }

    IFB_ENGINE_API u32 
    eng_map_chunk_create(
        const hnd_map       map_hnd,
        const u32           count_rows,
        const u32           count_cols,
        const u32           origin_row,
        const u32           origin_col,
        const map_color_u32 base_color) {

        assert(map_hnd     != INVALID_HANDLE);
        assert(count_rows  != 0);
        assert(count_cols  != 0);
       
        const u32 chunk_index = map_chunk_create(
            map_hnd,
            count_rows,
            count_cols,
            origin_row,
            origin_col,
            base_color
        );

        return(chunk_index);
    } 

    // the most text a map file can be, the most tiles and wall runs a map can have fit well inside it
    static constexpr u32 ENG_MAP_SAVE_CAPACITY = (u32)size_megabytes(8);

    // writes the map as a map file at path, see docs/map-format.md, named map_name
    // the file is replaced if it's there, false if the map can't be written or the file can't be
    IFB_ENGINE_API bool
    eng_map_save(
        const hnd_map h_map,
        const cchar*  map_name,
        const cchar*  path) {

        assert(h_map    != INVALID_HANDLE);
        assert(map_name != NULL);
        assert(path     != NULL);

        const hnd_arena h_arena = arena_large_alloc();
        if (h_arena == INVALID_HANDLE) return(false);

        bool did_save = false;

        cchar* buffer = (cchar*)arena_push(h_arena, ENG_MAP_SAVE_CAPACITY);
        const u32 length = (buffer != NULL) ? map_write_json(h_map, map_name, buffer, ENG_MAP_SAVE_CAPACITY) : 0;
        if (length != 0) {
            const hnd_file h_file = file_wo_overwrite(path);
            if (h_file != FILE_HANDLE_INVALID) {
                did_save = (file_write(h_file, length, (const byte*)buffer) == length);
                file_close(h_file);
            }
        }

        arena_free(h_arena);
        return(did_save);
    }

    // the name inside a map asset, which is what eng_map_load names the map
    // false if there's no text asset by that name, or it isn't a map file with a name
    IFB_ENGINE_API bool
    eng_map_asset_get_name(
        const cchar* asset_name,
        cchar*       name,
        const u32    name_size) {

        assert(asset_name != NULL);
        assert(name       != NULL);
        assert(name_size  != 0);

        name[0] = 0;

        const asset_database* db = asset_mngr_get_database();
        if (db == NULL) return(false);

        const hnd_arena h_arena = arena_alloc();
        if (h_arena == INVALID_HANDLE) return(false);

        bool did_get = false;

        const asset_text* txt = asset_database_load_text_by_name_to_arena(db, asset_name, h_arena);
        const json_doc*   doc = (txt != NULL && txt->size != 0) ? json_doc_create(h_arena, txt->size, txt->cstr) : NULL;
        if (doc != NULL) {
            const cchar* file_name = NULL;
            if (json_obj_try_get_cstr(json_doc_get_root(doc), "name", file_name)) {
                const u32 length = cstr_nvar_length(file_name, name_size);
                if (length != 0 && length < name_size) {
                    memcpy(name, file_name, length);
                    name[length] = 0;
                    did_get = true;
                }
            }
        }

        arena_free(h_arena);
        return(did_get);
    }

    IFB_ENGINE_API void
    eng_map_destroy(
        const hnd_map map) {

        assert(map != INVALID_HANDLE);
        (void)map_destroy(map);
    }

    //--------------------------------------------------------------------
    // MAP VIEW
    //--------------------------------------------------------------------

    // draws the map offscreen this frame, with the map view's camera
    // like the particle view, it only draws on frames it's requested
    IFB_ENGINE_API void
    eng_map_view_request(
        const hnd_map h_map,
        const u32     width,
        const u32     height) {

        renderer_request_map_view(h_map, width, height);
    }

    IFB_ENGINE_API u32
    eng_map_view_get_texture(
        void) {

        return(renderer_get_map_view_texture());
    }

    IFB_ENGINE_API void
    eng_map_view_get_camera(
        vec3& origin,
        vec3& target) {

        renderer_get_map_view_camera(origin, target);
    }

    IFB_ENGINE_API void
    eng_map_view_set_camera(
        const vec3& origin,
        const vec3& target) {

        renderer_set_map_view_camera(origin, target);
    }

    // the view projection for the size the view was last requested at,
    // for turning a point in the view into a ray, or a world position into the view
    IFB_ENGINE_API void
    eng_map_view_get_xform(
        mat4& xform) {

        renderer_get_map_view_xform(xform);
    }

    IFB_ENGINE_API void
    eng_map_view_set_grid_visible(
        const bool is_visible) {

        renderer_set_map_view_grid_visible(is_visible);
    }

    IFB_ENGINE_API bool 
    eng_map_render(
        const hnd_map map_hnd) {

        const bool result = renderer_set_map(map_hnd);
        return(result);
    }

    IFB_ENGINE_API void
    eng_map_set_colors(
        const hnd_map        map,
        const cmpnt_map_coords* coords,
        const color_rgba_u32*   color,
        const u32               count) {

        assert(map != INVALID_HANDLE);
        assert(coords   != NULL);
        assert(color    != NULL);
        assert(count    != 0);

    } 

    IFB_ENGINE_API bool
    eng_map_get_entity_coords(
        const hnd_map     map,
        const entity_id   eid,
        cmpnt_map_coords& coords) {

        assert(map != INVALID_HANDLE);
        assert(eid != ENTITY_ID_INVALID);

        entity e;
        bool result = true;
        result &= entity_lookup_by_id  (e, eid);
        result &= entity_has_component (e, cmpnt_type_e_position);
        if (!result) {
            return(false);
        }

        // the entity doesn't need to belong to the map,
        // this is where its position lands on it
        cmpnt_position pos;
        cmpnt_lookup_position(e.index_sparse, pos);
        coords.h_map = map;

        const bool did_get = map_get_coords_from_pos(pos, coords);
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_map_set_origin(
        const hnd_map map,
        const vec3&   origin) {

        assert(map != INVALID_HANDLE);

        const bool did_set = map_set_origin(map, origin);
        return(did_set);
    }

    IFB_ENGINE_API bool
    eng_map_get_dimensions(
        const hnd_map map,
        u32&          count_rows,
        u32&          count_cols) {

        assert(map != INVALID_HANDLE);

        map_dimensions dims;
        const bool did_get = map_get_dimensions(map, dims);
        if (did_get) {
            count_rows = dims.count_rows;
            count_cols = dims.count_cols;
        }
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_map_chunk_get_dimensions(
        const hnd_map map,
        const u32     chunk_index,
        u32&          count_rows,
        u32&          count_cols,
        u32&          origin_row,
        u32&          origin_col) {

        assert(map != INVALID_HANDLE);

        const bool did_get = map_chunk_get_dimensions(
            map,
            chunk_index,
            count_rows,
            count_cols,
            origin_row,
            origin_col
        );
        return(did_get);
    }

    IFB_ENGINE_API u32
    eng_map_get_chunk_count(
        const hnd_map map) {

        assert(map != INVALID_HANDLE);
        return(map_get_chunk_count(map));
    }

    IFB_ENGINE_API bool
    eng_map_chunk_get_info(
        const hnd_map   map,
        const u32       chunk_index,
        map_chunk_info& info) {

        assert(map != INVALID_HANDLE);
        return(map_chunk_get_info(map, chunk_index, info));
    }

    // sets the color of the chunk's tiles, the ones without sprites show it and the ones with sprites are tinted by it
    IFB_ENGINE_API bool
    eng_map_chunk_set_color(
        const hnd_map       map,
        const u32           chunk_index,
        const map_color_u32 color) {

        assert(map != INVALID_HANDLE);
        return(map_chunk_set_color(map, chunk_index, color));
    }

    // removes the chunk with its tiles and walls, the chunks after it move down one index
    // walls it shares with another chunk move to that chunk
    IFB_ENGINE_API bool
    eng_map_chunk_destroy(
        const hnd_map map,
        const u32     chunk_index) {

        assert(map != INVALID_HANDLE);
        return(map_chunk_destroy(map, chunk_index));
    }

    IFB_ENGINE_API bool
    eng_map_get_navigable_bounds(
        const hnd_map map,
        u32&          row_min,
        u32&          col_min,
        u32&          row_max,
        u32&          col_max) {

        assert(map != INVALID_HANDLE);

        const bool did_get = map_get_navigable_bounds(map, row_min, col_min, row_max, col_max);
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_map_is_in_bounds(
        const cmpnt_map_coords& coords) {

        const bool is_in_bounds = map_is_in_bounds(coords);
        return(is_in_bounds);
    }

    IFB_ENGINE_API bool 
    eng_map_get_pos_from_coords(
        const cmpnt_map_coords& coords,
              cmpnt_position&   pos) {

        const bool did_get = map_get_pos_from_coords(coords, pos);
        return(did_get); 
    }

    IFB_ENGINE_API bool 
    eng_map_get_coords_from_pos(
        const cmpnt_position& pos,
        cmpnt_map_coords&     coords) {

        const bool did_get = map_get_coords_from_pos(pos, coords);
        return(did_get);
    }
    
    IFB_ENGINE_API bool
    eng_map_set_tile_sprite(
        const hnd_map h_map,
        const u32     row,
        const u32     col,
        const cchar*  atlas_name_cstr,
        const cchar*  sprite_name_cstr) {

        assert(h_map            != INVALID_HANDLE);
        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        const bool did_set = map_tile_set_sprite(h_map, row, col, atlas_name_cstr, sprite_name_cstr);
        return(did_set);
    }

    IFB_ENGINE_API bool
    eng_map_clear_tile_sprite(
        const hnd_map h_map,
        const u32     row,
        const u32     col) {

        assert(h_map != INVALID_HANDLE);

        const bool did_clear = map_tile_clear_sprite(h_map, row, col);
        return(did_clear);
    }

    IFB_ENGINE_API bool
    eng_map_get_tile_info(
        const hnd_map  h_map,
        const u32      row,
        const u32      col,
        map_tile_info& info) {

        assert(h_map != INVALID_HANDLE);

        const bool did_get = map_tile_get_info(h_map, row, col, info);
        return(did_get);
    }

    IFB_ENGINE_API bool
    eng_map_chunk_set_sprite(
        const hnd_map h_map,
        const u32     chunk_index,
        const cchar*  atlas_name_cstr,
        const cchar*  sprite_name_cstr) {

        assert(h_map            != INVALID_HANDLE);
        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        const bool did_set = map_chunk_set_sprite(h_map, chunk_index, atlas_name_cstr, sprite_name_cstr);
        return(did_set);
    }

    IFB_ENGINE_API bool
    eng_map_wall_set(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation,
        const map_color_u32          color) {

        assert(h_map != INVALID_HANDLE);

        const bool did_set = map_wall_set(h_map, row, col, orientation, color);
        return(did_set);
    }

    IFB_ENGINE_API bool
    eng_map_wall_set_sprite(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation,
        const cchar*                 atlas_name_cstr,
        const cchar*                 sprite_name_cstr) {

        assert(h_map            != INVALID_HANDLE);
        assert(atlas_name_cstr  != NULL);
        assert(sprite_name_cstr != NULL);

        const bool did_set = map_wall_set_sprite(h_map, row, col, orientation, atlas_name_cstr, sprite_name_cstr);
        return(did_set);
    }

    IFB_ENGINE_API bool
    eng_map_wall_clear(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation) {

        assert(h_map != INVALID_HANDLE);

        const bool did_clear = map_wall_clear(h_map, row, col, orientation);
        return(did_clear);
    }

    IFB_ENGINE_API bool
    eng_map_wall_is_placed(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation) {

        assert(h_map != INVALID_HANDLE);

        const bool is_placed = map_wall_is_placed(h_map, row, col, orientation);
        return(is_placed);
    }

    // what's on a tile edge, false if no chunk has the edge, so no wall can go there
    IFB_ENGINE_API bool
    eng_map_wall_get_info(
        const hnd_map                h_map,
        const u32                    row,
        const u32                    col,
        const map_wall_orientation_e orientation,
        map_wall_info&               info) {

        assert(h_map != INVALID_HANDLE);

        const bool did_get = map_wall_get_info(h_map, row, col, orientation, info);
        return(did_get);
    }
};
