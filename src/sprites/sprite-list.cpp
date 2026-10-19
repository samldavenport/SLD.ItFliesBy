#pragma once

#include "asset-mngr.cpp"
#include "asset.cpp"
#include "ifb-types.hpp"
#include "sprites.hpp"
#include "sprite-internal.hpp"

namespace ifb {

    IFB_INTERNAL sprite_list*
    sprite_list_create(
        const hnd_arena h_arena) {

        assert(h_arena != INVALID_HANDLE);

        auto list = (sprite_list*)arena_push(h_arena, sizeof(sprite_list)); 

        if (list) {
            list->first = NULL;
            list->last  = NULL;
        }

        return(list);
    }

    IFB_INTERNAL bool 
    sprite_list_add(
        sprite_list*    list,
        const cchar*    asset_img_name,
        const hnd_arena h_arena) {
   
        assert(list);
        assert(asset_img_name);
        assert(h_arena != INVALID_HANDLE);

        // if the sprite is already in the list, there's nothing to add
        const hnd_asset h_name = asset_name_cstr_to_handle(asset_img_name);
        if (h_name == INVALID_HANDLE) return(false);
        for (
            const sprite_list_node* curr = list->first;
                                    curr != NULL;
                                    curr = curr->next) {

            if (curr->h_img_asset == h_name) return(true);
        }

        // load the asset
        const hnd_asset h_asset = asset_mngr_load_image(asset_img_name);
        if (h_asset == INVALID_HANDLE) return(false);

        // the size is kept with the node, the image may be evicted before the atlas is built
        const asset_image* img = asset_mngr_get_image(h_asset);
        if (img == NULL) return(false);

        // allocate a node
        auto* node = (sprite_list_node*)arena_push(h_arena, sizeof(sprite_list_node));
        if (node == NULL) return(false);

        node->h_img_asset     = h_asset;
        node->img_dims.width  = (f32)img->width;
        node->img_dims.height = (f32)img->height;
        node->next            = NULL;

        // add the node to the list
        if (list->first == NULL) {
            assert(list->last == NULL);
            list->first = node;
            list->last  = node;
        } 
        else {
            sprite_list_node* last = list->last;
            assert(last->next == NULL);
            last->next = node;
            list->last = node;
        }

        return(true);
    }

    IFB_INTERNAL bool 
    sprite_list_remove(
        sprite_list*    list,
        const cchar*    asset_img_name_cstr) {

        assert(list);
        assert(asset_img_name_cstr);

        // get the handle
        const hnd_asset h_asset = asset_name_cstr_to_handle(asset_img_name_cstr); 
        if (h_asset == INVALID_HANDLE) return(false);

        // check the first node
        sprite_list_node* first = list->first;
        if (first == NULL) return(false);
        if (first->h_img_asset == h_asset) {
            if (first == list->last) {
                list->first = NULL;
                list->last  = NULL;
            } else {
                
                sprite_list_node* next = first->next;
                assert(next);

                list->first = next;
            }
            return(true);
        }

        // check the other nodes
        sprite_list_node* prev = first;
        for (
            sprite_list_node* curr = first->next;
                              curr != NULL;
                              curr = curr->next) {

            if (curr->h_img_asset == h_asset) {
                prev->next = curr->next;
                if (curr == list->last) {
                    list->last = prev;
                }
                return(true);
            }

            prev = curr;
        }
    
        return(false);
    }

    IFB_INTERNAL const sprite_list_node*
    sprite_list_get_first(
        const sprite_list* list) {

        assert(list);
        return(list->first);
    }
    
    IFB_INTERNAL const sprite_list_node*
    sprite_list_get_next(
        const sprite_list_node* node) {

        assert(node);
        return(node->next);
    }

    IFB_INTERNAL hnd_asset
    sprite_list_get_img_asset_hnd(
        const sprite_list_node* node) {

        assert(node);
        return(node->h_img_asset);
    }

    IFB_INTERNAL const dimensions_2d&
    sprite_list_get_img_dims(
        const sprite_list_node* node) {

        assert(node);
        return(node->img_dims);
    }

    IFB_INTERNAL hnd_sprite_atlas
    sprite_list_save_to_atlas(
        const sprite_list* list,
        const cchar*       atlas,
        const hnd_arena    h_arena) {

        //TODO(SLD): implement
        return(INVALID_HANDLE);
    }
};
