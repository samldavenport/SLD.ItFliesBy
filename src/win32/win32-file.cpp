#pragma once

#include "win32.hpp"

namespace ifb {


    IFB_WIN32_API_FUNC pfm_file_handle
    pfm_file_open(
        const pfm_file_config* cfg) {

        assert(cfg != NULL);

        LPSECURITY_ATTRIBUTES security        = NULL;
        HANDLE                template_handle = NULL;
        DWORD                 access          = 0;
        DWORD                 share           = 0;
        DWORD                 mode            = 0;
        DWORD                 flags           = FILE_ATTRIBUTE_NORMAL;

        // access flags
        const bool access_read     = bit_test(cfg->access_flags, pfm_file_access_flag_e_read);
        const bool access_write    = bit_test(cfg->access_flags, pfm_file_access_flag_e_write);
        const bool access_is_valid = (access_read | access_write); 
        assert(access_is_valid);
        
        if (access_read)  access |= GENERIC_READ;
        if (access_write) access |= GENERIC_WRITE;

        // share flags
        const bool share_read     = bit_test(cfg->share_flags, pfm_file_share_flag_e_read);
        const bool share_write    = bit_test(cfg->share_flags, pfm_file_share_flag_e_write);
        const bool share_delete   = bit_test(cfg->share_flags, pfm_file_share_flag_e_delete);
        const bool share_is_valid = (share_read | share_write | share_delete);
        assert(share_is_valid);

        if (share_read)   share |= FILE_SHARE_READ;
        if (share_write)  share |= FILE_SHARE_WRITE;
        if (share_delete) share |= FILE_SHARE_DELETE;

        // mode
        constexpr DWORD create_mode_array[] = {
            CREATE_NEW,    // pfm_file_mode_create_new 
            OPEN_EXISTING, // pfm_file_mode_open_existing
            OPEN_ALWAYS,   // pfm_file_mode_open_always
            CREATE_ALWAYS  // pfm_file_mode_overwrite_existing
        };
        mode = create_mode_array[cfg->mode];

        // overlapped / async
        flags |= FILE_FLAG_OVERLAPPED;

        // create the file handle
        const HANDLE file_handle = CreateFile(
            cfg->path,
            access,
            share,
            security,
            mode,
            flags,
            template_handle
        );
        assert(file_handle);
        return(file_handle);
    }

    IFB_WIN32_API_FUNC u32 
    pfm_file_size(
        const pfm_file_handle file) {

        LARGE_INTEGER size;
        assert(file && GetFileSizeEx(file, &size));

        return(size.QuadPart);
    }

    IFB_WIN32_API_FUNC void 
    pfm_file_close(
        const pfm_file_handle file) {

        assert(file && CloseHandle(file));
    }

    IFB_WIN32_API_FUNC u32
    pfm_file_read(
        const pfm_file_handle file,
        pfm_file_buffer*      buffer) {

        // check args and clear error
        assert(
            file           != NULL &&
            buffer         != NULL &&
            buffer->data   != NULL &&
            buffer->offset <  buffer->size
        );

        // set the pointer
        PLARGE_INTEGER file_pointer_new              = NULL;
        DWORD          file_pointer_move_method      = FILE_BEGIN;
        LARGE_INTEGER  file_pointer_distance_to_move;
        file_pointer_distance_to_move.QuadPart = buffer->cursor;

        const bool did_set_pointer = (bool)SetFilePointerEx(
            file,
            file_pointer_distance_to_move,
            file_pointer_new,
            file_pointer_move_method
        );
        assert(did_set_pointer);

        // do the read
        LPOVERLAPPED file_read_overlapped     = NULL;
        LPVOID       file_read_buffer         = (LPVOID)(buffer->data + buffer->offset);
        DWORD        file_read_size_requested = (buffer->size - buffer->offset); 
        DWORD        file_read_size_actual    = 0;

        const bool did_read = (bool)ReadFile(
            file,
            file_read_buffer,
            file_read_size_requested,
            &file_read_size_actual,
            file_read_overlapped
        );
        assert(did_read);

        // return the bytes read
        return(file_read_size_actual);
    }

    IFB_WIN32_API_FUNC u32
    pfm_file_write(
        const pfm_file_handle file,
        pfm_file_buffer*      buffer) {

        // check args and clear error
        assert(
            file           != NULL &&
            buffer         != NULL &&
            buffer->data   != NULL &&
            buffer->offset <  buffer->size
        );

        // set the pointer
        PLARGE_INTEGER file_pointer_new              = NULL;
        DWORD          file_pointer_move_method      = FILE_BEGIN;
        LARGE_INTEGER  file_pointer_distance_to_move;
        file_pointer_distance_to_move.QuadPart       = buffer->cursor;
        
        const bool did_set_pointer = SetFilePointerEx(
            file,
            file_pointer_distance_to_move,
            file_pointer_new,
            file_pointer_move_method
        );
        assert(did_set_pointer);

        // do the write
        LPOVERLAPPED file_write_overlapped     = NULL;
        LPVOID       file_write_buffer         = (LPVOID)(buffer->data + buffer->offset);
        DWORD        file_write_size_requested = (buffer->size - buffer->offset); 
        DWORD        file_write_size_actual    = 0;

        const bool did_write = ReadFile(
            file,
            file_write_buffer,
            file_write_size_requested,
            &file_write_size_actual,
            file_write_overlapped
        );
        assert(did_write);

        // return the bytes written
        return(file_write_size_actual);
    }

    IFB_WIN32_API_FUNC u32
    pfm_file_async_read(
        const pfm_file_handle file,
        pfm_file_buffer*      buffer) {

        return(0);
    }

    IFB_WIN32_API_FUNC u32
    pfm_file_async_write(
        const pfm_file_handle file,
        pfm_file_buffer*      buffer) {

        return(0);
    }
};