#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "map.hpp"
#include "eng-stack.cpp"

namespace ifb {
    
    IFB_INTERNAL map_mngr*
    map_mngr_create(
        void) {

        _map_mngr = global_alloc<map_mngr>();

        assert(_map_mngr);

        _map_mngr->tbl_tiles = global_alloc<tile_table>();
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
        tbl_map->map_id      = _map_mngr->mem_stack.push_struct<map_id_u32> (cfg.map_capacity);
        tbl_map->count_rows  = _map_mngr->mem_stack.push_struct<u32>        (cfg.map_capacity);
        tbl_map->count_cols  = _map_mngr->mem_stack.push_struct<u32>        (cfg.map_capacity);
        tbl_map->offset_row  = _map_mngr->mem_stack.push_struct<s32>        (cfg.map_capacity);
        tbl_map->offset_col  = _map_mngr->mem_stack.push_struct<s32>        (cfg.map_capacity);
        tbl_map->name        = _map_mngr->mem_stack.push_struct<map_name>   (cfg.map_capacity);
        assert(tbl_map->map_id     != NULL); 
        assert(tbl_map->count_rows != NULL); 
        assert(tbl_map->count_cols != NULL); 
        assert(tbl_map->offset_row != NULL); 
        assert(tbl_map->offset_col != NULL); 
    
        // allocate tile table memory
        tbl_tiles->color   = _map_mngr->mem_stack.push_struct<color_rgba_u32>(tile_count_max);  
        tbl_tiles->flags   = _map_mngr->mem_stack.push_struct<tile_flags_u32>(tile_count_max);
        assert(tbl_tiles->color   != NULL);  
        assert(tbl_tiles->flags   != NULL);  

        // set the map id array to invalid
        memset(tbl_map->map_id, 0xFF, sizeof(map_id_u32) * cfg.map_capacity);
   
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

    IFB_INTERNAL f32
    map_mngr_get_tile_unit_size(
        void) {

        return(_map_mngr->tile_unit_size);
    }
};
