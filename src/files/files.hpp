#ifndef IFB_FILES_HPP
#define IFB_FILES_HPP

#include "ifb.hpp"

namespace ifb {

    constexpr u32 FILE_HANDLE_INVALID = 0;
    constexpr u32 FILE_INDEX_INVALID  = 0xFFFFFFFF;

    using file_handle = u32;

    struct file_path {
        cchar8 cstr[IFB_CONFIG_FILE_PATH_SIZE];
    };

    struct file_manager {
        struct {
            byte* start;
            u32   size;
            u32   granularity;
        } memory;
        struct {
            file_handle*     handle_internal; 
            pfm_file_handle* handle_platform;
            u32*             io_length;
            u32*             cursor;
            file_path*       paths;
        } array;
        u32 file_count_max;
    };

    IFB_INTERNAL u32           file_manager_memory_requirement       (const u32 file_count_max);
    IFB_INTERNAL file_manager* file_manager_init                     (const u32 file_count_max, const u32 mem_size, void* mem_ptr);
    IFB_INTERNAL void          file_manager_startup                  (file_manager* mngr, const u32 mem_size, const u32 mem_granularity, void* mem_ptr);
    IFB_INTERNAL void          file_manager_shutdown                 (file_manager* mngr, const u32 mem_size, const u32 mem_granularity, void* mem_ptr);
    IFB_INTERNAL void          file_manager_assert_valid             (const file_manager* mngr);
    IFB_INTERNAL u32           file_manager_index_of_next_free       (const file_manager* mngr);
    IFB_INTERNAL u32           file_manager_index_of_internal_handle (const file_manager* mngr, const file_handle     hnd);
    IFB_INTERNAL u32           file_manager_index_of_platform_handle (const file_manager* mngr, const pfm_file_handle hnd);
    IFB_INTERNAL byte*         file_manager_get_buffer               (const file_manager* mngr, const u32 index);
    IFB_INTERNAL file_handle   file_manager_commit                   (file_manager* mngr, const pfm_file_config* cfg);
    IFB_INTERNAL file_handle   file_ro_create_new                    (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_ro_open_existing                 (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_ro_open_always                   (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_ro_overwrite                     (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_wo_create_new                    (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_wo_open_existing                 (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_wo_open_always                   (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_wo_overwrite                     (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_rw_create_new                    (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_rw_open_existing                 (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_rw_open_always                   (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL file_handle   file_rw_overwrite                     (file_manager* mngr, const cchar8*     path);
    IFB_INTERNAL void          file_close                            (file_manager* mngr, const file_handle hnd);
    IFB_INTERNAL u32           file_get_size                         (file_manager* mngr, const file_handle hnd);
    IFB_INTERNAL void          file_set_cursor                       (file_manager* mngr, const file_handle hnd, const u32 cursor);
    IFB_INTERNAL u32           file_read                             (file_manager* mngr, const file_handle hnd, const u32 buffer_size, byte* buffer_ptr);
    IFB_INTERNAL u32           file_write                            (file_manager* mngr, const file_handle hnd, const u32 buffer_size, byte* buffer_ptr);
};

#endif //IFB_FILES_HPP