# Architecture

## Module Boundaries

```
 IFB.Game (exe) ──┬──> IFB.GUI (static lib) ──┐
                  └───────────────────────────┴──> IFB.Engine (DLL) ──> IFB.Core (headers) ──> SLD.*
```

- **IFB.Core** has no source files. It defines the types shared between modules (`ifb.hpp`), build settings (`ifb-config.hpp`), and the engine's public API (`ifb-engine.hpp`).
- **IFB.Engine** exports only the functions marked `IFB_ENGINE_API`. Everything else is `IFB_INTERNAL` (`static`), and engine structs are declared in `IFB.Engine/include`, which other modules can't include.
- **IFB.GUI** and **IFB.Game** work with handles (`hnd_*`) and `entity_id`s returned by the engine and never hold engine pointers.

## Unity Builds

Each module compiles as a single translation unit:

| Module | Unity file |
| --- | --- |
| IFB.Engine | `IFB.Engine/src/engine/eng.cpp` |
| IFB.GUI | `IFB.GUI/src/gui.cpp` |
| IFB.Game | `IFB.Game/src/game-win32-main.cpp` |

CMake globs the other `.cpp` files and marks them `HEADER_FILE_ONLY`, so they show up in the IDE but only build through the unity file. A new source file has to be `#include`d from the unity file to be compiled.

## Startup

`wWinMain` in `game-win32-main.cpp`:

1. Builds an `eng_mem_map`: a fixed stack buffer plus one `VirtualAlloc` reservation per system (core, files, rendering, entities, components, physics, tiles, particles, assets, sprites, events, jobs, arenas, ...).
2. Calls `eng_context_create` with the memory map and two callbacks: `game_proc` (game update) and `render_proc` (GUI render).
3. Calls `eng_context_startup`, which starts every manager. The scene manager starts last, and creates and binds the [default scene](scenes.md#the-default-scene).
4. Creates the game context (`game_context_create_and_init`), which creates and pushes the gameplay scene, and opens the GUI.
5. Calls `eng_context_run` until it returns `false`.
6. Calls `eng_context_shutdown`, which stops the worker threads first, then shuts down the assets and the platform.

## Frame Loop

`eng_context_run` (`src/engine/eng-context.cpp`) runs every iteration. Simulation steps run every iteration with the measured delta time; frame steps only run once enough time has passed for a new frame (`IFB_CONFIG_DEFAULT_FPS`).

| Step | When |
| --- | --- |
| Update system info and delta time | every iteration |
| `asset_mngr_update`, window frame start, process window events, `random_table_update`, `audio_mngr_frame_start` | new frame |
| `scene_mngr_frame_start`, which applies the [scene](scenes.md#when-changes-happen) pushes, pops, resets, and destroys requested since the last pass | every iteration |
| Clear the [immediate drawing](renderer.md#immediate-drawing) layers, then the game callback (`game_context_update_and_render`) | every iteration |
| `phys_mngr_simulate(dt)` | every iteration |
| `event_mngr_update`, which calls the [event](events.md) listeners | every iteration |
| Particles follow their entities, then update | every iteration |
| `renderer_draw_buffers` (which loads the top scene's parallax if it changed), GUI callback, `audio_mngr_frame_end`, present the frame | new frame |

### Physics Step

`phys_mngr_simulate` runs in this order:

1. Collision detection (BVH broad phase), which publishes a collision [event](events.md) for each pair
2. Spring forces
3. Force integration, which writes new velocities and positions into the components
4. Collision resolution, using the pairs found in step 1
5. Reset the force accumulator

## Threads

The frame loop runs on the main thread, which owns the game world, the window, and all rendering. The engine also starts a pool of worker threads for [jobs](jobs.md), which sleep until a job is added. Nothing in the frame loop uses them yet.

Only part of the engine is safe to call from a job. See the [thread rules](jobs.md#thread-rules).

## Memory

The engine doesn't use the C++ heap. All memory comes from reservations in the memory map the game passes to `eng_context_create`.

| Allocator | Description |
| --- | --- |
| **Reservation** | A range of reserved virtual memory owned by one system. Pages are committed as they're taken. |
| **Global stack** | The fixed stack buffer from the memory map. Used for allocations that live for the whole run (`global_alloc`). |
| **Arena** | A fixed size bump allocator returned as an `hnd_arena`. Supports `save`/`revert` and `reset`. Every [scene](scenes.md#ownership) has one, for `eng_scene_alloc`. |
| **Large arena** | Same API as an arena, for bigger allocations (`eng_arena_large_alloc`). Commits memory as it grows. |

See [Memory](memory.md) for details.

Sizes and capacities for every system are set in `IFB.Core/include/ifb-config.hpp`.

## Naming Conventions

| Prefix / suffix | Meaning |
| --- | --- |
| `eng_*` | Public engine API (`ifb-engine.hpp`) |
| `*_mngr` | The singleton manager for a system, e.g. `entity_mngr`, `phys_mngr` |
| `_name` (leading underscore) | A global pointer to a manager, e.g. `_eng_context` |
| `hnd_*` | An opaque handle, e.g. `hnd_scene`, `hnd_map` |
| `cmpnt_*` | A component |
| `atype_*` | An archetype struct |
| `*_e` | An enum |
| `IFB_U32(name)`, `IFB_HANDLE(name)` | Strong typedefs over integers, so different IDs and handles can't be mixed up |
