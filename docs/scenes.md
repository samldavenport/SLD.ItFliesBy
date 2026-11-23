# Scenes

A scene (`IFB.Engine/src/scenes`, `eng-scene.hpp`) is a container for what a part of the game is using: its entities, particle emitters, parallax, and any memory it allocates. Scenes sit on a **stack**. The top scene updates and renders, and the scenes under it can keep running if the scenes above allow it. Destroying a scene releases everything it owns.

Scenes work like OpenGL's bound objects: one scene is **bound** at a time, and everything created while it's bound belongs to it.

```cpp
const hnd_scene h_scene = eng_scene_create("GAMEPLAY");
eng_scene_push(h_scene, scene_flag_e_none);   // pushing also binds it

// everything created now belongs to GAMEPLAY
const entity_id id = eng_entity_create("CONNOR");
auto state = (my_state*)eng_scene_alloc(h_scene, sizeof(my_state));
```

## Ownership

These belong to the scene that's bound when they're created:

| Resource | Created by | Released when the scene is reset or destroyed |
| --- | --- | --- |
| Entities | `eng_entity_create` | Destroyed with `entity_destroy_by_scene` |
| Particle emitters | `eng_particle_emitter_create_parabolic` / `_radial` | Destroyed with `particle_mngr_destroy_by_scene` |
| Maps | `eng_map_create`, `eng_map_load`, `eng_map_load_as` | Destroyed with `map_destroy_by_scene` |
| Memory | `eng_scene_alloc(h_scene, size)` | The scene's arena is reset |
| Parallax | `eng_scene_set_parallax`, `eng_parallax_set` | Cleared |

`eng_entity_get_scene(id)` returns the scene that owns an entity, and `eng_scene_get_entity_count(h_scene)` counts them.

Each scene has its own arena from the [arena](memory.md) pool. `eng_scene_alloc` pushes onto it, so anything the game keeps for a scene's lifetime, like its state structs, should be allocated there instead of from a global arena.

Not everything is owned by a scene yet:

- **Sprite atlases, sounds, and event listeners** are global.
- Destroying an entity doesn't clear its component, physics, or particle data. The slot is reused by the next entity.

### The Default Scene

`scene_mngr_startup` creates a scene named `default`, pushes it, and binds it. It's always at the bottom of the stack:

- Entities created before the game makes its own scenes belong to it.
- It can't be popped or destroyed, but it can be reset.
- `eng_scene_get_default` returns it.

## Binding

| Function | Description |
| --- | --- |
| `eng_scene_bind(h_scene)` | Bind a scene without changing the stack |
| `eng_scene_get_bound` | The bound scene |

The stack functions bind for you:

- **Push** binds the pushed scene.
- **Pop** binds the new top.
- **Destroy** binds the top of the stack, if the destroyed scene was bound.

A scene doesn't have to be on the stack to be bound. That's how a scene can be loaded while another one is playing: create it, bind it, create its entities, then push or transition to it when it's ready. Bind the scene you were in again afterwards, or the next things you create go to the loaded scene.

## The Stack

| Function | Description |
| --- | --- |
| `eng_scene_push(h_scene, flags)` | Put the scene on top and bind it. Fails if it's already on the stack or is being destroyed. |
| `eng_scene_pop` | Take the top scene off and bind the new top. The scene isn't destroyed: it keeps what it owns and can be pushed again. The default scene can't be popped. |
| `eng_scene_transition(h_scene, flags)` | Destroy the top scene and push this one in its place. Over the default scene it just pushes. |
| `eng_scene_get_top` | The top scene |
| `eng_scene_get_stack_count` | How many scenes are on the stack, including the default scene |

A scene can only be on the stack once. The stack holds up to `IFB_CONFIG_SCENE_CAPACITY` (16) scenes, which is also how many scenes can exist at once.

### Flags

A scene's flags are given when it's pushed, and say what the scenes **under** it can do:

| Flag | Scenes below |
| --- | --- |
| `scene_flag_e_none` | Stop updating and rendering |
| `scene_flag_e_update_below` | Keep updating |
| `scene_flag_e_render_below` | Keep rendering |

The top scene always updates and renders. Going down the stack, a scene runs only if **every** scene above it lets it, so a scene that blocks rendering hides everything under it, whatever the scenes further down allow.

For example, a pause menu pushed with `scene_flag_e_render_below` over the gameplay scene:

| Stack | Flags | Updates | Renders |
| --- | --- | --- | --- |
| PAUSE (top) | `render_below` | yes | yes |
| GAMEPLAY | `none` | no | yes |
| default | `none` | no | no |

The game is drawn under the menu, but frozen.

`eng_scene_is_updating(h_scene)` and `eng_scene_is_rendering(h_scene)` give the state for the current frame.

### What Stops

The engine skips entities and emitters whose scene isn't running:

| When a scene isn't updating | When a scene isn't rendering |
| --- | --- |
| Physics doesn't integrate its entities, give them collision bounds, or apply their springs | `eng_entity_render` does nothing for its entities |
| Its emitters don't follow their entities, age, move, or spawn | Its parallax isn't drawn |

The engine doesn't know about the game's own logic, so the game has to check too. Wrap each scene's game code in a check like the dev scene does:

```cpp
if (!eng_scene_is_updating(h_scene) && !eng_scene_is_rendering(h_scene)) return;
```

Code that both updates and renders has to check them separately to keep a paused scene still while it's drawn.

## When Changes Happen

Push, pop, transition, destroy, and reset **bind right away**, and `eng_scene_get_top`, `_get_stack_count`, and `_get_bound` reflect them right away. But what runs and what gets released only changes at the start of the next pass of the [frame loop](architecture.md#frame-loop), in `scene_mngr_frame_start`, before the game callback. This keeps the frame consistent: physics, events, and rendering all see the same scenes the game callback started with.

`scene_mngr_frame_start` applies the requests in this order:

1. **Resets**: release everything the scene owns. The scene stays alive and keeps its place on the stack. A scene that's also being destroyed is skipped here.
2. **Destroys**: release everything the scene owns, free its arena, and free its slot.
3. **Stack**: the requested stack becomes the running stack, and each scene's updating and rendering state is worked out again.

If anything changed, the [event](events.md) queue is flushed, because queued events can name entities that were just destroyed.

A few things follow from this:

- A destroyed scene's handle stays valid until the next frame starts, so its name and entities can still be looked up.
- A reset releases everything the scene owns at the next frame start, including anything created in it after the reset was requested.
- A new scene doesn't run until the frame after it's pushed, so its entities aren't drawn or simulated in the frame they're created.

## Transitions

`eng_scene_transition` moves from one level to the next:

```cpp
const hnd_scene h_next = eng_scene_create("LEVEL-2");
eng_scene_transition(h_next, scene_flag_e_none);   // LEVEL-1 is destroyed, LEVEL-2 is pushed and bound

// these belong to LEVEL-2
eng_entity_create("CONNOR");
```

The old scene is released at the start of the next frame, and the new one starts running in the same frame. Any game state allocated with `eng_scene_alloc` in the old scene is gone after that, so don't keep pointers to it. The same goes for listeners that read scene state through a global: point them at the new state before the next frame starts.

## Parallax

The parallax backdrop is three image layers drawn behind the map, which scroll at different speeds as the camera moves. Each scene has its own.

```cpp
// front, mid, back, all image assets of the same size
eng_scene_set_parallax(h_scene, "PARALLAX-FRONT", "PARALLAX-MID", "PARALLAX-BACK");

// the same, for the bound scene
eng_parallax_set("PARALLAX-FRONT", "PARALLAX-MID", "PARALLAX-BACK");
```

Setting it checks that every name is an image in the asset database and fails if one isn't. The layers aren't loaded yet; the scene only stores the names. `eng_scene_clear_parallax` removes it.

The renderer draws the parallax of the **highest rendering scene that has one**. A pause menu without a parallax, pushed with `render_below`, shows the gameplay scene's backdrop under it. One that blocks rendering hides it.

Each time a parallax is set, it gets a new revision number. `renderer_sync_parallax` runs at the start of `renderer_draw_buffers` and loads the layers only when the revision it should draw changes, so the images aren't loaded again every frame. If they fail to load, nothing is drawn until a new parallax is set.

The backdrop stands just behind the far edge of the map being drawn, so nothing is drawn without a map. See `renderer-parallax.cpp` for the layer speeds and sizes.

## In the Game

The game runs one scene at a time (`IFB.Game/src/game-scene.cpp`). Each one has an ID in `game_scene_id_e`, an engine scene name in `GAME_SCENE_NAMES`, and three procs in the `GAME_SCENE_PROCS` table:

| Proc | When it runs | What it does |
| --- | --- | --- |
| `enter` | The scene is created and bound | Allocates the scene's state with `eng_scene_alloc`, and spawns its entities and NPCs |
| `update_and_render` | Every game update while it's active | Returns early when the engine says the scene isn't running |
| `exit` | After the engine released the scene | Clears what pointed into the scene, and releases anything the engine scene doesn't own |

`exit` runs after the scene's entities, maps, and `eng_scene_alloc` memory are gone, so it can't read them.

Game functions are declared with `IFB_GAME` (`ifb-game.hpp`), which is `static`, the same as the engine's `IFB_INTERNAL`.

### Switching Scenes

`game_scene_request(id, arrival, npc_keep, npc_keep_count)` asks for a switch. Everything after `id` is optional. It's safe to call from anywhere, including collision procs, and returns false while another switch is requested or underway (`game_scene_is_switching`).

A switch fades the screen to black, switches while it's black, then fades the new scene in. The game scene state moves through four phases (`game_scene_phase_e`):

| Phase | What happens | Next |
| --- | --- | --- |
| `running` | The active scene updates. A requested switch starts the fade out at the end of the update. | `fade_out` |
| `fade_out` | The old scene keeps running under a fade to black, for `GAME_SCENE_FADE_OUT_S` (0.3 s). Once it's black, the game creates the new engine scene and calls `eng_scene_transition`. The old scene's collision procs still run in that frame's physics step. | `switching` |
| `switching` | The next frame, the engine has released the old scene in `scene_mngr_frame_start`. The old scene exits, the NPCs that aren't kept are forgotten, the dialog and the scene triggers are reset, and the new scene enters and updates. | `fade_in` |
| `fade_in` | The screen stays black for `GAME_SCENE_FADE_HOLD_S` (0.1 s), which covers the new scene's first frames, then fades in over `GAME_SCENE_FADE_IN_S` (0.3 s). | `running` |

The fade's time step is capped at `GAME_SCENE_FADE_MAX_DT` (1/30 s), so a long frame, like the one that loads a map, doesn't skip it. `game_scene_render_fade` draws the black as a screen-space rectangle with an eased alpha. It's drawn last, after the dialog, so only the GUI is on top of it. Connor's movement keys are locked, and presses are dropped, from the request until the fade in is done.

The new scene enters a frame after the transition because entity IDs are a hash of the entity's name, and creating an entity whose name is already taken asserts. The old scene's entities, like `CONNOR`, are only destroyed at the start of the next frame, so a scene can't create its own `CONNOR` until then.

`game_scene_init` enters the first scene right away, since nothing is running yet, and fades it in from black.

### Arrivals

A `game_scene_arrival` is the tile Connor is placed on when a scene enters: a chunk index and a tile in that chunk, like the door he came through. Pass one to `game_scene_request` to choose where he arrives, or `NULL` for the scene's own spawn.

A scene's `enter` proc calls `game_scene_get_arrival(spawn)` with its own spawn tile, and gets the requested arrival if there was one, or the spawn if there wasn't. The arrival is used up by `enter`.

### Scene Triggers

A scene trigger (`game-scene-trigger.cpp`) is a tile, or one edge of a tile, that switches scenes when Connor touches it, like a door. Each one is an invisible [trigger](physics.md#colliders) entity in the scene that spawned it, so it's gone with the scene.

```cpp
const game_scene_arrival cliff_town_arrival = { 0, 1, 12 };

game_scene_trigger_def to_cliff_town;
to_cliff_town.cstr_name   = "TRG-CLIFF-TOWN";             // the entity's tag, unique in the scene
to_cliff_town.h_map       = map->eng_hnd;
to_cliff_town.chunk_index = 0;
to_cliff_town.chunk_x     = 5;
to_cliff_town.chunk_y     = 0;
to_cliff_town.side        = game_scene_trigger_side_e_y_min;
to_cliff_town.target      = game_scene_id_e_cliff_town;
to_cliff_town.arrival     = &cliff_town_arrival;          // NULL for the target's own spawn

const entity_id id = game_scene_trigger_spawn(to_cliff_town);
```

| Side | What it covers |
| --- | --- |
| `game_scene_trigger_side_e_tile` | The whole tile. Connor walks onto it. |
| `_x_min`, `_x_max`, `_y_min`, `_y_max` | A thin slab along that edge of the tile, thicker than a wall, so Connor touches it walking into a wall there, into the edge of the map, or through a gap in a wall. |

In a [map file](map-format.md), a row wall at `(row, col)` is the `y_min` side of tile `(col, row)`, and a column wall at `(row, col)` is the `x_min` side.

- `game_scene_trigger_spawn` returns `ENTITY_ID_INVALID` if the tile isn't on the map or the table is full. A scene can have up to `GAME_SCENE_TRIGGER_CAPACITY` (32).
- All the triggers share one collision proc. On a `begin` contact with Connor, it looks up the trigger's target and arrival and calls `game_scene_request`. Standing on a trigger doesn't keep asking, and a switch that's underway refuses anyway.
- The table is emptied whenever the scene switches.
- Never put an arrival on a trigger, or Connor walks straight back out. The dev scene and Cliff Town each arrive just inside the other's door.

### NPCs Across Scenes

The NPC manager is on the game context's global arena, so it outlives the scenes. An NPC's entity still belongs to the scene it was placed in, so the manager keeps the two parts apart:

- `game_npc_def` is who the NPC is: its name and sprites. The manager keeps this, and the way it's facing, across scenes.
- `game_npc_placement` is where it stands in a scene and how it behaves there: the map and tile, its roam range, its proc, and its collision proc.

`game_npc_spawn(def, at)` places the NPC in the bound scene. An NPC the manager doesn't know is added. One it kept from the last scene keeps its state, and gets a new entity with the same ID.

The NPCs passed to `game_scene_request` in `npc_keep` are kept through the switch. The rest are forgotten (`game_npc_mngr_reset_all_except`). A kept NPC that the next scene doesn't spawn stays in the manager without being placed, and isn't updated or drawn until a later scene spawns it again.

### The Dev Scene

The dev scene (`IFB.Game/src/game-scene-dev.cpp`, engine scene `DEV`) is the first scene. Its `enter` loads `DEV-MAP` and its parallax, and spawns the player rig, the four boxes, Rose, and a [trigger](#scene-triggers) on the wall along the top of the first chunk that goes to Cliff Town. The rig is created with `game_player_rig_init` and put on the map with `game_player_rig_place` (a map, chunk, and tile in the chunk, the same addressing as the boxes and NPCs) at the [arrival](#arrivals), or at chunk 0, tile 5, 5 between the boxes. `game_player_rig_place` can also be called again to move the rig. Its state is one `game_scene_dev` struct in its scene memory. The scene owns `DEV-MAP`, so the engine destroys it with the scene, and entering again can load it under the same name.

### Cliff Town

Cliff town (`IFB.Game/src/game-scene-cliff-town.cpp`, engine scene `CLIFF-TOWN`) loads the `CLIFF-TOWN` map (`assets/maps/cliff-town.json`) and the cliffs parallax, and places the player rig at the [arrival](#arrivals), or on the road across the middle of the map (chunk 0, tile 12, 12). Walking off the west end of the road, through a trigger on the map's edge, goes back to the dev scene. Its state is one `game_scene_cliff_town` struct in its scene memory. Switch to it with `game_scene_request(game_scene_id_e_cliff_town)`, by walking through the dev scene's door, or from **Game > Scenes** in the GUI.

### The Physics Test

The physics test (`IFB.Game/src/game-scene-physics.cpp`, `game_scene_id_e_physics`) is a stress test for [physics](physics.md). It builds a walled arena map (32 × 20 tiles in one chunk, with a ring of wood tiles under the walls) and fills it with quads that fly off in random directions and ricochet off of each other and the walls. The quads have no drag and no top speed, so only collisions change their speed. The walls are invisible, immovable boxes whose masks only collide with the quads.

Open it from **Game > Testing > Physics** in the GUI. While it's the active scene, the **Physics Test** window is open:

- **Respawn settings**: the quad count (up to `GAME_PHYS_TEST_QUAD_CAPACITY`, 1000), size, and speed range. The quads are spawned again with these when you ask.
- **Live settings**: the [restitution](physics.md#restitution) and mass range, which apply to the flying quads right away.
- **Readouts**: how many quads spawned and are still moving, their average and top speed, the kinetic energy against where it started (it stays flat with perfect bounces, so the drift shows how far the solver is off), and how many quads escaped the walls.
- **The physics step**: the [physics stats](physics.md#stats), with a plot of the step times.

The settings and readouts are in `game_phys_test` (`ifb-game.hpp`). `game_context::phys_test` points at it while the test is the active scene, and is NULL the rest of the time. The scene frames the arena with the camera every frame, and puts the camera back the way the last scene left it when it exits.

### The UI Test

The UI test (`IFB.Game/src/game-scene-ui-test.cpp`, `game_scene_id_e_ui_test`) has one of every [in-game UI](ui.md) control, wired to a preview swatch so you can see them work. It has no map or entities, only the screen layer. Open it from **Game > Testing > UI**. See [The UI Test Scene](ui.md#the-ui-test-scene).

### Switching from the GUI

**Game > Scenes** lists the dev scene and Cliff Town, and **Game > Testing** opens the physics test and the UI test, with the active one checked. The items are disabled while a switch is underway. The GUI only gets a const `game_context`, so it doesn't switch itself: `gui_render` returns the scene that was picked (`game_scene_id_e_none` if none was), and the game's render proc passes it to `game_scene_request` (`game-win32-main.cpp`). It's a fresh switch that keeps no NPCs, and it happens at the end of the next game update. Picking the active scene enters it again from the start.

A new scene needs a label in `gui_strings` and in `scene_labels` in `gui_main_window` (`gui.cpp`). A `static_assert` on `game_scene_id_e_count` there catches a missing one.

The game context's global arena only holds what lasts the whole game: the context, sprites, audio, the dialog, and the NPC manager. `game_context::player_rig` and `::map` point at the active scene's, and are NULL when it doesn't have one. `::phys_test` is only set while the physics test is active.

## How It Works

The scene manager (`scene_mngr`, `scene-mngr.cpp`) is created with the other managers and started last in `eng_context_startup`, after the memory manager, since every scene needs an arena.

| Piece | Description |
| --- | --- |
| **Table** | One slot per scene, indexed by its handle: name, arena, parallax, whether it's updating or rendering this frame, and whether a reset or destroy is pending. |
| **Running stack** | The stack the current frame runs with. |
| **Next stack** | The stack the requests change. It's copied to the running stack at the start of the next frame. |
| **Bound scene** | The scene new entities and emitters are stamped with. |

Entities store their scene in the entity manager's dense arrays, next to their ID and tag, and particle emitters store it in the emitter. The systems that skip non-running scenes check it with `entity_is_updating` and `entity_is_rendering`, or `scene_is_updating` for emitters.

The public API is in the **SCENES** section of `ifb-engine.hpp`, and the wrappers are in `src/engine/eng-scene.cpp`.
