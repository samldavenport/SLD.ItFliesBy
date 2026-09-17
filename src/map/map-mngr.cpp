#include "eng-internal.hpp"
#include "ifb-config.hpp"
#include "ifb-types.hpp"
#include "map-internal.hpp"
#include "eng-stack.cpp"
#include "map.hpp"
#include "memory-reservation.cpp"
#include "sld-strings.hpp"
#include "sld.hpp"

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

        const u32 size_hnds   = cfg.map_capacity * sizeof(map_handle);
        const u32 size_dims   = cfg.map_capacity * sizeof(map_dimensions);
        const u32 size_name   = cfg.map_capacity * sizeof(cstr_c16);
        const u32 size_chunks = cfg.map_capacity * sizeof(map_chunk_array);

        // allocate memory
        auto  tbl_map = _map_mngr->tbl_map; 
        auto& buffer  = _map_mngr->render_buffer;
        assert(tbl_map);
        tbl_map->hnd         =      (map_handle*)reservation_push_bytes(_map_mngr->res, size_hnds);
        tbl_map->dims        =  (map_dimensions*)reservation_push_bytes(_map_mngr->res, size_dims);
        tbl_map->name        =        (cstr_c16*)reservation_push_bytes(_map_mngr->res, size_name);
        tbl_map->chunk_array = (map_chunk_array*)reservation_push_bytes(_map_mngr->res, size_chunks);
        buffer.data.bytes    =            (byte*)reservation_push_bytes(_map_mngr->res, cfg.map_render_buffer_size); 
        buffer.data_size     = 0;
        assert(tbl_map->hnd);
        assert(tbl_map->dims);
        assert(tbl_map->name);
        assert(tbl_map->chunk_array);
        assert(buffer.data.bytes); 
        for (
            u32 map_index = 0;
                map_index < cfg.map_capacity;
              ++map_index) {

            tbl_map->hnd[map_index] = INVALID_HANDLE;
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
    
    IFB_INTERNAL const map_render_buffer&
    map_mngr_get_render_buffer(
        void) {

        assert(_map_mngr);
        const map_render_buffer& buffer = _map_mngr->render_buffer;
        assert(buffer.data.bytes != NULL);
        return(buffer); 
    } 
};
