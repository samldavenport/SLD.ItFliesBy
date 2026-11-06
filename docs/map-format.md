# Map Format

Maps are stored as JSON files in `assets/maps/`, and the game loads them from the asset database as text assets (see [Loading](#loading)). A map file describes exactly what the `eng_map_*` API can build: the map's size, its chunks, the sprites on their floor tiles, and the walls on tile edges. Loading a map file is the same as making those API calls in order (see [Loading](#loading)).

This is version **1** of the format. It only covers what the engine supports today. See [Not in Version 1](#not-in-version-1) for what's left out.

## Example

```json
{
    "version": 1,
    "name":    "dev-map",
    "rows":    25,
    "cols":    25,
    "origin":  [0.0, 0.0, 0.0],

    "chunks": [
        {
            "row": 0, "col": 0, "rows": 2, "cols": 3,
            "color":   "none",
            "atlas":   "CORE-SPRITES",
            "sprites": ["GRASS", "WOOD"],
            "tiles": [
                1, 0, 0,
                0, 0, 0
            ]
        },
        {
            "row": 2, "col": 0, "rows": 2, "cols": 3,
            "color": "gray_dark"
        }
    ],

    "walls": [
        { "orientation": "row",    "row": 0, "col": 0, "length": 3, "color": "gray_dark"  },
        { "orientation": "column", "row": 0, "col": 0, "length": 2, "color": "gray_light" }
    ]
}
```

`assets/maps/dev-map.json` is the map the game loads, as the text asset `DEV-MAP`.

## Map

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `version` | integer | yes | The format version. Always `1` for now. |
| `name` | string | yes | The map's name, passed to `eng_map_create`. It has to fit the engine's 16 character name buffer. |
| `rows` | integer | yes | Number of tile rows (the z axis). |
| `cols` | integer | yes | Number of tile columns (the x axis). |
| `origin` | `[x, y, z]` | no | Where the map sits in the world, passed to `eng_map_set_origin`. Defaults to `[0, 0, 0]`. |
| `chunks` | array | yes | The map's chunks. See [Chunks](#chunks). |
| `walls` | array | no | The map's walls. See [Walls](#walls). |

## Chunks

Each entry creates one chunk. A chunk's index is its position in the array, so the first chunk is chunk 0.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `row`, `col` | integer | yes | The chunk's top-left tile in the map (`origin_row`, `origin_col`). |
| `rows`, `cols` | integer | yes | The chunk's size in tiles. |
| `color` | [color](#colors) | yes | The chunk's base color. Every tile without a sprite is drawn in this color. |
| `atlas` | string | with `sprites` | The sprite atlas the chunk's sprites come from. A chunk can only use one atlas. |
| `sprites` | array of strings | no | The sprite names the chunk uses, in `atlas`. `tiles` refers to them by index. |
| `tiles` | array of integers | with `sprites` | One entry per tile, `rows × cols` long, row-major (the first `cols` entries are the chunk's first row). Each entry is an index into `sprites`, or `-1` for a tile with no sprite. |

A chunk with no `sprites` is a solid color, and `atlas` and `tiles` are left out.

Sprites are stored by name rather than by their index in the atlas, so rebuilding an atlas doesn't break the maps that use it. Write `tiles` one chunk row per line so the file reads like the grid.

## Walls

Walls lie on the edges between tiles (see [Maps](maps.md#walls)). Each entry is a **run**: a straight line of walls that share an orientation and a look.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `orientation` | `"row"` or `"column"` | yes | Which edges the run lies on. |
| `row`, `col` | integer | yes | The first wall in the run. |
| `length` | integer | no | How many walls are in the run. Defaults to `1`. |
| `color` | [color](#colors) | yes | The wall color. |
| `atlas`, `sprite` | string | no | A sprite for every wall in the run. It has to come from the same atlas as the chunk each wall belongs to. |

Which way a run goes depends on its orientation:

| Orientation | Lies on | Covers |
| --- | --- | --- |
| `row` | the line `z = row` | `(row, col)` to `(row, col + length - 1)` |
| `column` | the line `x = col` | `(row, col)` to `(row + length - 1, col)` |

Walls are given in map coordinates, not chunk coordinates. The engine finds the chunk that owns each edge, so a run can cross from one chunk to the next. Like `eng_map_wall_set`, `row` can be `rows` for the map's far row edge and `col` can be `cols` for its far column edge.

## Colors

Colors are the names of the `map_color_e` values without the prefix:

| | Light | Dark |
| --- | --- | --- |
| Red | `red_light` | `red_dark` |
| Orange | `orange_light` | `orange_dark` |
| Yellow | `yellow_light` | `yellow_dark` |
| Green | `green_light` | `green_dark` |
| Aqua | `aqua_light` | `aqua_dark` |
| Blue | `blue_light` | `blue_dark` |
| Purple | `purple_light` | `purple_dark` |
| Gray | `gray_light` | `gray_dark` |

`none` means no color.

## Loading

A map file is a **text asset**. To add one:

1. Put the file in `assets/maps/`.
2. Add it to the `text` list in `assets/asset-dictionary.json`, e.g. `{ "name": "DEV-MAP", "path": "maps/dev-map.json" }`. The name follows the usual [asset name](assets.md#names-and-handles) rules.
3. Rebuild the asset database and commit it, like any other asset change (see [Assets](assets.md#asset-database)). Editing a map file does nothing in the game until the database is rebuilt.

Then load it by its asset name, after the atlases it uses:

```cpp
hnd_map h_map = eng_map_load("DEV-MAP");
```

`eng_map_load` reads the text from the runtime database into a temporary arena and hands it to `map_parse_from_json` (`IFB.Engine/src/map/map.cpp`), which builds the map with the same calls as these public functions, in this order:

1. `eng_map_create(name, rows, cols)`, then `eng_map_set_origin(origin)` if `origin` is given.
2. For each chunk, in order: `eng_map_chunk_create(rows, cols, row, col, color)`.
3. For each chunk with sprites, for each tile whose entry isn't `-1`: `eng_map_set_tile_sprite(row, col, atlas, sprites[entry])`, with the tile's map coordinates.
4. For each wall run, for each wall in it: `eng_map_wall_set(row, col, orientation, color)`, then `eng_map_wall_set_sprite` if it has a sprite.

Loading fails if there's no text asset by that name, the file doesn't match this format, or any call fails. On failure, `eng_map_load` returns `INVALID_HANDLE` and the partly built map is destroyed, so a failed load never leaves a map behind. The text and the parsed document are freed before it returns, whether it succeeds or not.

The rules the API enforces apply to the file too:

- Chunks have to fit inside the map, and mustn't overlap. Overlaps aren't checked yet.
- The map's total tiles and walls have to fit the capacities in `ifb-config.hpp` (`IFB_CONFIG_MAP_TILE_CAPACITY`, `IFB_CONFIG_MAP_WALL_CAPACITY`, `IFB_CONFIG_MAP_CHUNK_CAPACITY`).
- Every wall has to be on the edge of a chunk tile.
- The atlases a map uses have to be loaded before it is.
- The name has to be 1 to 15 characters, and `version` has to be `1`.
- Optional fields that are there have to have the right type. A `length` of `"2"` or an `origin` with two values fails instead of being ignored.
- An unknown color or orientation name fails.

## Not in Version 1

These aren't in the format because the engine doesn't support them yet. They can be added in later versions.

- **Per-tile colors.** Every tile without a sprite uses its chunk's color, because there's no API for setting one tile's color.
- **Levels.** The format is one floor, like the engine's maps.
- **Navigability.** Every chunk tile is navigable. There's no way to mark one as blocked except with walls.
- **Entities and spawn points.**
