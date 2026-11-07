# Maps

Maps (`IFB.Engine/src/map`, `eng-map.hpp`) are the tile grids that entities stand on. A map is made of **chunks**, rectangular areas of floor tiles. **Walls** can be placed on the edges between tiles.

## Coordinates

A map is a grid of `count_rows × count_cols` tiles, placed in the world at its origin (`eng_map_set_origin`). Each tile is `IFB_CONFIG_MAP_TILE_UNIT_SIZE` (0.2) world units on a side.

| Map axis | World axis | Field in `cmpnt_map_coords` |
| --- | --- | --- |
| Column | x | `col_x` |
| Level | y | `lvl_y` |
| Row | z | `row_z` |

Coordinates are floats in tile units, so `col_x = 2.5` is halfway across column 2. Converting between them:

```
position    = origin + coords * tile_size
coords      = (position - origin) / tile_size
```

`eng_map_get_pos_from_coords` and `eng_map_get_coords_from_pos` do these conversions. Neither one checks the map bounds. Use `eng_map_is_in_bounds` for that.

### Entities on a Map

An entity belongs to a map if it has a `cmpnt_map_coords` component. The component's `h_map` says which map it's on. The **position component is the source of truth** for where the entity is:

- Reading an entity's map coordinates calculates them from its position.
- Updating them with `eng_cmpnt_update_map_coords` moves the entity's position to match.

Physics runs in world space, and entities on a map are kept inside it.

## Creating a Map

```cpp
hnd_map h_map = eng_map_create("dev-map", 25, 25);

// chunk: 10 × 10 tiles starting at row 0, column 0
u32 i_chunk = eng_map_chunk_create(h_map, 10, 10, 0, 0, map_color_e_none);

eng_map_chunk_set_sprite(h_map, i_chunk, "CORE-SPRITES", "GRASS");
eng_map_set_tile_sprite (h_map, 0, 0,    "CORE-SPRITES", "WOOD");

// a row wall along the top edge of the chunk
for (u32 col = 0; col < 10; ++col) {
    eng_map_wall_set(h_map, 0, col, map_wall_orientation_e_row, map_color_e_gray_dark);
}

// every frame
eng_map_render(h_map);
```

Maps can also be described in a JSON file and loaded from the asset database with `eng_map_load("DEV-MAP")`, which is how the game gets its map (`IFB.Game/src/game-map.cpp`). See [Map Format](map-format.md).

### Loading a Map Twice

A map's handle is the hash of its name, so two maps can't have the same name. `eng_map_load_as(asset_name, map_name)` loads a map asset under a different name, which is how the map builder gets its own copy of a map the game has loaded. `eng_map_destroy` frees a map's slot so it can be used again.

## Chunks

`eng_map_chunk_create(h_map, count_rows, count_cols, origin_row, origin_col, base_color)` adds a chunk and returns its index, or `INVALID_INDEX` if it doesn't fit.

- The chunk has to fit inside the map, and its tiles and walls have to fit in the map's tile and wall arrays.
- Every tile in a new chunk is **navigable**. Tiles that aren't covered by a chunk aren't navigable.
- Overlapping chunks aren't checked yet, so don't create chunks that overlap.
- Each chunk can only use sprites from **one sprite atlas**. Setting a tile or wall sprite from a different atlas than the chunk already uses fails. `eng_map_chunk_set_sprite` sets every tile in the chunk and replaces the chunk's atlas.

`eng_map_get_navigable_bounds` returns the smallest tile area that contains every chunk.

## Floor Tiles

Each floor tile (`map_floor_tile`) has:

| Field | Description |
| --- | --- |
| `i_sprite` | Sprite index in the chunk's atlas, or `INVALID_INDEX` for a solid color |
| `color` | A `map_color_e` color |
| `flags` | `map_tile_flag_e`: navigable, textured |

## Walls

Walls lie on the edges between tiles rather than on the tiles themselves:

| Orientation | Lies on | Runs along | Position |
| --- | --- | --- | --- |
| `map_wall_orientation_e_row` | the line `z = row` | x | The min-z edge of tile `(row, col)` |
| `map_wall_orientation_e_column` | the line `x = col` | z | The min-x edge of tile `(row, col)` |

Because walls sit on edges, there's one more line of them than tiles: `row` can be `count_rows` for the far row edge, and `col` can be `count_cols` for the far column edge.

| Function | Description |
| --- | --- |
| `eng_map_wall_set` | Place a solid color wall |
| `eng_map_wall_set_sprite` | Give a wall a sprite |
| `eng_map_wall_clear` | Remove a wall |
| `eng_map_wall_is_placed` | Check if a wall is placed |

A wall only exists if it's placed (`map_wall_flag_e_placed`).

## Colors

Maps use a 16 color table (`map_color_e`) with a light and dark shade of red, orange, yellow, green, aqua, blue, purple, and gray. `map_color_e_none` means no color. The table is passed to the tile shader as `u_color_table`.

## Movement Constraints

`map_constrain_movement` keeps an entity's box on the navigable tiles of its map and out of its walls. Physics calls it after integration and after collision resolution, so the map always has the final say. Everything happens on the x/z plane, in tile units:

1. The entity is a square box centered on its position, extending the **inset** (half its quad width) in each direction.
2. If its position isn't on a navigable tile, it's first moved to the edge of the nearest chunk.
3. Up to `MAP_CONSTRAIN_ITERATION_COUNT` (8) times: find the tile or wall the box overlaps the most, and push it out the shortest way. Resolving the biggest overlap first keeps the box from catching on the seams between tiles along an edge.
4. Move the position by the total push, and zero the velocity on any axis it was pushed against.

Non-navigable tiles are solid squares. Walls are slabs `MAP_WALL_HALF_THICKNESS` (0.05 tiles) thick on each side of their edge, extended past their ends by the same amount so corners are closed.

## Rendering

`eng_map_render` sets the map the renderer draws. The map is packed into a buffer of `map_tile` values (`map_render_buffer`), floor tiles first and walls after. The tile shaders (`assets/shaders/tile-vert.glsl`, `tile-frag.glsl`) build a quad for each tile and stand walls up on their edges. Textured tiles sample the chunk's sprite atlas, and the rest use the color table.

## Map Builder

**Engine > Tools > Map Builder** (`IFB.GUI/src/gui-engine-map-builder.cpp`) previews and edits a map the way the game draws it. So far it can paint floor tiles. It can't save yet, so changes are lost when the map is loaded again.

- Pick a map asset (any text asset in the runtime database) and press **Load**. The builder loads its own copy as `MAP-BUILDER` with `eng_map_load_as`, so the game's map isn't touched. Once you've painted, the button says **Reload**, and pressing it throws the changes away.
- The view is the [map view](renderer.md#map-view), with the same tile shader, atlas, and projection as the game.
- **Frame Map** looks down at the middle of the map's chunks. **Game View** puts the camera at the game camera's angle and distance.

### Tools

| Tool | Left mouse |
| --- | --- |
| **Move** | Drag to pan |
| **Brush** | Click or drag to paint tiles |

With either tool, **middle drag** pans, **right drag** orbits around the point the camera looks at, and the **mouse wheel** zooms. Panning keeps the spot you grabbed under the mouse.

### Painting Tiles

The palette on the left shows the sprites in an atlas, picked from the atlases in the runtime database. It starts on the active atlas, which is the only one the map draws with. Click a sprite to paint with it, or **No sprite** to clear tiles back to their chunk's color, like `-1` in a [map file](map-format.md). Picking from the palette switches to the Brush tool.

The brush paints with the same calls as the map loader, so the view shows exactly what the game would:

- Only tiles on a chunk can be painted. The tile under the mouse is outlined in yellow on a chunk and red off one, and the line above the view shows its row, column, chunk, and sprite.
- A chunk's sprites all come from one atlas, so painting a sprite from a different atlas onto a chunk fails, and the builder says why.
- A cleared tile on a chunk whose color is `none` draws black.

| Function | Description |
| --- | --- |
| `eng_map_get_tile_info` | A floor tile's chunk, atlas, sprite, and color, or `false` if no chunk covers it |
| `eng_map_clear_tile_sprite` | Take a tile's sprite off, so it shows its chunk's color |

## Configuration

| Setting | Default | Description |
| --- | --- | --- |
| `IFB_CONFIG_MAP_CAPACITY` | 64 | Maximum number of maps |
| `IFB_CONFIG_MAP_CHUNK_CAPACITY` | 64 | Maximum chunks per map |
| `IFB_CONFIG_MAP_TILE_CAPACITY` | 4096 | Maximum floor tiles per map |
| `IFB_CONFIG_MAP_WALL_CAPACITY` | 8192 | Maximum walls per map, for each orientation |
| `IFB_CONFIG_MAP_TILE_UNIT_SIZE` | 0.2 | Size of a tile in world units |
| `IFB_CONFIG_MAP_RENDER_BUFFER_SIZE` | 400 KB | Size of the map render buffer |
