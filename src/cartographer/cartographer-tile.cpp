#pragma once

#include "cartographer-internal.cpp"
#include "cartographer.hpp"
#include <cassert>

namespace ifb {

    IFB_INTERNAL tile*
    cartographer_tile_create(
        void) {

        tile* t = cartographer_memory_alloc_tile();
        assert(t);

        t->map         = NULL; 
        t->next        = NULL;
        t->prev        = NULL;
        t->tile_north  = NULL;
        t->tile_south  = NULL;
        t->tile_east   = NULL;
        t->tile_west   = NULL;
        t->wall_north  = NULL;
        t->wall_south  = NULL;
        t->wall_east   = NULL;
        t->wall_west   = NULL;
        t->color.hex   = 0xFFFFFFFF;
        t->unit_length = 0;
        t->unit_width  = 0;
        t->unit_pos_x  = 0;
        t->unit_pos_y  = 0;

        return(t);
    }

    IFB_INTERNAL void
    cartographer_tile_destroy(
        tile* t) {

        t = {0};

        cartographer_memory_free_tile(t);
    }

    IFB_INTERNAL void
    cartogrpaher_tile_set_color(
        tile*                t,
        const color_rgba_u32 color) {

        assert(t);

        t->color = color;
    }

    IFB_INTERNAL void
    cartographer_tile_set_atlas_image(
        tile*        t,
        const atlas* a,
        const u32    index) {

    }

    IFB_INTERNAL void
    cartographer_tile_add_tile_north(
        tile* t,
        tile* n) {

        assert(t != NULL);
        assert(n != NULL);

        t->tile_north = n;
        n->tile_south = t;
    }

    IFB_INTERNAL void
    cartographer_tile_add_tile_south(
        tile* t,
        tile* s) {

        assert(t);
        assert(s);

        t->tile_south = s;
        s->tile_north = t;
    }

    IFB_INTERNAL void
    cartographer_tile_add_tile_east(
        tile* t,
        tile* e) {

        assert(t);
        assert(e);
       
        t->tile_east = e;
        e->tile_west = t;
    }

    IFB_INTERNAL void
    cartographer_tile_add_tile_west(
        tile* t,
        tile* w) {

        assert(t);
        assert(w);

        t->tile_west = w;
        w->tile_east = t;
    }

    IFB_INTERNAL void
    cartographer_tile_add_wall_north(
        tile* t,
        wall* n) {

        assert(t);
        assert(n);

        t->wall_north = n;
        n->tile_south = t;
        n->dir        = wall_direction_e_north_south;
    }
    
    IFB_INTERNAL void
    cartographer_tile_add_wall_south(
        tile* t,
        wall* s) {
        
        assert(t);
        assert(s);

        t->wall_south = s;
        s->tile_north = t;
    }

    IFB_INTERNAL void
    cartographer_tile_add_wall_east(
        tile* t,
        wall* e) {
        
        assert(t);
        assert(e);

        t->wall_east = e;
        e->tile_west = t;
    }

    IFB_INTERNAL void
    cartographer_tile_add_wall_west(
        tile*       t,
         wall* w) {

        assert(t);
        assert(w);

        t->wall_west = w;
        w->tile_east = t;
    }
    
    IFB_INTERNAL void
    cartographer_set_unit_dimensions(
        tile* t,
        const u32 unit_length,
        const u32 unit_width) {

        assert(t);

        t->unit_length = unit_length;
        t->unit_width  = unit_width;
    }

    IFB_INTERNAL void
    cartographer_set_unit_position(
        tile* t,
        const u32 unit_pos_x,
        const u32 unit_pos_y) {

        assert(t);
        t->unit_pos_x = unit_pos_x;
        t->unit_pos_y = unit_pos_y;
    }
};
