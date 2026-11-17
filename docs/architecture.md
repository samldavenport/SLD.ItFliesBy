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
4. Creates the game context (`game_context_create_and_init`), which enters the first scene (the dev scene), and opens the GUI.
5. Calls `eng_context_run` until it returns `false`.
6. Calls `eng_context_shutdown`, which stops the worker threads first, then shuts down the assets and the platform.

## Frame Loop

`eng_context_run` (`src/engine/eng-context.cpp`) runs every iteration. Simulation steps run every iteration with the measured delta time; frame steps only run once enough time has passed for a new frame (`IFB_CONFIG_DEFAULT_FPS`).

The delta time is capped at `IFB_CONFIG_DELTA_TIME_MAX_S` (0.1 seconds). A long pause, like a breakpoint, a window drag, or the first frame, would otherwise go into one step and move everything by its velocity for all of it. The game, physics, and particles all see the capped value.

| Step | When |
| --- | --- |
| Update system info and delta time | every iteration |
| `asset_mngr_update`, window frame start, process window events, `random_table_update`, `audio_mngr_frame_start` | new frame |
| `scene_mngr_frame_start`, which applies the [scene](scenes.md#when-changes-happen) pushes, pops, resets, and destroys requested since the last pass | every iteration |
| Clear the [immediate drawing](renderer.md#immediate-drawing) layers, then the game callback (`game_context_update_and_render`) | every iteration |
| `phys_mngr_simulate(dt)` | every iteration |
| `event_mngr_update`, which calls the [event](events.md) listeners | every iteration |
| Particles follow their entities, then update | every iteration |
| `renderer_draw_buffers` (which loads the top scene's parallax if it changed), clear the entities' drawn marks, GUI callback, `audio_mngr_frame_end`, present the frame | new frame |

### Physics Step

`phys_mngr_simulate` runs in this order:

1. Swap this step's and the last step's contact lists
2. Collision detection (a BVH broad phase per map, queried on the job workers, filtered by collider masks)
3. Spring forces
4. Force integration, which writes new velocities and positions into the components
5. Collision resolution, which records the pairs from step 2 that actually overlap as contacts, and pushes and bounces the solid ones apart
6. Reset the force accumulator
7. Contact dispatch, which gives each contact a begin, stay, or end phase, calls the collider procs, and publishes a collision [event](events.md) for each

See [Physics](physics.md#simulation-step) for details.

## Threads

The frame loop runs on the main thread, which owns the game world, the window, and all rendering. The engine also starts a pool of worker threads for [jobs](jobs.md), which sleep until a job is added. In the frame loop, physics uses them to [query its BVH](physics.md#querying-on-worker-threads), and waits for those jobs before it goes on. Audio streaming reads on them too.

Only part of the engine is safe to call from a job. See the [thread rules](jobs.md#thread-rules).

## Profiling

The engine, GUI, and game are instrumented for the [Tracy](https://github.com/wolfpld/tracy) profiler through the macros in `IFB.Core/include/ifb-profile.hpp`, so nothing else includes Tracy directly:

| Macro | Description |
| --- | --- |
| `IFB_PROFILE_FUNCTION()` | A zone named after the enclosing function, until the end of the scope |
| `IFB_PROFILE_SCOPE(name)` | A zone with its own name, for a block inside a function |
| `IFB_PROFILE_FRAME()` | Marks the end of a frame. `eng_context_run` calls it once the frame is presented. |
| `IFB_PROFILE_THREAD(name)` | Names the calling thread. The job workers are `Job Worker`. |
| `IFB_PROFILE_PLOT(name, value)` | Plots a value over time |

The CMake option `IFB_ENABLE_PROFILING` (on by default) links `Tracy::TracyClient` into every module and defines `IFB_PROFILE_ENABLED`. With it off, every macro compiles away. Tracy comes from vcpkg with its `gui-tools` feature, which also builds the viewer, `tracy-profiler.exe`.

To profile, run the game and pick **Engine > Tools > Profiler (Tracy)** in the GUI. It starts the viewer and connects it to the running game. The item is disabled, with a tooltip saying why, when the build doesn't have profiling or CMake couldn't find the viewer. The viewer can also be started by hand from `vcpkg_installed/x64-windows/tools/tracy`.

Add a zone to anything that runs a few times per frame. Skip the ones that run per entity or per quad, where the zone costs more than the work.

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
