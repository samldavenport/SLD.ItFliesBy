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

        _map_mngr->memory    = global_alloc<map_memory>();
        assert(_map_mngr->memory    != NULL); 

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
        map_memory* map_mem = _map_mngr->memory;
        map_mem->size = res.size;
        map_mem->ptr  = pfm_memory_commit(res.ptr, 0, res.size); 
        assert(map_mem->ptr != NULL);

        // cache the tables

        // calculate max tiles
        const u32 tile_count_max = cfg.map_capacity * cfg.tile_capacity; 
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
