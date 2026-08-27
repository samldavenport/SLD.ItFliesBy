#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "tile.hpp"
#include "eng-stack.cpp"

namespace ifb {
    
    IFB_INTERNAL tile_mngr*
    tile_mngr_create(
        void) {

        _tile_mngr = global_alloc<tile_mngr>();

        assert(_tile_mngr);

        _tile_mngr->tbl_tiles          = global_alloc<tile_table>();
        _tile_mngr->tbl_map            = global_alloc<tile_map_table>();
        assert(_tile_mngr->tbl_tiles          != NULL); 
        assert(_tile_mngr->tbl_map            != NULL); 

        return(_tile_mngr);
    }

    IFB_INTERNAL void
    tile_mngr_startup(
        memory& res) {

        assert(
            _tile_mngr  != NULL && 
            res.size    != 0    &&
            res.address != 0
        );

        const auto& cfg = config_instance();
        
        // initialize memory
        memory commit;
        commit.size = res.size;
        commit.ptr  = pfm_memory_commit(res.ptr, 0, res.size); 
        assert(commit.ptr);
        _tile_mngr->mem_stack.init(commit); 
       

        // cache the tables
        auto tbl_map   = _tile_mngr->tbl_map; 
        auto tbl_tiles = _tile_mngr->tbl_tiles; 
        assert(tbl_map);
        assert(tbl_tiles);

        // calculate max tiles
        const u32 tile_count_max = cfg.tile_map_capacity * cfg.tile_capacity; 
       
        // allocate map table memory
        tbl_map->capacity         = cfg.tile_map_capacity; 
        tbl_map->count            = 0; 
        tbl_map->data.map_id      = _tile_mngr->mem_stack.push_struct<tile_map_id_u32>(cfg.tile_map_capacity);
        tbl_map->data.tile_width  = _tile_mngr->mem_stack.push_struct<f32>(cfg.tile_map_capacity);
        tbl_map->data.tile_height = _tile_mngr->mem_stack.push_struct<f32>(cfg.tile_map_capacity);
        tbl_map->data.count_rows  = _tile_mngr->mem_stack.push_struct<u32>(cfg.tile_map_capacity);
        tbl_map->data.count_cols  = _tile_mngr->mem_stack.push_struct<u32>(cfg.tile_map_capacity);
        assert(tbl_map->data.map_id      != NULL); 
        assert(tbl_map->data.tile_width  != NULL); 
        assert(tbl_map->data.tile_height != NULL); 
        assert(tbl_map->data.count_rows  != NULL); 
        assert(tbl_map->data.count_cols  != NULL); 
    
        // allocate tile table memory
        tbl_tiles->capacity     = cfg.tile_capacity;  
        tbl_tiles->count        = 0;  
        tbl_tiles->map_id_array = _tile_mngr->mem_stack.push_struct<tile_map_id_u32>(cfg.tile_map_capacity);  
        tbl_tiles->data.row     = _tile_mngr->mem_stack.push_struct<u16            >(tile_count_max);  
        tbl_tiles->data.col     = _tile_mngr->mem_stack.push_struct<u16            >(tile_count_max);  
        tbl_tiles->data.color   = _tile_mngr->mem_stack.push_struct<color_rgba_u32 >(tile_count_max);  
        tbl_tiles->data.flags   = _tile_mngr->mem_stack.push_struct<tile_flags_u32 >(tile_count_max);
        assert(tbl_tiles->map_id_array != NULL); 
        assert(tbl_tiles->data.row     != NULL);    
        assert(tbl_tiles->data.col     != NULL);    
        assert(tbl_tiles->data.color   != NULL);  
        assert(tbl_tiles->data.flags   != NULL);  
    }

    IFB_INTERNAL void
    tile_mngr_shutdown(
        void) {

        //TODO(SLD)
    }

};
