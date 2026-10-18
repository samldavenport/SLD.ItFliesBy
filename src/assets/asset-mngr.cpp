#pragma once

#include "asset-database.cpp"
#include "asset.hpp"
#include "asset-internal.hpp"
#include "eng-internal.hpp"
#include "ifb-types.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the runtime database stays open, so assets can be loaded again after they're evicted
    // every asset in the database has a slot in array_blocks for where its data is in asset memory
    struct asset_mngr {
        reservation*        res;
        asset_dictionary*   dict;
        asset_memory*       mem;
        hnd_arena           h_arena;
        asset_database*     db;
        asset_memory_block* array_blocks;
    } static * _asset_mngr;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    inline addr asset_mngr_get_or_load    (const hnd_asset hnd, const asset_type_e type);
    inline u32  asset_mngr_size_in_memory (const asset_index& index);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL asset_mngr*
    asset_mngr_create(
        void) {

        auto mngr = global_alloc<asset_mngr>();
        assert(mngr);

        mngr->res          = NULL;
        mngr->dict         = NULL;
        mngr->mem          = asset_memory_create();
        mngr->h_arena      = INVALID_HANDLE;
        mngr->db           = NULL;
        mngr->array_blocks = NULL;
        assert(mngr->mem);

        _asset_mngr = mngr;
        return(_asset_mngr);
    }

    IFB_INTERNAL void
    asset_mngr_startup(
        reservation* res) {

        assert(_asset_mngr);
        assert(res);

        const auto& cfg = config_instance();

        _asset_mngr->res = res;
        asset_memory_init(_asset_mngr->mem, res);

        // open the runtime database
        // without it nothing can be loaded, but the engine can still run
        _asset_mngr->h_arena = arena_alloc();
        assert(_asset_mngr->h_arena != INVALID_HANDLE);

        _asset_mngr->db = asset_database_load(cfg.asset_database_path, _asset_mngr->h_arena);
        if (!_asset_mngr->db) {
            arena_free(_asset_mngr->h_arena);
            _asset_mngr->h_arena = INVALID_HANDLE;
            return;
        }

        const u32 asset_count = asset_database_get_asset_count(_asset_mngr->db);
        _asset_mngr->array_blocks = ifb_arena_push_array(_asset_mngr->h_arena, asset_count, asset_memory_block);
        assert(_asset_mngr->array_blocks);

        // nothing is loaded yet, an invalid block never resolves
        (void)memset(
            (void*)_asset_mngr->array_blocks,
            0xFF,
            asset_count * sizeof(asset_memory_block)
        );
    }

    // called once at the start of every frame
    IFB_INTERNAL void
    asset_mngr_update(
        void) {

        assert(_asset_mngr);

        const u64 frame = eng_context_get_frame_count();
        asset_memory_update(_asset_mngr->mem, frame);
    }

    IFB_INTERNAL void
    asset_mngr_shutdown(
        void) {

        assert(_asset_mngr);

        // every pin needs a matching unpin before shutdown
        assert(asset_memory_count_pinned(_asset_mngr->mem) == 0);

        if (_asset_mngr->db) {
            asset_database_close(_asset_mngr->db);
            _asset_mngr->db           = NULL;
            _asset_mngr->array_blocks = NULL;
        }

        if (_asset_mngr->h_arena != INVALID_HANDLE) {
            arena_free(_asset_mngr->h_arena);
            _asset_mngr->h_arena = INVALID_HANDLE;
        }
    }

    // loads the text into asset memory if it isn't already there
    // returns INVALID_HANDLE if there's no text by that name, or no room for it
    IFB_INTERNAL hnd_asset
    asset_mngr_load_text(
        const cchar* name) {

        assert(_asset_mngr);
        assert(name);

        const hnd_asset hnd = asset_database_handle_of(name);
        if (hnd == INVALID_HANDLE) return(INVALID_HANDLE);

        const addr data = asset_mngr_get_or_load(hnd, asset_type_e_text);
        if (!data) return(INVALID_HANDLE);

        return(hnd);
    }

    // loads the image into asset memory if it isn't already there
    // returns INVALID_HANDLE if there's no image by that name, or no room for it
    IFB_INTERNAL hnd_asset
    asset_mngr_load_image(
        const cchar* name) {

        assert(_asset_mngr);
        assert(name);

        const hnd_asset hnd = asset_database_handle_of(name);
        if (hnd == INVALID_HANDLE) return(INVALID_HANDLE);

        const addr data = asset_mngr_get_or_load(hnd, asset_type_e_image);
        if (!data) return(INVALID_HANDLE);

        return(hnd);
    }

    // if the text was evicted, it's loaded again
    // the pointer is only valid until the next asset load or asset_mngr_update, either can move the data
    IFB_INTERNAL const asset_text*
    asset_mngr_get_text(
        const hnd_asset hnd) {

        assert(_asset_mngr);

        const addr data = asset_mngr_get_or_load(hnd, asset_type_e_text);
        if (!data) return(NULL);

        // the text pointer was set when it was loaded, and the data may have moved since
        auto txt  = (asset_text*)data;
        txt->cstr = (cchar*)(data + sizeof(asset_text));
        return(txt);
    }

    // if the image was evicted, it's loaded again
    // the pointer is only valid until the next asset load or asset_mngr_update, either can move the data
    IFB_INTERNAL const asset_image*
    asset_mngr_get_image(
        const hnd_asset hnd) {

        assert(_asset_mngr);

        const addr data = asset_mngr_get_or_load(hnd, asset_type_e_image);
        if (!data) return(NULL);

        // the pixel pointer was set when it was loaded, and the data may have moved since
        auto img    = (asset_image*)data;
        img->pixels = (color_rgba_u32*)(data + sizeof(asset_image));
        return(img);
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // returns the asset's data in asset memory, loading it from the database if it isn't there
    inline addr
    asset_mngr_get_or_load(
        const hnd_asset    hnd,
        const asset_type_e type) {

        assert(_asset_mngr);

        if (!_asset_mngr->db || hnd == INVALID_HANDLE) return(NULL);

        const u32 slot = asset_database_slot_of(_asset_mngr->db, hnd);
        if (slot == INVALID_INDEX) return(NULL);

        const asset_index& index = _asset_mngr->db->array_indexes[slot];
        if (index.type.val != (u32)type) return(NULL);

        // it may already be in memory
        asset_memory_block& block         = _asset_mngr->array_blocks[slot];
        const addr          data_resident = asset_memory_get_data(_asset_mngr->mem, block, hnd);
        if (data_resident) return(data_resident);

        // make room for it
        const u32 size = asset_mngr_size_in_memory(index);
        if (size == 0) return(NULL);

        asset_memory_block block_new;
        if (!asset_memory_alloc(_asset_mngr->mem, hnd, size, block_new)) return(NULL);

        const addr data = asset_memory_get_data(_asset_mngr->mem, block_new, hnd);
        assert(data);

        // read it from the database
        memory mem_block;
        mem_block.size = size;
        mem_block.ptr  = (void*)data;

        bool did_load = false;
        switch (type) {

            case (asset_type_e_text): {
                did_load = asset_database_load_text_by_handle_to_memory(_asset_mngr->db, hnd, mem_block) != NULL; 
            } break;
            case (asset_type_e_image): {
                did_load = asset_database_load_image_by_handle_to_memory(_asset_mngr->db, hnd, mem_block) != NULL;
            } break;
            default: break;
        }

        if (!did_load) {
            asset_memory_free(_asset_mngr->mem, block_new);
            return(NULL);
        }

        block = block_new;
        return(data);
    }

    // the size of the asset once it's loaded, the struct followed by its data
    // returns 0 if the index doesn't make sense or the asset is too big
    inline u32
    asset_mngr_size_in_memory(
        const asset_index& index) {

        u64 size = 0;

        switch (index.type.val) {

            // text has its size in front of it in the file, and a null terminator after it in memory
            case asset_type_e_text: {
                if (index.size < sizeof(u32)) return(0);
                size = (u64)sizeof(asset_text) + (index.size - sizeof(u32)) + 1;
            } break;

            // images have their dimensions in front of the pixels in the file
            case asset_type_e_image: {
                if (index.size < (sizeof(u32) * 2)) return(0);
                size = (u64)sizeof(asset_image) + (index.size - (sizeof(u32) * 2));
            } break;

            default: return(0);
        }

        if (size > config::asset_size_max) return(0);
        return((u32)size);
    }
};
