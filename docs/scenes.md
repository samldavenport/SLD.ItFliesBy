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
| Memory | `eng_scene_alloc(h_scene, size)` | The scene's arena is reset |
| Parallax | `eng_scene_set_parallax`, `eng_parallax_set` | Cleared |

`eng_entity_get_scene(id)` returns the scene that owns an entity, and `eng_scene_get_entity_count(h_scene)` counts them.

Each scene has its own arena from the [arena](memory.md) pool. `eng_scene_alloc` pushes onto it, so anything the game keeps for a scene's lifetime, like its state structs, should be allocated there instead of from a global arena.

Not everything is owned by a scene yet:

- **Maps** are loaded globally and stay loaded when a scene is destroyed.
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

The engine doesn't know about the game's own logic, so the game has to check too. Wrap each scene's game code in a check like the gameplay scene does:

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

The game puts its level in a scene named `GAMEPLAY` (`IFB.Game/src/game-scene-gameplay.cpp`):

- `game_scene_gameplay_init` creates and pushes the scene, allocates the player rig, map, and box entity structs with `eng_scene_alloc`, loads the map and its parallax, and spawns the rig and boxes.
- `game_scene_gameplay_update_and_render` returns early when the scene isn't running.

The game context's global arena only holds what lasts the whole game: the context, sprites, and audio.

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
