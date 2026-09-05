#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "map-internal.hpp"
#include "eng-stack.cpp"
#include "map.hpp"

namespace ifb {
    
    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------
    
    
    //--------------------------------------------------------------------
    // INTERNAL METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL map_mngr*
    map_mngr_create(
        void) {

        _map_mngr = global_alloc<map_mngr>();

        assert(_map_mngr);

        _map_mngr->tbl_tiles = global_alloc<map_tile_table>();
        _map_mngr->tbl_map   = global_alloc<map_table>();
        assert(_map_mngr->tbl_tiles != NULL); 
        assert(_map_mngr->tbl_map   != NULL); 

        return(_map_mngr);
    }

    IFB_INTERNAL void
    map_mngr_startup(
        memory& res) {

        assert(
            _map_mngr  != NULL && 
            res.size    != 0    &&
            res.address != 0
        );

        const auto& cfg = config_instance();
        
        // initialize memory
        memory commit;
        commit.size = res.size;
        commit.ptr  = pfm_memory_commit(res.ptr, 0, res.size); 
        assert(commit.ptr);
        _map_mngr->mem_stack.init(commit); 

        // cache the tables
        auto tbl_map   = _map_mngr->tbl_map; 
        auto tbl_tiles = _map_mngr->tbl_tiles; 
        assert(tbl_map);
        assert(tbl_tiles);

        // calculate max tiles
        const u32 tile_count_max = cfg.map_capacity * cfg.tile_capacity; 
       
        // allocate map table memory
        tbl_map->map_hnd      = _map_mngr->mem_stack.push_struct<map_handle> (cfg.map_capacity);
        tbl_map->count_rows  = _map_mngr->mem_stack.push_struct<u32>        (cfg.map_capacity);
        tbl_map->count_cols  = _map_mngr->mem_stack.push_struct<u32>        (cfg.map_capacity);
        tbl_map->offset_row  = _map_mngr->mem_stack.push_struct<s32>        (cfg.map_capacity);
        tbl_map->offset_col  = _map_mngr->mem_stack.push_struct<s32>        (cfg.map_capacity);
        tbl_map->name        = _map_mngr->mem_stack.push_struct<map_name>   (cfg.map_capacity);
        assert(tbl_map->map_hnd     != NULL); 
        assert(tbl_map->count_rows != NULL); 
        assert(tbl_map->count_cols != NULL); 
        assert(tbl_map->offset_row != NULL); 
        assert(tbl_map->offset_col != NULL); 
    
        // allocate tile table memory
        tbl_tiles->color   = _map_mngr->mem_stack.push_struct<color_rgba_u32>(tile_count_max);  
        assert(tbl_tiles->color   != NULL);  

        // set the map id array to invalid
        memset(tbl_map->map_hnd, 0xFF, sizeof(map_handle) * cfg.map_capacity);
   
        // store capacities
        _map_mngr->map_capacity   = cfg.map_capacity;
        _map_mngr->tiles_per_map  = cfg.tile_capacity;
        _map_mngr->tile_unit_size = cfg.tile_unit_size;
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
    
    IFB_INTERNAL f32
    map_mngr_get_tile_unit_size(
        void) {

        return(_map_mngr->tile_unit_size);
    }
};
