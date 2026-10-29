# Quads

A quad is the flat rectangle an entity is drawn as (`IFB.Engine/src/quads`, `eng-quad.hpp`). Every character and object in the world is a quad with a sprite or a solid color, standing up off the map. The [renderer](renderer.md) builds the vertices for every quad on the CPU each frame and draws them in one call.

## Quad Entities

There's no separate quad object. A quad is any entity with every component in `ENTITY_ARCHETYPE_SPRITE`:

| Component | Used for |
| --- | --- |
| `cmpnt_position` | Where the quad is (see [Placement](#placement)) |
| `cmpnt_quad` | Width and height |
| `cmpnt_color` | The solid color, or the tint for a sprite |
| `cmpnt_sprite` | The atlas and sprite index |
| `cmpnt_map_coords` | The map the entity is on |

```cpp
entity_id id = eng_entity_create("TREE");
eng_entity_add_components(id, ENTITY_ARCHETYPE_SPRITE);

atype_sprite s;
eng_atype_lookup_sprite(id, s);
s.quad  = { 0.2f, 0.4f };
s.color = 0xFFFFFFFF;            // white, so the sprite isn't tinted
eng_atype_update_sprite(id, s);

eng_entity_set_sprite(id, "CORE-SPRITES", "GRASS");

// every frame
eng_entity_render(id);
```

`game_entity_spawn_static_sprite` in `game-entity.cpp` does this for the game's static objects.

> **Note:** an entity is only drawn if it has **every** component in `ENTITY_ARCHETYPE_SPRITE`. `eng_entity_render` only checks for a position, quad, and color before it returns `true`, but the renderer's draw list skips an entity that's missing the map coordinates or sprite component. So an entity with only a position, quad, and color isn't drawn, even though `eng_entity_render` reports success.

### Reading a Quad

`eng_entity_quad_lookup(q, id)` fills a `quad_entity`, which has the entity's ID, tag, archetype, and indexes, plus its position, dimensions, color, and sprite in one struct. It returns `false` if the entity isn't a quad. **Entities > Quad Manager** in the GUI uses it to list every quad.

## Placement

A quad's **position is the center** of the quad. Inside the engine, though, the quad is drawn so its **bottom edge stays on the ground**:

- **Width** runs along world x. The quad's horizontal edge always stays parallel to the x axis.
- **Height** runs up from the bottom edge and tilts to match the camera's **pitch**, so the quad leans back as the camera looks down. Turning the camera left or right (**yaw**) doesn't rotate the quad.
- The bottom edge sits half the quad's height below the position, so it doesn't move when the quad tilts. Only the top moves.

[Physics](physics.md#bounding-boxes) and [map constraints](maps.md#movement-constraints) also treat the position as the center, using half the width and height.

## Color and Sprites

Each vertex carries a color, texture coordinates, and a `textured` flag. The fragment shader (`quad-shader-fragment.glsl`) uses them like this:

```glsl
vec4 text_color = texture(u_texture_0_sprite_atlas, vert_text_coords);
out_color       = vert_color * mix(vec4(1.0), text_color, vert_textured);
```

| Case | `textured` | Result |
| --- | --- | --- |
| The sprite is found in its atlas | `1` | The sprite, multiplied by the color. Use white (`0xFFFFFFFF`) to show the sprite as is. |
| The sprite can't be found | `0` | The solid color |

A sprite "can't be found" when the atlas isn't loaded or the sprite index is invalid, for example a sprite component that was never set.

The texture coordinates are the sprite's rectangle in the atlas, divided by the atlas size. The atlas isn't flipped when it's uploaded, so the top row is `v = 0`. Quads always sample the **active** atlas, so a quad whose sprite comes from a different atlas shows the wrong part of the active atlas's texture (see [Renderer](renderer.md#sprite-atlases)).

## Building the Vertex Buffer

Each frame, `quad_render_buffer` (`quad.cpp`) turns the renderer's quad list into vertices:

1. For each entity in the list, `quad_render` reads its components and writes four `quad_vertex` values (top right, bottom right, bottom left, top left) into a `quad_vertices`.
2. The quads are **sorted back to front** by the distance from the camera plane to each quad's center, so nearer quads draw over farther ones. The renderer doesn't depth-test quads, so this order decides what's in front. The order barely changes between frames, so the sort is an insertion sort, which is fast on nearly sorted data.

[Particles](particles.md#rendering) are written into the same buffer after the quads, so they always draw on top.

### Vertex Layout

| Field | Type | Shader input |
| --- | --- | --- |
| `position` | `vec3` | `in_pos` (location 0) |
| `color` | `color_rgba_f32` | `in_color` (location 1) |
| `text_coords` | `vec2` | `in_text_coords` (location 2) |
| `textured` | `f32` | `in_textured` (location 3) |

The element buffer never changes, so it's filled once at startup with two triangles per quad: vertices `0, 1, 3` and `1, 2, 3`.

The vertex shader (`quad-shader-vertex.glsl`) multiplies each position by `u_mat4_view_proj` and `u_mat4_model`. The model matrix is always the identity, since the vertices are already in world space.

## Capacity

The quad vertex buffer holds `IFB_CONFIG_QUAD_MAX` (1024) quads, shared with particles. That matches `IFB_CONFIG_ENTITY_CAPACITY`, so every entity can be drawn, but particles only get whatever room the entity quads leave.

## Internal Functions

| Function | Description |
| --- | --- |
| `quad_lookup_by_id` | Fill a `quad_entity` from an entity's components. `eng_entity_quad_lookup` wraps it. |
| `quad_update(quad_entity)` | Write a `quad_entity` back to the position, color, quad, and sprite components |
| `quad_update(id, cmpnt_quad)` | Write only the quad component |
| `quad_does_exist` | Check if an entity has every component in `ENTITY_ARCHETYPE_SPRITE` |
| `quad_render` | Write one quad's four vertices |
| `quad_render_buffer` | Write and sort the vertices for a list of quads |
| `quad_tests` | Creates 100 quads and updates them (`quad-tests.cpp`). Not called by the engine. |

`eng-quad.hpp` also defines a `quad_mngr` struct and declares `quad_tile`, but neither is used.
