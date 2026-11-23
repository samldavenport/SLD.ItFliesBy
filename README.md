# It Flies By

A 2.5D game and the custom C++ engine it's built on. Sprites move around on 3D tile maps, with a physics system, particles, and an ImGui editor/debug interface over the top. The engine runs on Windows (Win32 + OpenGL).

More detailed documentation lives in [`docs/`](docs/). The game's own [in-game UI](docs/ui.md) controls (buttons, checkboxes, sliders, and drop downs) are documented there too.

## Repository Layout

```
IFB.Core/      header-only types and the public engine API, shared by every module
IFB.Engine/    the engine, built as a DLL (ItFliesBy.Engine.dll)
IFB.GUI/       ImGui editor and debug windows, built as a static library
IFB.Game/      the game itself and the Win32 entry point (ItFliesBy.exe)
SLD.*/         general purpose libraries, pulled in as git submodules
assets/        images, sprite atlases, shaders, fonts, and the asset dictionary/database
scripts/       PowerShell scripts for setup and building
```

## Modules

| Module | Target | Description |
| --- | --- | --- |
| **IFB.Core** | `IFB::Core` (header only) | Common types used across the project: handles and IDs, component and archetype structs, asset/sprite/map/particle info structs, the build configuration in `ifb-config.hpp`, and the profiler macros in `ifb-profile.hpp`. `ifb-engine.hpp` is the engine's public API: every `eng_*` function the game and GUI are allowed to call. |
| **IFB.Engine** | `IFB::Engine` (`ItFliesBy.Engine.dll`) | The engine. Owns the window, input, memory, assets, scenes, entities and components, physics, events, jobs, maps, particles, sprites, and the renderer. Its internal headers in `IFB.Engine/include` (`eng-*.hpp`) aren't visible to other modules. |
| **IFB.GUI** | `IFB::GUI` (`ItFliesBy.GUI.lib`) | ImGui tooling built on the public engine API: entity and quad managers, camera controls, the particle system editor, the sprite atlas builder and inspector, asset dictionary/database editors, the physics test window, and a menu item that opens the Tracy profiler. |
| **IFB.Game** | `ifb` (`ItFliesBy.exe`) | The game. `game-win32-main.cpp` reserves memory, creates the engine context, and runs the main loop. The game code runs one scene at a time (the dev scene, Cliff Town, and a physics stress test), fading through black when it switches. Scene triggers act as doors between maps. |
| **SLD.Core** | `SLD::Core` | Base types (`u32`, `f32`, `cchar`, ...), strong typedefs, hashing, and memory helpers. |
| **SLD.Math** | `SLD::Math` | Vectors, matrices, and colors. |
| **SLD.OpenGL** | `SLD::OpenGL` | OpenGL loading and wrappers. |
| **SLD.Strings** | `SLD::Strings` | C-string utilities. |

The game and GUI only talk to the engine through `ifb-engine.hpp`. Objects inside the engine are referred to by handles (`hnd_map`, `hnd_sprite_atlas`, ...) and entity IDs, so engine structs never cross the DLL boundary.

## Engine Systems

| System | Source | Description | Docs |
| --- | --- | --- | --- |
| **Context** | `src/engine/eng-context.cpp`, `eng-context.hpp` | Creates and starts every manager, runs the frame loop, and calls back into the game and GUI each frame. | [Architecture](docs/architecture.md) |
| **Memory** | `src/memory`, `eng-memory.hpp` | Virtual memory reservations, the global stack, and arenas (normal and large). Each system gets its own reservation from the memory map the game passes in. | [Memory](docs/memory.md) |
| **Assets** | `src/assets`, `eng-asset.hpp` | Text, images, and sprite atlases. Source files are listed in a JSON asset dictionary, packed into a binary asset database (`.ifbdb`), and loaded by name at runtime. | [Assets](docs/assets.md) |
| **Sprites** | `src/sprites`, `eng-sprite.hpp` | Sprite atlases (`.ifbatlas`) packed from image assets. Includes the atlas builder and inspector used by the GUI. | [Sprites](docs/sprites.md) |
| **Scenes** | `src/scenes`, `eng-scene.hpp` | A stack of scenes. Entities, particle emitters, parallax, and scene memory belong to the bound scene and are released with it. Flags decide whether the scenes under the top keep updating and rendering. | [Scenes](docs/scenes.md) |
| **Entities** | `src/entities`, `eng-entity.hpp` | Entities are IDs hashed from a tag of up to 31 characters, stored in a sparse set along with each entity's archetype (component mask) and the scene that owns it. | [Entities & Components](docs/entities-and-components.md) |
| **Components** | `src/components`, `eng-component.hpp` | One array per component type (position, velocity, sprite, ...), indexed by the entity's sparse index. | [Entities & Components](docs/entities-and-components.md#components) |
| **Archetypes** | `src/engine/eng-atype.cpp` | Predefined component sets (`sprite`, `particle`, `physics_sprite`) that can be read and written as one struct. | [Entities & Components](docs/entities-and-components.md#archetypes) |
| **Physics** | `src/physics`, `eng-physics.hpp` | Force accumulation, springs, integration, BVH collision detection per map (queried on the job workers) with collider masks, collision resolution with restitution against other entities and map walls, begin/stay/end contacts sent to collider procs and events, and per-step stats. | [Physics](docs/physics.md) |
| **Maps** | `src/map`, `eng-map.hpp` | Tile maps made of chunks, with floor tiles, row/column walls, colors, and sprites. Converts between world positions and map coordinates. | [Maps](docs/maps.md), [Map Format](docs/map-format.md) |
| **Particles** | `src/particle`, `eng-particle.hpp` | Parabolic (fountain) and radial (explosion) particle emitters, which can follow an entity. | [Particles](docs/particles.md) |
| **Renderer** | `src/renderer`, `eng-renderer.hpp` | OpenGL rendering for quads, tiles, particles, the parallax backdrop, the debug grid, and the direction gizmo. Immediate drawing of quads, sprites, and text in screen or world space, for UI like dialog boxes. Owns the camera and viewport. | [Renderer](docs/renderer.md) |
| **Quads** | `src/quads`, `eng-quad.hpp` | Quad vertex buffers used to draw sprites and colored entities, radix sorted back to front. | [Quads](docs/quads.md) |
| **Files** | `src/files`, `eng-file.hpp` | File handles for reading and writing, plus the native file dialog. | [Files](docs/files.md) |
| **Input** | `src/engine/eng-input.cpp`, `eng-input.hpp` | Keyboard state, and the mouse's position (in screen-layer units) and buttons. Both are released when the window loses the focus, and hidden from the game while the GUI has them. | [Input](docs/input.md) |
| **Events** | `src/events`, `eng-events.hpp` | A queue of engine events, sent to subscribed listeners once per pass of the frame loop. Physics publishes collision events. | [Events](docs/events.md) |
| **Jobs** | `src/jobs`, `eng-jobs.hpp` | A pool of worker threads that sleep until a job is added, job counters for waiting on a batch, and the rules for what jobs can call. Used by the physics BVH query and audio streaming. | [Jobs](docs/jobs.md) |
| **Audio** | `src/audio`, `eng-audio.hpp` | A buffer written each frame and sent to the output device, and a test window for checking that it plays cleanly. | [Audio](docs/audio.md) |
| **JSON / Tiled** | `src/json`, `src/tiled` | RapidJSON wrappers, and a parser for [Tiled](https://www.mapeditor.org/) map files. | [JSON & Tiled](docs/json-and-tiled.md) |
| **Random** | `src/random`, `eng-random.hpp` | A table of random values refreshed on a fixed frame interval. | [Random](docs/random.md) |
| **Profiling** | `IFB.Core/include/ifb-profile.hpp` | `IFB_PROFILE_*` macros over the [Tracy](https://github.com/wolfpld/tracy) profiler, which compile away without `IFB_ENABLE_PROFILING`. | [Architecture](docs/architecture.md#profiling) |
| **Platform (Win32)** | `src/win32`, `ifb-platform.hpp` | Window, OpenGL context, memory, files, monitors, threads, and system info for Windows. | [Platform](docs/platform.md) |

## Building

Requirements:

- Windows with Visual Studio (MSVC) and the C++ workload
- CMake 3.21+ and Ninja
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set or `vcpkg` on your `PATH`
- [Git LFS](https://git-lfs.com), for the sound banks (`*.ifbsnd`)

vcpkg installs `glew`, `imgui` (Win32 + OpenGL3 bindings, docking), `implot`, `stb`, `rapidjson`, and `tracy` (with `gui-tools`, which builds the profiler viewer) from `vcpkg.json`.

```powershell
# pull the SLD submodules, set up git lfs, and download the sound banks
./scripts/Initialize-Repo.ps1

# build (Debug by default)
./scripts/Build-ItFliesBy.ps1
./scripts/Build-ItFliesBy.ps1 -Config Release

# remove the build directory and compile_commands.json
./scripts/Build-ItFliesBy.ps1 -Clean
```

Or with CMake presets directly:

```powershell
cmake --preset debug
cmake --build --preset debug
```

Profiling with Tracy is on by default. Configure with `-DIFB_ENABLE_PROFILING=OFF` to build without it, and the `IFB_PROFILE_*` macros compile away. See [Profiling](docs/architecture.md#profiling).

The executable, engine DLL, shaders, and asset database are all written to `build/<preset>/bin`. The engine loads its shaders and the asset database from the working directory, so run the game from that folder.

Each module is a unity build: `eng.cpp`, `gui.cpp`, and `game-win32-main.cpp` include the rest of their module's `.cpp` files. When you add a source file, include it from the module's unity file. C++ exceptions are disabled.

## License

[MIT](LICENSE)
