# Assets

Assets go through two stages: an editable JSON **asset dictionary** that lists source files, and a binary **asset database** built from it that the game loads at runtime.

```
assets/images/*.png ─┐
assets/sprites/*.ifbatlas ─┼──> asset-dictionary.json ──(build)──> asset-database.ifbdb ──> runtime
assets/json/*.json ─┘
```

## Asset Types

| Type | `asset_type_e` | Runtime struct | Description |
| --- | --- | --- | --- |
| Text | `asset_type_e_text` | `asset_text` | Raw text, such as JSON. |
| Image | `asset_type_e_image` | `asset_image` | RGBA pixels, decoded with stb_image. |
| Sprite atlas | `asset_type_e_sprite_atlas` | `asset_sprite_atlas` | A packed image plus the name, position, and size of each sprite in it. |

## Names and Handles

Assets are referred to by name. Names are up to `IFB_CONFIG_ASSET_NAME_SIZE` (16) characters and stored uppercase in the dictionary (e.g. `CONNOR-LEFT`, `CORE-SPRITES`). An asset's `hnd_asset` is the hash of its name, so the same name always gives the same handle.

## Asset Dictionary

`assets/asset-dictionary.json` maps names to file paths, grouped by type:

```json
{
    "text": [],
    "image": [
        { "name": "GRASS", "path": "images/grass.png" }
    ],
    "sprite_atlas": [
        { "name": "CORE-SPRITES", "path": "sprites/core-sprites.ifbatlas" }
    ]
}
```

Edit it in the GUI under **Game > Assets > Asset Dictionary**, or through the API:

| Function | Description |
| --- | --- |
| `eng_asset_dictionary_new` / `_load` / `_save` | Create, load, or save a dictionary |
| `eng_asset_dictionary_add_file` / `_remove_file` | Add or remove an entry |
| `eng_asset_dictionary_get_file_count` / `_get_file_info` | List the entries of a type |
| `eng_asset_dictionary_load_preview` | Load a text or image preview for the GUI |

## Asset Database

`eng_asset_database_build(path_dictionary, path_database)` reads every file in the dictionary and writes them to a single `.ifbdb` file, with an index (`asset_index`: type, offset, size) for each asset.

At startup the engine opens `IFB_CONFIG_ASSET_DATABASE_PATH` (`asset-database.ifbdb`) from the working directory. CMake copies `assets/*.ifbdb` into `build/<preset>/bin`, so **rebuild the database and commit it** after changing the dictionary or any source asset.

The **Game > Assets > Asset Database** window can build, open, and preview a database. `eng_asset_database_copy_to_runtime` replaces the database the running game is using with the open one.

## Runtime Memory

The asset manager loads assets from the database on demand, into blocks sorted by size:

| Partition | Block size | Partitions | Blocks per partition |
| --- | --- | --- | --- |
| Small | 64 KB | 32 | 64 |
| Medium | 1 MB | 48 | 256 |
| Large | 64 MB | 1 | 64 |

These values come from `IFB_CONFIG_ASSET_*` in `ifb-config.hpp`.

## Sprite Atlases

A sprite atlas (`.ifbatlas`) packs several image assets into one texture. Build them with **Engine > Sprite Tools > Sprite Atlas Builder**:

1. Add image assets by name (`eng_sprite_atlas_builder_add`).
2. Pack them (`eng_sprite_atlas_builder_pack`) and check the preview.
3. Save the atlas (`eng_sprite_atlas_builder_save`), add it to the asset dictionary, and rebuild the database.

**Engine > Sprite Tools > Sprite Atlas Inspector** opens an existing atlas and lists its sprites.

At runtime, load an atlas by name and look up sprites in it:

```cpp
hnd_sprite_atlas h_atlas  = eng_sprite_atlas_load("CORE-SPRITES");
u32              i_sprite = eng_sprite_atlas_lookup_sprite_index(h_atlas, "GRASS");

// or set an entity's sprite directly by name
eng_entity_set_sprite(id, "CORE-SPRITES", "CONNOR-LEFT");
```

Map tiles and walls take atlas and sprite names the same way (`eng_map_set_tile_sprite`, `eng_map_wall_set_sprite`).

See [Sprites](sprites.md) for packing, the `.ifbatlas` file format, and sprite memory.
