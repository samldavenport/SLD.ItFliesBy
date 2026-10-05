#pragma once

#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "component.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include "sld.hpp"
#include "map.hpp"

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

    IFB_ENGINE_API void
    eng_map_destroy(
        const hnd_map map) {

        assert(map != INVALID_HANDLE);
    }

    IFB_ENGINE_API bool 
    eng_map_render(
        const hnd_map map_hnd) {

        const bool result = renderer_tile_set_map(map_hnd);
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
};
