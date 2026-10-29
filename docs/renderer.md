# Renderer

The renderer (`IFB.Engine/src/renderer`, `eng-renderer.hpp`) draws the map, entity quads, particles, and the debug grid with OpenGL. It also sets up the OpenGL context and ImGui.

## How Drawing Works

The game doesn't draw directly. Each frame it tells the renderer **what** to draw, and the renderer draws it all at once at the end of the frame.

| Call | What it adds |
| --- | --- |
| `eng_map_render(h_map)` | Sets the map to draw. The map stays set until it's changed. |
| `eng_entity_render(id)` | Adds the entity's quad, if it has every component in `ENTITY_ARCHETYPE_SPRITE` (see [Quads](quads.md#quad-entities)). Also adds its particle emitter, if it has one. |

These go into the renderer's **draw lists** (`renderer-draw-lists.cpp`). Entities have to be pushed again every frame: the lists are cleared after each draw, but the map stays set.

## Frame

`renderer_draw_buffers` runs from the [frame loop](architecture.md#frame-loop) on frames that render, after physics and particles have updated:

| Order | Step | Description |
| --- | --- | --- |
| 1 | Lists → buffers | Pack the map's tiles into the tile buffer, and the entity quads and particles into the quad vertex buffer. |
| 2 | Tiles | Draw the map with the tile shader. |
| 3 | Grid | Draw the debug grid, if it's visible. |
| 4 | Quads | Draw entity quads, then particles, with the quad shader. |
| 5 | Particle view | Draw the offscreen [particle view](particles.md#particle-view), if it was requested this frame. |
| 6 | Reset | Clear the draw lists for the next frame. |

After that, the engine calls the GUI render callback, so ImGui draws on top of everything, and presents the frame.

## Depth

The renderer mostly uses draw order instead of the depth buffer:

- **Map tiles** get a real depth test, because walls stand up off the floor and have to hide each other.
- **The grid** is tested against the map's depth, so walls hide it.
- **Quads** always pass the depth test. They're sorted back to front by distance from the camera before they're drawn, so nearer quads cover farther ones. The order barely changes between frames, so the sort is an insertion sort.
- **Particles** are drawn after the quads, so they're always on top.

## Quads

Entity quads (`src/quads/quad.cpp`, `eng-quad.hpp`) are built on the CPU every frame:

- A quad is centered on its entity's position and sized by its `cmpnt_quad`.
- It tilts toward the camera's pitch, but its horizontal axis stays locked to world x, so turning the camera left or right doesn't rotate it.
- It pivots around its bottom edge, so its base stays on the ground and only the top tilts.
- If the entity's sprite is found, the quad samples that sprite's rectangle in the active atlas, tinted by the entity's color. Otherwise it's drawn in the entity's color.

The number of quads is limited by `IFB_CONFIG_QUAD_MAX`, and particles share that same buffer. See [Quads](quads.md) for details.

## Map Tiles

The map is drawn with `assets/shaders/tile-vert.glsl` and `tile-frag.glsl`. Each tile is sent as a packed `map_tile` value plus its sprite rectangle, and the vertex shader builds the quad from the tile's index and the map's origin, columns, and tile size. Walls are stood up on their edges. See [Maps](maps.md#rendering).

## Sprite Atlases

Sprite atlases are uploaded to the GPU as textures when they're loaded (`eng_sprite_atlas_load`) and released when they're unloaded. The renderer has `IFB_CONFIG_SPRITE_TABLE_CAPACITY` (4) atlas texture slots, the same as the number of atlases the sprite manager can have loaded.

Only **one atlas is active** at a time, and the tile and quad shaders both draw from it:

```cpp
hnd_sprite_atlas h_atlas = eng_sprite_atlas_load("CORE-SPRITES");
eng_sprite_atlas_set_active(h_atlas);
```

The game sets `CORE-SPRITES` as the active atlas at startup (`game-context.cpp`).

## Camera

The camera is a look-at camera defined by an origin and a target:

| Function | Description |
| --- | --- |
| `eng_camera_get_origin` / `_set_origin` | Where the camera is |
| `eng_camera_get_target` / `_set_target` | The point the camera looks at |
| `eng_camera_get_fov` / `_set_fov` | Vertical field of view |
| `eng_camera_get_xform` | The view matrix |

The projection is a perspective projection with a 30° vertical field of view, a near plane of `0.01`, and a far plane of `100`, using the window's aspect ratio. The game moves the camera to follow the player (`game_player_rig_update_camera`), and **Renderer > Camera** in the GUI lets you edit it directly.

## Shaders

Shaders are loaded from the working directory at startup. CMake copies `assets/shaders/*.glsl` next to the executable.

| Shader | Files | Draws |
| --- | --- | --- |
| Tile | `tile-vert.glsl`, `tile-frag.glsl` | Map floor tiles and walls |
| Quad | `quad-shader-vertex.glsl`, `quad-shader-fragment.glsl` | Entity quads and particles |
| Grid | `grid-vert.glsl`, `grid-frag.glsl` | The debug grid |
| Direction gizmo | `direction-gizmo-shader-vert.glsl`, `direction-gizmo-shader-frag.glsl` | An axis gizmo. It's set up at startup, but its draw call is currently commented out. |

## Other Features

| Function | Description |
| --- | --- |
| `eng_renderer_is_grid_visible` / `_set_grid_visible` | Toggle the debug grid |
| `eng_particle_view_request` | Draw the offscreen particle view this frame (see [Particles](particles.md#particle-view)) |

The renderer also has `RENDERER_ASSET_PREVIEW_COUNT` (4) textures, one for each GUI window that previews assets.

## Memory

The renderer allocates its draw lists and CPU-side buffers from the `rendering` reservation in the memory map. The OpenGL buffers are created at startup at the same size as the CPU buffers, so nothing is resized while running.
