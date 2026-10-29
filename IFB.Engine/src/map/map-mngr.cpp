#include "component-tables.cpp"
#include "eng-internal.hpp"
#include "entity-lookup.cpp"
#include "ifb-collections.hpp"
#include "ifb-config.hpp"
#include "ifb-entities.hpp"
#include "ifb.hpp"
#include "map-internal.hpp"
#include "eng-stack.cpp"
#include "ifb-map.hpp"
#include "memory-arena.cpp"
#include "memory-reservation.cpp"
#include "sld-strings.hpp"
#include "sld.hpp"
#include "memory-internal.hpp"

namespace ifb {
    
    //--------------------------------------------------------------------
    // INTERNAL METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL map_mngr*
    map_mngr_create(
        void) {

        _map_mngr = global_alloc<map_mngr>();
        assert(_map_mngr);

        _map_mngr->tbl_map       = global_alloc<map_table>();
        assert(_map_mngr->tbl_map);

        return(_map_mngr);
    }

    IFB_INTERNAL void
    map_mngr_startup(
        reservation* res) {

        assert(_map_mngr != NULL);
        assert(res       != NULL);

        _map_mngr->res = res;

        const auto& cfg = config_instance();

        const u32 size_hnds   = cfg.map_capacity * sizeof(hnd_map);
        const u32 size_dims   = cfg.map_capacity * sizeof(map_dimensions);
        const u32 size_name   = cfg.map_capacity * sizeof(cstr_c16);
        const u32 size_chunks = cfg.map_capacity * sizeof(map_chunk_array);
        const u32 size_floor  = cfg.map_capacity * sizeof(map_floor_array);
        const u32 size_walls  = cfg.map_capacity * sizeof(map_wall_array);
        const u32 size_origin = cfg.map_capacity * sizeof(vec3);
        const u32 size_wall_tiles = cfg.map_wall_capacity * sizeof(map_wall_tile);

        // allocate memory
        auto  tbl_map = _map_mngr->tbl_map;
        assert(tbl_map);
        tbl_map->hnd         =      (hnd_map*)reservation_push_bytes(_map_mngr->res, size_hnds);
        tbl_map->dims        =  (map_dimensions*)reservation_push_bytes(_map_mngr->res, size_dims);
        tbl_map->name        =        (cstr_c16*)reservation_push_bytes(_map_mngr->res, size_name);
        tbl_map->chunk_array = (map_chunk_array*)reservation_push_bytes(_map_mngr->res, size_chunks);
        tbl_map->floor_array = (map_floor_array*)reservation_push_bytes(_map_mngr->res, size_floor);
        tbl_map->row_wall_array    = (map_wall_array*)reservation_push_bytes(_map_mngr->res, size_walls);
        tbl_map->column_wall_array = (map_wall_array*)reservation_push_bytes(_map_mngr->res, size_walls);
        tbl_map->origin      =            (vec3*)reservation_push_bytes(_map_mngr->res, size_origin);
        assert(tbl_map->hnd);
        assert(tbl_map->dims);
        assert(tbl_map->name);
        assert(tbl_map->chunk_array);
        assert(tbl_map->floor_array);
        assert(tbl_map->row_wall_array);
        assert(tbl_map->column_wall_array);
        assert(tbl_map->origin);
        for (
            u32 map_index = 0;
                map_index < cfg.map_capacity;
              ++map_index) {

            tbl_map->hnd[map_index] = INVALID_HANDLE;

            // each map gets its own pool of wall tiles per orientation
            map_wall_array& row_walls    = tbl_map->row_wall_array    [map_index];
            map_wall_array& column_walls = tbl_map->column_wall_array [map_index];
            row_walls.count    = 0;
            row_walls.tiles    = (map_wall_tile*)reservation_push_bytes(_map_mngr->res, size_wall_tiles);
            column_walls.count = 0;
            column_walls.tiles = (map_wall_tile*)reservation_push_bytes(_map_mngr->res, size_wall_tiles);
            assert(row_walls.tiles);
            assert(column_walls.tiles);
        }
    }

    IFB_INTERNAL void
    map_mngr_shutdown(
        void) {

        //TODO(SLD)
    }

    IFB_INTERNAL const map_color_table&
    map_mngr_get_color_table(
        void) {

        static map_color_table color_tbl;
        static bool init = false;
        if (!init) {

            color_tbl.red_light    = color_rgba_u32(0xFB4934FF);
            color_tbl.red_dark     = color_rgba_u32(0xCC241DFF);
            color_tbl.orange_light = color_rgba_u32(0xFE8019FF);
            color_tbl.orange_dark  = color_rgba_u32(0xD65D0EFF);
            color_tbl.yellow_light = color_rgba_u32(0xFABD2FFF);
            color_tbl.yellow_dark  = color_rgba_u32(0xD79921FF);
            color_tbl.green_light  = color_rgba_u32(0xB8BB26FF);
            color_tbl.green_dark   = color_rgba_u32(0x98971AFF);
            color_tbl.aqua_light   = color_rgba_u32(0x8EC07CFF);
            color_tbl.aqua_dark    = color_rgba_u32(0x689D6AFF);
            color_tbl.blue_light   = color_rgba_u32(0x83A598FF);
            color_tbl.blue_dark    = color_rgba_u32(0x458588FF);
            color_tbl.purple_light = color_rgba_u32(0xD3869BFF);
            color_tbl.purple_dark  = color_rgba_u32(0xB16286FF);
            color_tbl.gray_light   = color_rgba_u32(0xA89984FF);
            color_tbl.gray_dark    = color_rgba_u32(0x928374FF);

            init = true;
        }
        return(color_tbl);
    }
    
    // the number of active maps, use with map_mngr_get_map to iterate
    IFB_INTERNAL u32
    map_mngr_get_count(
        void) {

        assert(_map_mngr);
        assert(_map_mngr->tbl_map);

        const auto  tbl = _map_mngr->tbl_map;
        const auto& cfg = config_instance();

        u32 count = 0;
        for (
            u32 map_index = 0;
                map_index < cfg.map_capacity;
              ++map_index) {

            if (tbl->hnd[map_index] != INVALID_HANDLE) {
                ++count;
            }
        }
        return(count);
    }

    // the handle of the nth active map, skipping empty slots in the table
    // INVALID_HANDLE if there aren't that many maps
    IFB_INTERNAL hnd_map
    map_mngr_get_map(
        const u32 index) {

        assert(_map_mngr);
        assert(_map_mngr->tbl_map);

        const auto  tbl = _map_mngr->tbl_map;
        const auto& cfg = config_instance();

        u32 count = 0;
        for (
            u32 map_index = 0;
                map_index < cfg.map_capacity;
              ++map_index) {

            const hnd_map h_map = tbl->hnd[map_index];
            if (h_map == INVALID_HANDLE) {
                continue;
            }
            if (count == index) {
                return(h_map);
            }
            ++count;
        }
        return(INVALID_HANDLE);
    }
};
