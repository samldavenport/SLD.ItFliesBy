#pragma once

#include "asset-dictionary.cpp"
#include "asset.hpp"
#include "file-rw.cpp"
#include "ifb-engine.hpp"
#include "ifb-types.hpp"
#include "memory.hpp"
#include "files.hpp"
#include "sld.hpp"

namespace ifb {
    
    //--------------------------------------------------------------------
    // HEADER FORMAT 
    //--------------------------------------------------------------------
    // - verification string | 10 bytes
    // - asset count (n)     | 4 bytes
    // - handles             | n * 4 bytes
    // - names               | n * IFB_CONFIG_ASSET_NAME_SIZE bytes
    // - indexes             | n * 12 bytes 
    //--------------------------------------------------------------------
    // TEXT FORMAT
    //--------------------------------------------------------------------
    // - size (n)            | 4 bytes
    // - text                | n bytes 
    //--------------------------------------------------------------------
    // IMAGE FORMAT
    //--------------------------------------------------------------------
    // - pixel width (w)     | 4 bytes
    // - pixel height (h)    | 4 bytes
    // - pixels (rgba)       | w * h * 4 bytes
    //--------------------------------------------------------------------

    //--------------------------------------------------------------------
    // CONSTANTS 
    //--------------------------------------------------------------------

    static constexpr cchar ASSET_DB_VERIFICATION_CSTR[] = "IFBASSETDB";

    //--------------------------------------------------------------------
    // TYPE DEFINITIONS 
    //--------------------------------------------------------------------

    struct asset_database {
        hnd_file          file_handle;
        u32               asset_count;
        hnd_asset*        array_handles;
        asset_name*       array_names;
        asset_index*      array_indexes;
    };
   
    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS 
    //--------------------------------------------------------------------
    
    inline bool      asset_database_find      (const asset_database* db, const hnd_asset h_asset,const asset_type_e type, asset_index& index);
    inline bool      asset_database_read_at   (const hnd_file h_file, const u32 offset, const u32 size, void* data);
    inline u32       asset_database_write_at  (const hnd_file h_file, const u32 offset, const u32 size, const void* data);
    inline void      asset_database_set_index (asset_database* db,    const u32 index,  const asset_dictionary_file_node* node, const asset_type_u32 type,const u32 offset,const u32 size);
    inline hnd_asset asset_database_handle_of (const cchar* name_cstr);
    inline bool      asset_database_index_of  (const asset_database* db, const hnd_asset hnd, asset_index& index);
    inline u32       asset_database_slot_of   (const asset_database* db, const hnd_asset hnd);
    inline bool      asset_database_find      (const asset_database* db, const hnd_asset h_asset, const asset_type_e type, asset_index& index);
 
    //--------------------------------------------------------------------
    // INTERNAL METHOD DECLARATIONS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL asset_database*
    asset_database_load(
        const cchar*    path,
        const hnd_arena h_arena) {

        assert(path);
        assert(h_arena != INVALID_HANDLE);

        // open the file, the database keeps it open to read assets later
        const hnd_file h_file = file_ro_open_existing(path);
        if (h_file == INVALID_HANDLE) {
            return(NULL);
        }

        // the file needs to be at least big enough for the verification string and count
        const u32 size_file         = file_get_size(h_file);
        const u32 size_verification = sizeof(ASSET_DB_VERIFICATION_CSTR) - 1;
        const u32 offset_count      = size_verification;
        const u32 offset_handles    = offset_count + sizeof(u32);
        if (size_file < offset_handles) {
            file_close(h_file);
            return(NULL);
        }

        // check the verification string
        cchar verification[sizeof(ASSET_DB_VERIFICATION_CSTR) - 1];
        bool is_valid = asset_database_read_at(h_file, 0, size_verification, verification);
        is_valid     &= (memcmp(verification, ASSET_DB_VERIFICATION_CSTR, size_verification) == 0);

        // read the number of assets
        u32 asset_count = 0;
        is_valid &= asset_database_read_at(h_file, offset_count, sizeof(u32), &asset_count);
        is_valid &= (asset_count != 0);
        if (!is_valid) {
            file_close(h_file);
            return(NULL);
        }

        // header offsets, 64 bits so a garbage count can't overflow
        const u64 offset_names   = offset_handles + ((u64)sizeof(hnd_asset)   * asset_count); 
        const u64 offset_indexes = offset_names   + ((u64)sizeof(asset_name)  * asset_count); 
        const u64 offset_data    = offset_indexes + ((u64)sizeof(asset_index) * asset_count); 
        if (offset_data > size_file) {
            file_close(h_file);
            return(NULL);
        }

        // save the position prior to our work here
        const u32 init_save = arena_save(h_arena);

        // create the database 
        auto db = ifb_arena_push_struct(h_arena, asset_database); 
        if (!db) {
            file_close(h_file);
            return(NULL);
        }
        db->file_handle = h_file;
        db->asset_count = asset_count;

        // allocate space for the arrays 
        db->array_handles = ifb_arena_push_array(h_arena, asset_count, hnd_asset);   
        db->array_names   = ifb_arena_push_array(h_arena, asset_count, asset_name);   
        db->array_indexes = ifb_arena_push_array(h_arena, asset_count, asset_index);   
        const bool did_alloc_arrays = (
            db->array_handles != NULL &&
            db->array_names   != NULL &&
            db->array_indexes != NULL
        );
        if (!did_alloc_arrays) {
            file_close(h_file);
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        // read the arrays
        is_valid &= asset_database_read_at(h_file, offset_handles,      sizeof(hnd_asset)   * asset_count, db->array_handles);
        is_valid &= asset_database_read_at(h_file, (u32)offset_names,   sizeof(asset_name)  * asset_count, db->array_names);
        is_valid &= asset_database_read_at(h_file, (u32)offset_indexes, sizeof(asset_index) * asset_count, db->array_indexes);

        // make sure every index points at data inside the file
        for (
            u32 index = 0;
                index < asset_count && is_valid;
              ++index) {

            const asset_index& idx = db->array_indexes[index];
            const u64          end = (u64)idx.offset + idx.size;

            is_valid &= (idx.type.val < asset_type_e_count);
            is_valid &= (idx.offset   >= offset_data);
            is_valid &= (end          <= size_file);
        }

        if (!is_valid) {
            file_close(h_file);
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        return(db);
    }

    IFB_INTERNAL asset_database* 
    asset_database_build(
        const cchar*            path, 
        const asset_dictionary* dict,
        const hnd_arena         h_arena) {

        assert(path);
        assert(dict);
        assert(h_arena != INVALID_HANDLE);

        // count the number of assets
        const u32 asset_count = asset_dictionary_count_assets(dict); 
        if (asset_count == 0) {
            return(NULL);
        } 

        // save the position prior to our work here
        const u32 init_save = arena_save(h_arena);

        // create the database 
        auto db = ifb_arena_push_struct(h_arena, asset_database); 
        if (!db) return(NULL);
        db->asset_count = asset_count;

        // allocate space for the arrays 
        db->array_handles = ifb_arena_push_array(h_arena, asset_count, hnd_asset);   
        db->array_names   = ifb_arena_push_array(h_arena, asset_count, asset_name);   
        db->array_indexes = ifb_arena_push_array(h_arena, asset_count, asset_index);   
        const bool did_alloc_arrays = (
            db->array_handles != NULL &&
            db->array_names   != NULL &&
            db->array_indexes != NULL
        );
        if (!did_alloc_arrays) {
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        // open the file, an old database there is replaced
        db->file_handle = file_rw_overwrite(path);
        if (db->file_handle == INVALID_HANDLE) {
            arena_revert(h_arena, init_save);
            return(NULL);
        }

        // header offsets, the verification string is written without its null terminator
        const u32 size_verification = sizeof(ASSET_DB_VERIFICATION_CSTR) - 1;
        const u32 offset_count      = size_verification;
        const u32 offset_handles    = offset_count   + sizeof(u32);
        const u32 offset_names      = offset_handles + (sizeof(hnd_asset)   * asset_count); 
        const u32 offset_indexes    = offset_names   + (sizeof(asset_name)  * asset_count); 
        const u32 offset_data       = offset_indexes + (sizeof(asset_index) * asset_count); 

        // the asset data goes after the header, we write the header once we know the indexes
        u32 offset_cur  = offset_data;
        u32 index_cur   = 0;

        // save our place
        const u32 write_asset_save = arena_save(h_arena);

        // write the text files to the database
        for (
            auto node = dict->text_files.first;
                 node != NULL;
                 node = node->next) {

            // open the next file, it should always exist
            const hnd_file h_file = file_ro_open_existing(node->path);
            assert(h_file != INVALID_HANDLE);

            // load the file contents 
            const text* txt = eng_text_file_load_to_arena(h_file, h_arena); 
            file_close(h_file);
            assert(txt != NULL);
     
            // write the size and text to the file
            const u32 offset_asset = offset_cur;
            offset_cur += asset_database_write_at(db->file_handle, offset_cur, sizeof(u32), &txt->size);
            if (txt->size > 0) {
                offset_cur += asset_database_write_at(db->file_handle, offset_cur, txt->size, txt->cstr);
            }

            // update the index
            asset_database_set_index(db, index_cur, node, asset_type_e_text, offset_asset, offset_cur - offset_asset);
            ++index_cur;

            // revert the arena
            arena_revert(h_arena, write_asset_save);
        }

        // write the image files to the database
        for (
            auto node = dict->image_files.first;
                 node != NULL;
                 node = node->next) {

            // open the next file, it should always exist
            const hnd_file h_file = file_ro_open_existing(node->path);
            assert(h_file != INVALID_HANDLE);

            // load the image, always rgba
            const image* img = eng_image_load_to_arena(h_file, h_arena);
            file_close(h_file);
            assert(img != NULL);

            // write the dimensions and pixels to the file
            const u32 offset_asset = offset_cur;
            offset_cur += asset_database_write_at(db->file_handle, offset_cur, sizeof(u32), &img->pixel_width);
            offset_cur += asset_database_write_at(db->file_handle, offset_cur, sizeof(u32), &img->pixel_height);
            if (img->data.size > 0) {
                offset_cur += asset_database_write_at(db->file_handle, offset_cur, img->data.size, img->data.vptr);
            }

            // update the index
            asset_database_set_index(db, index_cur, node, asset_type_e_image, offset_asset, offset_cur - offset_asset);
            ++index_cur;

            // revert the arena
            arena_revert(h_arena, write_asset_save);
        }
        assert(index_cur == asset_count);

        // now that the indexes are filled in, write the header
        (void)asset_database_write_at(db->file_handle, 0,              size_verification,                   ASSET_DB_VERIFICATION_CSTR);
        (void)asset_database_write_at(db->file_handle, offset_count,   sizeof(u32),                         &db->asset_count);
        (void)asset_database_write_at(db->file_handle, offset_handles, sizeof(hnd_asset)   * asset_count,   db->array_handles);
        (void)asset_database_write_at(db->file_handle, offset_names,   sizeof(asset_name)  * asset_count,   db->array_names);
        (void)asset_database_write_at(db->file_handle, offset_indexes, sizeof(asset_index) * asset_count,   db->array_indexes);

        return(db);
    }

    IFB_INTERNAL bool 
    asset_database_get_index(
        const asset_database* db,
        const cchar*          name_cstr,
        asset_index&          index) {

        assert(db);
        assert(name_cstr);

        asset_name name;
        name.init(name_cstr);

        const bool did_find = asset_database_index_of(db, name.to_handle(), index);
        return(did_find);
    }
    
    IFB_INTERNAL const asset_text*
    asset_database_load_text_by_handle_to_arena(
        const asset_database* db,
        const hnd_asset       h_asset,
        const hnd_arena       h_arena) {

        assert(db);
        assert(db->file_handle   != INVALID_HANDLE);
        assert(db->array_handles != NULL);
        assert(db->array_indexes != NULL);
        assert(db->array_names   != NULL);
        assert(db->asset_count   != 0);
        assert(h_arena           != INVALID_HANDLE);

        asset_index index;
        if (!asset_database_find(db, h_asset, asset_type_e_text, index)) {
            return(NULL);
        }

        // read the size, it has to agree with the index
        u32  size     = 0;
        bool is_valid = asset_database_read_at(db->file_handle, index.offset, sizeof(u32), &size);
        is_valid     &= (index.size == (u64)sizeof(u32) + size);
        if (!is_valid) {
            return(NULL);
        }

        // the text goes right after the struct, with a null terminator
        const u32 save      = arena_save(h_arena);
        const u32 size_push = sizeof(asset_text) + size + 1;
        auto      txt       = (asset_text*)arena_push(h_arena, size_push);
        if (!txt) {
            return(NULL);
        }
        zero_memory((void*)txt, size_push);

        txt->hnd  = h_asset;
        txt->size = size;
        txt->cstr = ((cchar*)txt) + sizeof(asset_text);

        // read the text
        if (size > 0) {
            const u32 offset_text = index.offset + sizeof(u32);
            if (!asset_database_read_at(db->file_handle, offset_text, size, txt->cstr)) {
                arena_revert(h_arena, save);
                return(NULL);
            }
        }

        return(txt);
    }

    IFB_INTERNAL const asset_text*
    asset_database_load_text_by_name_to_arena(
        const asset_database* db,
        const cchar*          name_cstr,
        const hnd_arena       h_arena) {

        assert(name_cstr);

        const hnd_asset h_asset = asset_database_handle_of(name_cstr);
        if (h_asset == INVALID_HANDLE) {
            return(NULL);
        }

        const asset_text* txt = asset_database_load_text_by_handle_to_arena(db, h_asset, h_arena);
        return(txt);
    }

    IFB_INTERNAL const asset_image*
    asset_database_load_image_by_handle_to_arena(
        const asset_database* db,
        const hnd_asset       h_asset,
        const hnd_arena       h_arena) {

        assert(db);
        assert(db->file_handle   != INVALID_HANDLE);
        assert(db->array_handles != NULL);
        assert(db->array_indexes != NULL);
        assert(db->array_names   != NULL);
        assert(db->asset_count   != 0);
        assert(h_arena           != INVALID_HANDLE);

        asset_index index;
        if (!asset_database_find(db, h_asset, asset_type_e_image, index)) {
            return(NULL);
        }

        // read the dimensions, the pixels have to agree with the index
        u32  dimensions[2] = { 0, 0 };
        bool is_valid      = asset_database_read_at(db->file_handle, index.offset, sizeof(dimensions), dimensions);

        const u32 width       = dimensions[0];
        const u32 height      = dimensions[1];
        const u64 size_pixels = (u64)width * height * sizeof(color_rgba_u32);
        is_valid &= (index.size == sizeof(dimensions) + size_pixels);
        if (!is_valid) {
            return(NULL);
        }

        // the pixels go right after the struct
        const u32 save      = arena_save(h_arena);
        const u32 size_push = sizeof(asset_image) + (u32)size_pixels;
        auto      img       = (asset_image*)arena_push(h_arena, size_push);
        if (!img) {
            return(NULL);
        }

        img->hnd    = h_asset;
        img->width  = width;
        img->height = height;
        img->pixels = (color_rgba_u32*)(((byte*)img) + sizeof(asset_image));

        // read the pixels
        if (size_pixels > 0) {
            const u32 offset_pixels = index.offset + sizeof(dimensions);
            if (!asset_database_read_at(db->file_handle, offset_pixels, (u32)size_pixels, img->pixels)) {
                arena_revert(h_arena, save);
                return(NULL);
            }
        }

        return(img);
    }

    IFB_INTERNAL const asset_image*
    asset_database_load_image_by_name_to_arena(
        const asset_database* db,
        const cchar*          name_cstr,
        const hnd_arena       h_arena) {

        assert(name_cstr);

        const hnd_asset h_asset = asset_database_handle_of(name_cstr);
        if (h_asset == INVALID_HANDLE) {
            return(NULL);
        }

        const asset_image* img = asset_database_load_image_by_handle_to_arena(db, h_asset, h_arena);
        return(img);
    }
    
    IFB_INTERNAL const asset_text*
    asset_database_load_text_by_handle_to_memory(
        const asset_database* db,
        const hnd_asset       h_asset,
        memory&               mem) {

        assert(db);
        assert(db->file_handle   != INVALID_HANDLE);
        assert(db->array_handles != NULL);
        assert(db->array_indexes != NULL);
        assert(db->array_names   != NULL);
        assert(db->asset_count   != 0);
        assert(mem.size          != 0);
        assert(mem.address       != 0);

        asset_index index;
        if (!asset_database_find(db, h_asset, asset_type_e_text, index)) {
            return(NULL);
        }

        // read the size, it has to agree with the index
        u32  size     = 0;
        bool is_valid = asset_database_read_at(db->file_handle, index.offset, sizeof(u32), &size);
        is_valid     &= (index.size == (u64)sizeof(u32) + size);
        if (!is_valid) {
            return(NULL);
        }

        // make sure the data can fit
        const u32 size_needed = sizeof(asset_text) + size + 1;
        if (mem.size < size_needed) {
            return(NULL);
        }

        zero_memory(mem);

        // the text goes right after the struct, with a null terminator
        auto txt = (asset_text*)mem.ptr;
        txt->hnd  = h_asset;
        txt->size = size;
        txt->cstr = ((cchar*)txt) + sizeof(asset_text);

        // read the text
        if (size > 0) {
            const u32 offset_text = index.offset + sizeof(u32);
            if (!asset_database_read_at(db->file_handle, offset_text, size, txt->cstr)) {
                return(NULL);
            }
        }

        return(txt);
    }

    IFB_INTERNAL const asset_text*
    asset_database_load_text_by_name_to_memory(
        const asset_database* db,
        const cchar*          name_cstr,
        memory&               mem) {

        assert(name_cstr);

        const hnd_asset h_asset = asset_database_handle_of(name_cstr);
        if (h_asset == INVALID_HANDLE) {
            return(NULL);
        }

        const asset_text* txt = asset_database_load_text_by_handle_to_memory(db, h_asset, mem);
        return(txt);
    }
    
    IFB_INTERNAL const asset_image*
    asset_database_load_image_by_handle_to_memory(
        const asset_database* db,
        const hnd_asset       h_asset,
        memory&         mem) {

        assert(db);
        assert(db->file_handle   != INVALID_HANDLE);
        assert(db->array_handles != NULL);
        assert(db->array_indexes != NULL);
        assert(db->array_names   != NULL);
        assert(db->asset_count   != 0);
        assert(mem.size          != 0);
        assert(mem.address       != 0);

        asset_index index;
        if (!asset_database_find(db, h_asset, asset_type_e_image, index)) {
            return(NULL);
        }

        // read the dimensions, the pixels have to agree with the index
        u32  dimensions[2] = { 0, 0 };
        bool is_valid      = asset_database_read_at(db->file_handle, index.offset, sizeof(dimensions), dimensions);

        const u32 width       = dimensions[0];
        const u32 height      = dimensions[1];
        const u64 size_pixels = (u64)width * height * sizeof(color_rgba_u32);
        is_valid &= (index.size == sizeof(dimensions) + size_pixels);
        if (!is_valid) {
            return(NULL);
        }

        // make sure the data can fit
        const u32 size_needed = sizeof(asset_image) + (u32)size_pixels;
        if (mem.size < size_needed) {
            return(NULL);
        }

        // the pixels go right after the struct
        auto img    = (asset_image*)mem.ptr;
        img->hnd    = h_asset;
        img->width  = width;
        img->height = height;
        img->pixels = (color_rgba_u32*)(((byte*)img) + sizeof(asset_image));

        // read the pixels
        if (size_pixels > 0) {
            const u32 offset_pixels = index.offset + sizeof(dimensions);
            if (!asset_database_read_at(db->file_handle, offset_pixels, (u32)size_pixels, img->pixels)) {
                return(NULL);
            }
        }

        return(img);
    }
    
    IFB_INTERNAL const asset_image*
    asset_database_load_image_by_name_to_memory(
        const asset_database* db,
        const cchar*          name_cstr,
        memory&               mem) {

        assert(name_cstr);

        const hnd_asset h_asset = asset_database_handle_of(name_cstr);
        if (h_asset == INVALID_HANDLE) {
            return(NULL);
        }

        const asset_image* img = asset_database_load_image_by_handle_to_memory(db, h_asset, mem);
        return(img);
    }
    
    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS 
    //--------------------------------------------------------------------
    
    inline bool
    asset_database_find(
        const asset_database* db,
        const hnd_asset       h_asset,
        const asset_type_e    type,
        asset_index&          index) {

        const bool is_valid = (
            h_asset != INVALID_HANDLE                   &&
            asset_database_index_of(db, h_asset, index) &&
            index.type == type
        );
        return(is_valid);
    }

    inline bool
    asset_database_read_at(
        const hnd_file h_file,
        const u32      offset,
        const u32      size,
        void*          data) {

        // like writing, the cursor doesn't move on its own
        file_set_cursor(h_file, offset);
        const u32  bytes_read = file_read_to_buffer(h_file, size, (byte*)data);
        const bool did_read   = (bytes_read == size);

        return(did_read);
    }

    inline u32
    asset_database_write_at(
        const hnd_file h_file,
        const u32      offset,
        const u32      size,
        const void*    data) {

        // file_write doesn't move the cursor, so we always set it first
        file_set_cursor(h_file, offset);
        const u32 bytes_written = file_write(h_file, size, (const byte*)data);
        assert(bytes_written == size);

        return(bytes_written);
    }

    inline void
    asset_database_set_index(
        asset_database*                   db,
        const u32                         index,
        const asset_dictionary_file_node* node,
        const asset_type_u32              type,
        const u32                         offset,
        const u32                         size) {

        assert(index < db->asset_count);

        // the handle is the hash of the stored name, the same as a lookup by name
        db->array_names  [index] = {};
        db->array_names  [index].init(node->name);
        db->array_handles[index] = db->array_names[index].to_handle();
        db->array_indexes[index].type   = type;
        db->array_indexes[index].offset = offset;
        db->array_indexes[index].size   = size;
    }

    inline hnd_asset
    asset_database_handle_of(
        const cchar* name_cstr) {

        // names in the dictionary are upper case, so lookups are too
        cchar     name_upper[IFB_CONFIG_FILE_PATH_SIZE];
        const u32 length = asset_dictionary_name_to_upper(name_cstr, name_upper);
        if (length == 0) return(INVALID_HANDLE);

        // the whole name is hashed, so the unused bytes need to be zero
        asset_name name = {};
        name.init(name_upper);

        const hnd_asset hnd = name.to_handle();
        return(hnd);
    }

    inline bool
    asset_database_index_of(
        const asset_database* db,
        const hnd_asset       hnd,
        asset_index&          index) {

        // the indexes are already loaded, so there's no need to go to the file
        const u32 slot = asset_database_slot_of(db, hnd);
        if (slot == INVALID_INDEX) return(false);

        index = db->array_indexes[slot];
        return(true);
    }

    // the position of the asset in the database arrays
    inline u32
    asset_database_slot_of(
        const asset_database* db,
        const hnd_asset       hnd) {

        assert(db);

        for (
            u32 slot = 0;
                slot < db->asset_count;
              ++slot) {

            if (hnd == db->array_handles[slot]) {
                return(slot);
            }
        }
        return(INVALID_INDEX);
    }
};
