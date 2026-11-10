# Files

The file system (`IFB.Engine/src/files`, `eng-file.hpp`) opens, reads, and writes files through the platform layer, and refers to open files with `hnd_file` handles. It also wraps the native open and save dialogs.

## Opening Files

Each open function combines an **access** prefix with an **open mode**:

| Prefix | Access |
| --- | --- |
| `eng_file_ro_*` | Read only |
| `eng_file_wo_*` | Write only |
| `eng_file_rw_*` | Read and write |

| Suffix | If the file exists | If it doesn't |
| --- | --- | --- |
| `_create_new` | Fails | Creates it |
| `_open_existing` | Opens it | Fails |
| `_open_always` | Opens it | Creates it |
| `_overwrite` | Truncates it | Creates it |

The modes map to the Win32 `CreateFile` creation dispositions (`win32-file.cpp`).

```cpp
hnd_file h_file = eng_file_ro_open_existing("asset-dictionary.json");
if (h_file != 0) {   // a failed open returns 0, not INVALID_HANDLE
    const u32    size = eng_file_get_size(h_file);
    const cchar* data = eng_file_read(h_file);
    // ...
    eng_file_close(h_file);
}
```

Relative paths are resolved from the working directory, which is `build/<preset>/bin` when you run the game normally.

## Handles

A file's handle is the hash of its path, so opening the same path twice gives the same handle value. The file manager keeps a table of up to `IFB_CONFIG_FILE_COUNT` (64) open files, with each one's platform handle, cursor, and path.

Opening a file returns `FILE_HANDLE_INVALID` (`0`) if the file can't be opened or the table is full. Note that this is different from `INVALID_HANDLE` (there's a TODO in `eng-file.hpp` to make them the same).

## Reading and Writing

| Function | Description |
| --- | --- |
| `eng_file_get_size` | The file's size in bytes |
| `eng_file_set_cursor` | Set the byte offset the next read or write starts from |
| `eng_file_read` | Read from the cursor into the file's buffer, and return a pointer to it |
| `eng_file_read_to_buffer` | Read up to `buffer_size` bytes from the cursor into your buffer, and return the number of bytes read |
| `eng_file_read_to_arena` | Read the whole file into a new block on an arena, and return it as a `memory` (empty on failure) |
| `eng_file_write` | Write `buffer_size` bytes from `buffer_ptr` at the cursor, and return the number of bytes written |
| `eng_file_close` | Close the file and free its table slot |

Things to know:

- **The cursor doesn't move on its own.** Reads and writes always start at the cursor, so set it before each one when reading or writing a file in pieces.
- **`eng_file_read` reads at most `IFB_CONFIG_FILE_GRANULARITY` (64 KB).** Each table slot has its own 64 KB buffer, and that's where the data goes. The result is null-terminated, so a file of 64 KB or more loses its last byte to the terminator. The pointer is only valid until the next read of the same file.
- `eng_image_load_to_arena` and `eng_image_load_to_memory` read through `eng_file_read`, so they have the same limit.
- **For files of 64 KB or more, or binary files, use `eng_file_read_to_buffer` or `eng_file_read_to_arena`.** They read straight into your memory, with no size limit and no null terminator. The asset database uses the same internal read for asset files.
- `eng_file_read_to_arena` always reads from the start of the file and leaves the cursor at 0. It returns an empty `memory` (size 0, null pointer) if the file is empty, the arena doesn't have room, or the read comes up short, and gives back the arena space in that last case.

```cpp
hnd_file h_font = eng_file_ro_open_existing("PressStart2P-Regular.ttf");
if (h_font != 0) {
    const memory font = eng_file_read_to_arena(h_font, h_arena);
    eng_file_close(h_font);
    if (font.size != 0) {
        // font.bytes holds the whole file
    }
}
```

## Threads

Any thread can open or close a file, because the file manager has a lock around its table. It isn't held while the operating system opens the file, so a slow open doesn't hold up other threads.

Reads, writes, and the cursor aren't locked, so only one thread can use an open file at a time. Don't open a path that's already open from another thread, because both opens get the same [handle](#handles). See the [thread rules](jobs.md#thread-rules).

File dialogs are main thread only.

## File Dialogs

```cpp
const cchar* path = eng_file_dialog_open("*.json", "Asset Dictionary");
if (path) {
    // the user picked a file
}
```

| Function | Description |
| --- | --- |
| `eng_file_dialog_open` | Show the native open dialog |
| `eng_file_dialog_save` | Show the native save dialog |

Both take one file type filter (like `*.ifbatlas`) and a description, and return the selected path, or `NULL` if the user cancelled. The path is in a static buffer of 256 characters, which the next call to the same dialog overwrites, so copy it if you need to keep it.

The GUI uses these to pick asset dictionaries, asset databases, and sprite atlases.

## Text and Images

Helpers that load whole files into memory:

| Function | Description |
| --- | --- |
| `eng_text_file_size` / `eng_text_file_load_to_arena` / `_to_memory` | Load a text file as a `text` (pointer and size) |
| `eng_image_size` / `eng_image_load_to_arena` / `_to_memory` | Decode an image file with stb_image into an `image` |

Most game content should go through the [asset database](assets.md) instead of loading files directly.

## Memory

The file manager's tables are allocated on the global stack, and its read buffers (`IFB_CONFIG_FILE_COUNT × IFB_CONFIG_FILE_GRANULARITY`, so 4 MB) come from the `files` reservation in the memory map.

## Configuration

| Setting | Default | Description |
| --- | --- | --- |
| `IFB_CONFIG_FILE_COUNT` | 64 | Maximum number of open files |
| `IFB_CONFIG_FILE_GRANULARITY` | 64 KB | Size of each file's read buffer |
| `IFB_CONFIG_FILE_PATH_SIZE` | 256 | Maximum path length |
