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

1. Builds an `eng_mem_map`: a fixed stack buffer plus one `VirtualAlloc` reservation per system (core, files, rendering, entities, components, physics, tiles, particles, assets, sprites, arenas, ...).
2. Calls `eng_context_create` with the memory map and two callbacks: `game_proc` (game update) and `render_proc` (GUI render).
3. Calls `eng_context_startup`, which starts every manager.
4. Creates the game context (`game_context_create_and_init`) and opens the GUI.
5. Calls `eng_context_run` until it returns `false`.

## Frame Loop

`eng_context_run` (`src/engine/eng-context.cpp`) runs every iteration. Simulation steps run every iteration with the measured delta time; frame steps only run once enough time has passed for a new frame (`IFB_CONFIG_DEFAULT_FPS`).

| Step | When |
| --- | --- |
| Update system info and delta time | every iteration |
| `asset_mngr_update`, window frame start, process window events, `random_table_update` | new frame |
| Game callback (`game_context_update_and_render`) | every iteration |
| `phys_mngr_simulate(dt)` | every iteration |
| Particles follow their entities, then update | every iteration |
| `renderer_draw_buffers`, GUI callback, present the frame | new frame |

### Physics Step

`phys_mngr_simulate` runs in this order:

1. Collision detection (BVH broad phase)
2. Spring forces
3. Force integration, which writes new velocities and positions into the components
4. Collision resolution, using the pairs found in step 1
5. Reset the force accumulator

## Memory

The engine doesn't use the C++ heap. All memory comes from reservations in the memory map the game passes to `eng_context_create`.

| Allocator | Description |
| --- | --- |
| **Reservation** | A range of reserved virtual memory owned by one system. Pages are committed as they're needed. |
| **Global stack** | The fixed stack buffer from the memory map. Used for long lived allocations made at startup (`global_alloc`). |
| **Arena** | A bump allocator returned as an `hnd_arena`. Supports `save`/`revert` and `reset`. Arenas are committed in `IFB_CONFIG_ARENA_MEM_GRANULARITY` steps. |
| **Large arena** | Same API as an arena, for bigger allocations (`eng_arena_large_alloc`). Large arena handles have the high bit set (`ARENA_HANDLE_LARGE_BIT`), so every arena function works with either kind. |
| **Block allocator** | Fixed size blocks tracked with a bitmask. |

Sizes and capacities for every system are set in `IFB.Core/include/ifb-config.hpp`.

## Naming Conventions

| Prefix / suffix | Meaning |
| --- | --- |
| `eng_*` | Public engine API (`ifb-engine.hpp`) |
| `*_mngr` | The singleton manager for a system, e.g. `entity_mngr`, `phys_mngr` |
| `_name` (leading underscore) | A global pointer to a manager, e.g. `_eng_context` |
| `hnd_*` | An opaque handle |
| `cmpnt_*` | A component |
| `atype_*` | An archetype struct |
| `*_e` | An enum |
| `IFB_U32(name)`, `IFB_HANDLE(name)` | Strong typedefs over integers, so different IDs and handles can't be mixed up |
