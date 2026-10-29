# Platform

The platform layer is everything the engine needs from the operating system: the window, the OpenGL context, memory, files, dialogs, monitors, and time. Its interface is in `IFB.Core/include/ifb-platform.hpp` (`pfm_*` functions), and the only implementation is Win32 (`IFB.Engine/src/win32`).

The rest of the engine only calls `pfm_*` functions, so another platform would need its own implementation of `ifb-platform.hpp` and wouldn't touch the other systems. Win32 code is still used directly in a few places outside the platform layer: `game-win32-main.cpp` (the entry point and `VirtualAlloc`), and the Win32 ImGui backend.

## Headers

| Header | Contents |
| --- | --- |
| `IFB.Core/include/ifb-platform.hpp` | The platform interface: config structs and every `pfm_*` function |
| `IFB.Engine/include/eng-platform.hpp` | Window event flags, file access/share/mode enums, and the window context |
| `IFB.Core/include/ifb-win32.hpp` | Win32 types: the window struct, the window callback, keycode translation, and string conversion |

## Functions

| Area | Functions | Win32 implementation |
| --- | --- | --- |
| Context | `pfm_context_startup`, `_shutdown` | `win32.cpp`: starts COM and creates the open and save dialog objects |
| Window | `pfm_window_open`, `_frame_start`, `_frame_render`, `_process_events`, `_close`, `_quit_received` | `win32-window.cpp` |
| Graphics | `pfm_graphics_init_opengl`, `_init_imgui` | `win32-graphics.cpp` |
| Monitor | `pfm_monitor_count`, `_get_primary_index`, `_get_working_area`, `_get_info` | `win32-monitor.cpp` |
| Memory | `pfm_memory_commit`, `_decommit`, `_get_info` | `win32-memory.cpp`: `VirtualAlloc(MEM_COMMIT)` / `VirtualFree(MEM_DECOMMIT)`, page size and allocation granularity |
| Files | `pfm_file_open`, `_size`, `_close`, `_read`, `_write` | `win32-file.cpp`: `CreateFile`, `ReadFile`, `WriteFile` |
| File dialogs | `pfm_file_dialog_open`, `_save` | `win32-file-dialog.cpp`: `IFileOpenDialog`, `IFileSaveDialog` |
| System | `pfm_system_timestamp`, `_frequency` | `win32-system.cpp` |

## Window

`eng_context_startup` opens the window with `pfm_window_open`, centered on the primary monitor at `IFB_CONFIG_WINDOW_START_WIDTH × IFB_CONFIG_WINDOW_START_HEIGHT` (1024 × 768). The title is `It Flies By (Debug)` when `IFB_CONFIG_BUILD_MODE` is `0`, and `It Flies By` otherwise. `IFB_CONFIG_BUILD_MODE` is set to `0` in `ifb-config.hpp`, not by the CMake build type, so release builds show the debug title too. It's a normal resizable window (`WS_OVERLAPPEDWINDOW`).

Each rendered frame:

| Call | When | What it does |
| --- | --- | --- |
| `pfm_window_frame_start` | Start of the frame | Clear the viewport, and start a new ImGui frame |
| `pfm_window_process_events` | Start of the frame | Handle all waiting window messages (see below) |
| `pfm_window_frame_render` | End of the frame | Render ImGui on top of everything, then `SwapBuffers` |

See the [frame loop](architecture.md#frame-loop) for where these fall.

### Events

`pfm_window_process_events` reads every waiting message with `PeekMessage`:

| Message | Handling |
| --- | --- |
| `WM_KEYDOWN`, `WM_SYSKEYDOWN`, `WM_KEYUP`, `WM_SYSKEYUP` | Translated with `win32_input_get_keycode` and passed to the [input system](input.md) |
| `WM_MOUSEMOVE` | Passed to `eng_input_mouse_move`, which doesn't do anything yet |
| `WM_QUIT`, `WM_DESTROY`, `WM_CLOSE` | Marks the window to quit |

Every message is then passed on to the window callback, which gives ImGui first look at it, and also handles:

| Message | Handling |
| --- | --- |
| `WM_SIZE` | Updates the window size, and the renderer's viewport with it |
| `WM_MOVE` | Updates the window position |
| `WM_CLOSE`, `WM_DESTROY` | Destroys the window and posts the quit message |

When the quit flag is set, `eng_context_run` returns `false` and the game's main loop ends.

## Graphics

`pfm_graphics_init_opengl` creates the OpenGL context for the window (`gl_context_init_win32` from SLD.OpenGL), turns on depth buffering and smoothing, sets the clear color to `#282828`, and sets the viewport to the window size.

`pfm_graphics_init_imgui` sets up ImGui's Win32 and OpenGL 3 backends. The shaders use GLSL `#version 330`.

## Time

`pfm_system_timestamp` and `pfm_system_frequency` wrap the high-resolution performance counter. The engine uses them to calculate the delta time each pass of the frame loop (`system_update_time`), and the [random system](random.md) uses the timestamp as its seed.

## Memory and System Info

`system_refresh_info` (`eng-system.cpp`) fills in `eng_system_info` from the platform: the memory page size and allocation granularity, the monitor count, the first monitor's size and position, and the working area. Reservations and arenas use the page size to commit memory in whole pages.

## Not Implemented

| Function | Status |
| --- | --- |
| `pfm_file_open_async_r`, `_w`, `_rw` | Declared in `ifb-platform.hpp`, but not defined |
| `pfm_file_async_read`, `_write` | Defined, but return `0` without doing anything |

## Adding a Platform

To port the engine, implement everything in `ifb-platform.hpp` for the new platform, then:

- Replace the files in `IFB.Engine/src/win32` and their `#include` in `eng.cpp`.
- Provide keycode translation into `input_keycode_e` (see [Input](input.md#keycodes)).
- Replace the entry point and `VirtualAlloc` calls in `game-win32-main.cpp`.
- Update `CMakeLists.txt`, which links `user32`, `gdi32`, and `kernel32` and builds the game as a `WIN32` executable.
- Use the matching ImGui platform backend.
