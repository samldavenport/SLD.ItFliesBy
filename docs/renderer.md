# Renderer

The renderer (`IFB.Engine/src/renderer`, `eng-renderer.hpp`) draws the map, entity quads, particles, immediate quads and text, and the debug grid with OpenGL. It also sets up the OpenGL context and ImGui.

## How Drawing Works

The game doesn't draw directly. Each frame it tells the renderer **what** to draw, and the renderer draws it all at once at the end of the frame.

| Call | What it adds |
| --- | --- |
| `eng_map_render(h_map)` | Sets the map to draw. The map stays set until it's changed. |
| `eng_entity_render(id)` | Adds the entity's quad, if it has every component in `ENTITY_ARCHETYPE_SPRITE` (see [Quads](quads.md#quad-entities)). Also adds its particle emitter, if it has one. |

These go into the renderer's **draw lists** (`renderer-draw-lists.cpp`). Entities have to be pushed again every frame: the lists are cleared after each draw, but the map stays set.

The game can also draw quads and text that aren't entities, like a dialog box or a HUD, with the `eng_draw_*` functions. See [Immediate Drawing](#immediate-drawing).

## Frame

`renderer_draw_buffers` runs from the [frame loop](architecture.md#frame-loop) on frames that render, after physics and particles have updated:

| Order | Step | Description |
| --- | --- | --- |
| 1 | Sync parallax | Load the parallax layers if the [scene parallax](scenes.md#parallax) to draw has changed. |
| 2 | Lists → buffers | Pack the map's tiles into the tile buffer, and the entity quads and particles into the quad vertex buffer. |
| 3 | Parallax | Draw the parallax backdrop behind the map, if there's a map and a parallax. |
| 4 | Tiles | Draw the map with the tile shader. |
| 5 | Grid | Draw the debug grid, if it's visible. |
| 6 | Quads | Draw entity quads, then particles, with the quad shader. |
| 7 | World immediate quads | Draw the [world layer](#immediate-drawing) with the quad shader and the camera. |
| 8 | Screen immediate quads | Draw the [screen layer](#immediate-drawing) over everything, with an orthographic projection. |
| 9 | Particle view | Draw the offscreen [particle view](particles.md#particle-view), if it was requested this frame. |
| 10 | Map view | Draw the offscreen [map view](#map-view), if it was requested this frame. |
| 11 | Reset | Clear the draw lists for the next frame. |

After that, the engine calls the GUI render callback, so ImGui draws on top of everything, and presents the frame.

## Depth

The renderer mostly uses draw order instead of the depth buffer:

- **Map tiles** get a real depth test, because walls stand up off the floor and have to hide each other.
- **The grid** is tested against the map's depth, so walls hide it.
- **Quads** always pass the depth test. They're sorted back to front by distance from the camera before they're drawn, so nearer quads cover farther ones. The order barely changes between frames, so the sort is an insertion sort.
- **Particles** are drawn after the quads, so they're always on top.
- **Immediate quads** aren't sorted. Each layer draws in the order the quads were pushed, so later ones are on top. The world layer draws after the particles, and the screen layer draws last.

## Quads

Entity quads (`src/quads/quad.cpp`, `eng-quad.hpp`) are built on the CPU every frame:

- A quad is centered on its entity's position and sized by its `cmpnt_quad`.
- It tilts toward the camera's pitch, but its horizontal axis stays locked to world x, so turning the camera left or right doesn't rotate it.
- It pivots around its bottom edge, so its base stays on the ground and only the top tilts.
- If the entity's sprite is found, the quad samples that sprite's rectangle in the active atlas, tinted by the entity's color. Otherwise it's drawn in the entity's color.

The number of quads is limited by `IFB_CONFIG_QUAD_MAX`, and particles share that same buffer. See [Quads](quads.md) for details.

## Immediate Drawing

Immediate drawing (`renderer-immediate.cpp`, `eng-draw.cpp`) is for things that aren't entities and only need to be drawn, like a dialog box, a HUD, or a marker in the world. The game pushes quads during its callback, and they're drawn that frame. Nothing is kept: to keep something on screen, push it every frame.

There are two layers:

| Layer | Coordinates | Drawn |
| --- | --- | --- |
| **Screen** | Screen pixels. `(0, 0)` is the top left and y goes down. | Last, over everything else in the window. ImGui still draws on top of it. |
| **World** | World units, placed with a `mat4`. | With the camera, after the entity quads and particles. |

Both layers draw with the [quad shader](quads.md#color-and-sprites), so a quad can be a solid color, a sprite from the **active atlas**, or a glyph from the **active font**, and they can be mixed in any order. Each layer holds `IFB_CONFIG_QUAD_MAX` (1024) quads. A push that doesn't fit returns `false` and draws nothing.

The game callback can run more than once in a frame (see [Frame Loop](architecture.md#frame-loop)), so the layers are cleared before each run, and the frame draws what the last run pushed. Only quads pushed from the game callback are drawn. Anything pushed from the GUI render callback is cleared before the next draw.

### Screen Size

By default one screen pixel is one window pixel. `eng_draw_set_screen_size(width, height)` lays the screen layer out at a fixed size instead, and stretches it to fill the window, so a pixel art UI can be laid out at something like 320 × 180 no matter how big the window is. `eng_draw_set_screen_size(0, 0)` goes back to the window's size, and `eng_draw_get_screen_size` returns the size in use.

The stretch isn't held to whole multiples or to the screen's aspect ratio, so pick a size with the window's aspect ratio to keep pixels square.

### Functions

| Function | Description |
| --- | --- |
| `eng_draw_rect_screen(rect, color)` | A solid rectangle |
| `eng_draw_sprite_screen(rect, h_atlas, i_sprite, tint)` | A whole sprite, stretched to the rectangle |
| `eng_draw_sprite_region_screen(rect, h_atlas, i_sprite, region, tint)` | Part of a sprite. `region` is in the sprite's own pixels from its top left, for things like nine slice borders. |
| `eng_draw_text_screen(fnt, str, len, pos, color)` | Text, see [Text](#text) |
| `eng_draw_rect_world(xform, color)` | A solid 1 × 1 quad on the xy plane, centered on the origin and placed by `xform` |
| `eng_draw_sprite_world(xform, h_atlas, i_sprite, tint)` | The same quad with a sprite |
| `eng_draw_set_screen_size` / `_get_screen_size` | See [Screen Size](#screen-size) |

Rectangles are `draw_rect` (`ifb.hpp`): `x` and `y` are the top left, then the width and height. A sprite draw returns `false` and draws nothing if `h_atlas` isn't the active atlas, since the shader can only sample the active one.

```cpp
// a box along the bottom of a 320 x 180 screen
eng_draw_set_screen_size(320, 180);

const draw_rect box = { 8.0f, 124.0f, 304.0f, 48.0f };
eng_draw_rect_screen(box, color_rgba_f32(0x202030FF));
eng_draw_text_screen(fnt, "Hello!", 6, { box.x + 8.0f, box.y + 6.0f }, color_rgba_f32(0xFFFFFFFF));
```

### Text

Text is drawn one quad per glyph, from the font's atlas. The font has to be on the GPU and active first (see [Fonts](#fonts)), or `eng_draw_text_screen` returns `false`.

- `pos` is the **top left of the first line**. The first baseline is the font's `ascent` below it.
- `len` is how many characters of `str` to draw, so a typewriter effect is drawing the same string with a growing `len`.
- A newline moves down by the font's `line_height` and back to `pos.x`.
- Characters the font doesn't have are skipped. Spaces only move the pen.
- Kerning is applied between neighboring characters.
- Pixel fonts (baked without oversampling) are snapped to whole screen pixels so they stay sharp.

Layout is done separately, and doesn't need the font on the GPU:

| Function | Description |
| --- | --- |
| `eng_font_measure_text(fnt, str, len)` | The width of the widest line |
| `eng_font_wrap_text(fnt, str, len, max_width, lines, line_capacity)` | Breaks the text into lines no wider than `max_width`. Returns how many lines it needs, and writes the first `line_capacity` of them. |

Each wrapped line is a `font_line` (`ifb.hpp`): a range of the original string (`start`, `length`) and its width. Lines break at spaces when they can and always at newlines. A word wider than `max_width` is broken where it runs out of room. The spaces a line breaks on aren't part of either line.

Wrap the whole text before revealing any of it. If text is wrapped as it's revealed, a long word starts at the end of one line and then jumps to the next when it no longer fits.

```cpp
font_line lines[8];
const u32 line_count = eng_font_wrap_text(fnt, msg, msg_len, box.width - 16.0f, lines, 8);

vec2 pen = { box.x + 8.0f, box.y + 6.0f };
for (u32 i = 0; i < line_count && i < 8; ++i) {
    eng_draw_text_screen(fnt, &msg[lines[i].start], lines[i].length, pen, color_rgba_f32(0xFFFFFFFF));
    pen.y += fnt->line_height;
}
```

## Fonts

Fonts are uploaded to the GPU separately from loading them, since a font is a blob on the game's own arena (see [Font Atlases](assets.md#font-atlases)):

```cpp
font* fnt = eng_font_atlas_load_to_arena("UI-FONT", h_arena);
eng_font_upload(fnt);
eng_font_set_active(fnt);
```

| Function | Description |
| --- | --- |
| `eng_font_upload` | Put the font's atlas on the GPU. Uploading it again does nothing. `false` if every slot is taken. |
| `eng_font_release` | Take it off the GPU. If it was active, no font is active afterwards. |
| `eng_font_set_active` | The font text draws with. `false` if it isn't uploaded. |
| `eng_font_get_active` | The active font, `NULL` for none |

The renderer has `RENDERER_FONT_CAPACITY` (4) font slots, and finds fonts by their address. Release a font before the arena it's on is freed or reset, or a later font at the same address will be mistaken for it.

The atlas is uploaded as a single red channel texture, one coverage byte per pixel, so nothing is converted or copied first. Fonts baked without oversampling (pixel fonts) are filtered nearest, so they stay sharp when the screen layer is stretched. Oversampled fonts are filtered linearly.

Only one font is active at a time, the same as sprite atlases, so all the text in a frame uses one font.

## Parallax

The parallax backdrop is three image layers (back, mid, front) drawn on one quad that stands just behind the far edge of the map, with the parallax shader. Each layer scrolls at its own speed as the camera moves sideways, so the back layer looks farthest away. It's drawn first, so the map and everything else draws over it. Layer speeds, sizes, and the backdrop's depth are constants at the top of `renderer-parallax.cpp`.

The parallax belongs to a scene. `eng_parallax_set` sets it on the bound scene, and the renderer draws the one from the highest rendering scene that has one. See [Scenes](scenes.md#parallax).

## Map Tiles

The map is drawn with `assets/shaders/tile-vert.glsl` and `tile-frag.glsl`. Each tile is sent as a packed `map_tile` value plus its sprite rectangle, and the vertex shader builds the quad from the tile's index and the map's origin, columns, and tile size. Walls are stood up on their edges. See [Maps](maps.md#rendering).

## Sprite Atlases

Sprite atlases are uploaded to the GPU as textures when they're loaded (`eng_sprite_atlas_load`) and released when they're unloaded. The renderer has `IFB_CONFIG_SPRITE_TABLE_CAPACITY` (4) atlas texture slots, the same as the number of atlases the sprite manager can have loaded.

Only **one atlas is active** at a time, and the tile and quad shaders both draw from it. `eng_sprite_atlas_get_active` returns it, and `eng_sprite_atlas_get_texture` returns an atlas's OpenGL texture, for showing its sprites in the GUI:

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

The projection is a perspective projection with a 30° vertical field of view, a near plane of `0.01`, and a far plane of `100`, using the window's aspect ratio (see [Projection](#projection) for a quirk in it). The game moves the camera to follow the player (`game_player_rig_update_camera`), and **Renderer > Camera** in the GUI lets you edit it directly.

### Projection

`renderer_viewport_get_xform` starts from `mat4_identity()` and doesn't zero `r3c3`, so a point's `w` is its depth in front of the camera **plus one**. Everything is drawn as if it were one unit farther away than it is, which makes nearby things noticeably smaller than a 30° field of view would. The game's camera distances are tuned with this in place, so fixing it would change how the game looks. Anything that maps between the screen and the world has to use the actual matrix, not the field of view.

## Shaders

Shaders are loaded from the working directory at startup. CMake copies `assets/shaders/*.glsl` next to the executable.

| Shader | Files | Draws |
| --- | --- | --- |
| Tile | `tile-vert.glsl`, `tile-frag.glsl` | Map floor tiles and walls |
| Quad | `quad-shader-vertex.glsl`, `quad-shader-fragment.glsl` | Entity quads, particles, and immediate quads and text. The active atlas is in texture slot 0 and the active font in slot 1. |
| Parallax | `parallax-vert.glsl`, `parallax-frag.glsl` | The parallax backdrop |
| Grid | `grid-vert.glsl`, `grid-frag.glsl` | The debug grid |
| Direction gizmo | `direction-gizmo-shader-vert.glsl`, `direction-gizmo-shader-frag.glsl` | An axis gizmo. It's set up at startup, but its draw call is currently commented out. |

## Map View

The map view (`renderer-map-view.cpp`) is an offscreen render of one map, with its own camera, for tools like the [map builder](maps.md#map-builder). It works like the particle view: a framebuffer with a color texture and a depth buffer, drawn only on frames it's requested, and shown in the GUI with `ImGui::Image`.

It draws with the same tile shader, active atlas, and projection as the game, so a map looks the way it does in the game. It packs its map into the same tile buffer the game's map uses, so it draws after the game's map, which is packed again next frame.

| Function | Description |
| --- | --- |
| `eng_map_view_request(h_map, width, height)` | Draw the map into the view at the end of this frame, resizing the texture if needed |
| `eng_map_view_get_texture` | The view's OpenGL texture. It's drawn upside down, so show it with the UVs flipped, `(0, 1)` to `(1, 0)`. |
| `eng_map_view_get_camera` / `_set_camera` | The view camera's origin and target |
| `eng_map_view_get_xform` | The view projection for the size the view was last drawn at, for projecting world points into the view |
| `eng_map_view_set_grid_visible` | Draw the debug grid in the view |

To find what's under a point in the view, solve with the view projection itself rather than building a ray from the camera's field of view, because the projection isn't a textbook one (see [Projection](#projection)). A point lands on a spot in the view when it's on two planes that come straight from the matrix's rows, so intersecting those with the plane you're picking on gives the point. `xform_viewport_to_plane` in SLD.Math does this, without inverting the matrix, and `xform_world_to_viewport` goes the other way. The map builder picks floor tiles with them in `map_builder_screen_to_floor`. See the [SLD.Math README](../SLD.Math/README.md#screen-space) for how it works.

## Other Features

| Function | Description |
| --- | --- |
| `eng_renderer_is_grid_visible` / `_set_grid_visible` | Toggle the debug grid |
| `eng_particle_view_request` | Draw the offscreen particle view this frame (see [Particles](particles.md#particle-view)) |
| `eng_map_view_request` | Draw the offscreen map view this frame (see [Map View](#map-view)) |

The renderer also has `RENDERER_ASSET_PREVIEW_COUNT` (5) textures, one for each GUI window that previews assets.

## Memory

The renderer allocates its draw lists and CPU-side buffers, including the two immediate layers, from the `rendering` reservation in the memory map. The OpenGL buffers are created at startup at the same size as the CPU buffers, so nothing is resized while running.
