#ifndef CARTOGRAPHER_HPP
#define CARTOGRAPHER_HPP

#include "ifb.hpp"
#include "sld.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS 
    //--------------------------------------------------------------------
    
    static constexpr u32 CARTOGRAPHER_TILE_UNIT_SIZE = 0.2;

    //--------------------------------------------------------------------
    // ENUMS 
    //--------------------------------------------------------------------
   
    enum tile_type_e {
        tyle_type_e_normal = 0,
    };
    
    //--------------------------------------------------------------------
    // PRIMITIVE TYPES 
    //--------------------------------------------------------------------
   
    using tile_type = u32;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES 
    //--------------------------------------------------------------------
    
    struct cartographer;
    struct map;
    struct tile;
    struct wall;
    struct atlas;
    struct atlas_key;

    //--------------------------------------------------------------------
    // GLOBALS 
    //--------------------------------------------------------------------
    
    static cartographer* _cartographer;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL cartographer* cartographer_create               (void);     
    IFB_INTERNAL void          cartographer_startup              (memory& mem);
    IFB_INTERNAL void          cartographer_shutdown             (void);

    IFB_INTERNAL atlas*        cartographer_atlas_create         (const u32 image_count, const u32 image_buffer_size, const byte* image_buffer_ptr);
    IFB_INTERNAL atlas*        cartographer_atlas_destroy        (atlas* a);
    IFB_INTERNAL void          cartographer_atlas_add_key        (atlas* a, const u32 index, const u32 unit_x, const u32 unit_y, const u32 unit_length, const u32 unit_width);

    IFB_INTERNAL map*          cartographer_map_create           (void);    
    IFB_INTERNAL void          cartographer_map_destroy          (map* m);
    IFB_INTERNAL void          cartographer_map_add_tile         (map* m, tile* t);
    IFB_INTERNAL void          cartographer_map_remove_tile      (map* m, tile* t);
    IFB_INTERNAL void          cartographer_map_set_root_tile    (map* m, tile* t);

    IFB_INTERNAL wall*         cartographer_wall_create          (void);
    IFB_INTERNAL void          cartographer_wall_destroy         (wall* w);
    IFB_INTERNAL void          cartographer_wall_set_color       (wall* w, const color_rgba_u32 color);
    IFB_INTERNAL void          cartographer_wall_set_atlas_image (wall* w, const atlas* a, const u32 index);

    IFB_INTERNAL tile*         cartographer_tile_create          (void);
    IFB_INTERNAL void          cartographer_tile_destroy         (tile* t);
    IFB_INTERNAL void          cartogrpaher_tile_set_color       (tile* t, const color_rgba_u32 color); 
    IFB_INTERNAL void          cartographer_tile_set_atlas_image (tile* t, const atlas* a, const u32 index);
    IFB_INTERNAL void          cartographer_tile_add_tile_north  (tile* t, tile* n);
    IFB_INTERNAL void          cartographer_tile_add_tile_south  (tile* t, tile* s);
    IFB_INTERNAL void          cartographer_tile_add_tile_east   (tile* t, tile* e);
    IFB_INTERNAL void          cartographer_tile_add_tile_west   (tile* t, tile* w);
    IFB_INTERNAL void          cartographer_tile_add_wall_north  (tile* t, wall* n);
    IFB_INTERNAL void          cartographer_tile_add_wall_south  (tile* t, wall* s);
    IFB_INTERNAL void          cartographer_tile_add_wall_east   (tile* t, wall* e);
    IFB_INTERNAL void          cartographer_tile_add_wall_west   (tile* t, wall* w);
    IFB_INTERNAL void          cartographer_set_unit_dimensions  (tile* t, const u32 unit_length, const u32 unit_width);
    IFB_INTERNAL void          cartographer_set_unit_position    (tile* t, const u32 unit_pos_x,  const u32 unit_pos_y);
};
#endif //CARTOGRAPHER_HPP
