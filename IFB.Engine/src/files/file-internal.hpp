#ifndef FILE_INTERNAL_HPP
#define FILE_INTERNAL_HPP

#include "ifb-files.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static file_mngr* _file_mngr;

    //--------------------------------------------------------------------
    // METHODS 
    //--------------------------------------------------------------------

    IFB_INTERNAL file_mngr*    file_mngr_create                   (void);
    IFB_INTERNAL void          file_mngr_startup                  (reservation* res);
    IFB_INTERNAL void          file_mngr_shutdown                 (void);
    IFB_INTERNAL void          file_mngr_assert_valid             (void);
    IFB_INTERNAL u32           file_mngr_index_of_next_free       (void);
    IFB_INTERNAL u32           file_mngr_index_of_internal_handle (const hnd_file     hnd);
    IFB_INTERNAL u32           file_mngr_index_of_platform_handle (const pfm_hnd_file hnd);
    IFB_INTERNAL byte*         file_mngr_get_buffer               (const u32 index);
    IFB_INTERNAL hnd_file      file_mngr_commit                   (const pfm_file_config* cfg);
    IFB_INTERNAL hnd_file      file_ro_create_new                 (const cchar*     path);
    IFB_INTERNAL hnd_file      file_ro_open_existing              (const cchar*     path);
    IFB_INTERNAL hnd_file      file_ro_open_always                (const cchar*     path);
    IFB_INTERNAL hnd_file      file_ro_overwrite                  (const cchar*     path);
    IFB_INTERNAL hnd_file      file_wo_create_new                 (const cchar*     path);
    IFB_INTERNAL hnd_file      file_wo_open_existing              (const cchar*     path);
    IFB_INTERNAL hnd_file      file_wo_open_always                (const cchar*     path);
    IFB_INTERNAL hnd_file      file_wo_overwrite                  (const cchar*     path);
    IFB_INTERNAL hnd_file      file_rw_create_new                 (const cchar*     path);
    IFB_INTERNAL hnd_file      file_rw_open_existing              (const cchar*     path);
    IFB_INTERNAL hnd_file      file_rw_open_always                (const cchar*     path);
    IFB_INTERNAL hnd_file      file_rw_overwrite                  (const cchar*     path);
    IFB_INTERNAL void          file_close                         (const hnd_file hnd);
    IFB_INTERNAL u32           file_get_size                      (const hnd_file hnd);
    IFB_INTERNAL void          file_set_cursor                    (const hnd_file hnd, const u32 cursor);
    IFB_INTERNAL const cchar*  file_read                          (const hnd_file hnd, const u32 buffer_size);
    IFB_INTERNAL u32           file_read_to_buffer                (const hnd_file hnd, const u32 buffer_size, byte* buffer_ptr);
    IFB_INTERNAL u32           file_write                         (const hnd_file hnd, const u32 buffer_size, const byte* buffer_ptr);
};

#endif //FILE_INTERNAL_HPP
