# Sprites

A sprite is an image inside a **sprite atlas**: one large texture with many images packed into it (`IFB.Engine/src/sprites`, `eng-sprite.hpp`). Entities, map tiles, and walls all draw sprites from atlases.

## Names and Handles

Every sprite comes from an image asset, and **a sprite's name is the name of the image asset it was made from**. For example, the `GRASS` image asset becomes the `GRASS` sprite.

| Handle | Value |
| --- | --- |
| `hnd_sprite_atlas` | The atlas's asset handle, which is the hash of the atlas's asset name |
| `hnd_sprite` | The image's asset handle, which is the hash of the sprite's name |

Since handles are name hashes, a sprite is found by hashing its name and searching the atlas for that handle. Most of the API takes names instead of handles.

A sprite can also be referred to by its **index** in the atlas, which is what the sprite component stores (`cmpnt_sprite::i_sprite`).

## Using Sprites

```cpp
// load the atlas from the asset database and upload it to the gpu
hnd_sprite_atlas h_atlas = eng_sprite_atlas_load("CORE-SPRITES");

// the shaders draw from one atlas at a time
eng_sprite_atlas_set_active(h_atlas);

// give an entity a sprite
eng_entity_set_sprite(id, "CORE-SPRITES", "CONNOR-LEFT");

// or look one up yourself
u32 i_sprite = eng_sprite_atlas_lookup_sprite_index(h_atlas, "GRASS");
```

| Function | Description |
| --- | --- |
| `eng_sprite_atlas_load` | Load an atlas from the asset database and upload it to the GPU. Loading an atlas that's already loaded returns its handle. Returns `INVALID_HANDLE` if the atlas isn't in the database or there's no room. |
| `eng_sprite_atlas_unload` | Release the atlas's memory and GPU texture |
| `eng_sprite_atlas_set_active` | Set the atlas the renderer draws from. See [Renderer](renderer.md#sprite-atlases). |
| `eng_sprite_atlas_get_info` | Atlas width, height, and sprite count |
| `eng_sprite_atlas_lookup_sprite_index` | Find a sprite's index by name |
| `eng_sprite_atlas_get_sprite_info` | A sprite's name, handle, and pixel rectangle in the atlas |
| `eng_sprite_atlas_get_sprite_image` | Copy one sprite's pixels out into an arena |

Map tiles and walls take atlas and sprite names in the same way: `eng_map_set_tile_sprite`, `eng_map_chunk_set_sprite`, `eng_map_wall_set_sprite`. Each map chunk can only use one atlas (see [Maps](maps.md#chunks)).

## Building an Atlas

Atlases are built from image assets with the **atlas builder**, usually through **Engine > Sprite Tools > Sprite Atlas Builder** in the GUI.

```cpp
eng_sprite_atlas_builder_clear();
eng_sprite_atlas_builder_add("CONNOR-LEFT");
eng_sprite_atlas_builder_add("GRASS");

asset_preview preview;
eng_sprite_atlas_builder_pack(preview);                       // pack and preview
eng_sprite_atlas_builder_save("sprites/core-sprites.ifbatlas");
```

1. **Add** image assets by name. Each image is loaded once when it's added to read its size. Adding the same image twice does nothing.
2. **Pack** the sprites with `stb_rect_pack`. The atlas starts as the smallest power of two square that could hold every sprite, then doubles its width and height in turns until everything fits, up to `4096 × 4096`. Every sprite gets 1 pixel of padding on its right and bottom so texture filtering doesn't bleed into its neighbors.
3. **Build** the atlas image by copying each sprite's pixels to where it was packed. Anything that isn't a sprite is transparent.
4. **Save** the atlas to an `.ifbatlas` file.

Then add the `.ifbatlas` file to the asset dictionary and rebuild the asset database (see [Assets](assets.md)). The game loads atlases from the database, not from `.ifbatlas` files directly.

Packing fails if the list is empty or the sprites don't fit in `4096 × 4096`.

## Inspecting an Atlas

**Engine > Sprite Tools > Sprite Atlas Inspector** opens an `.ifbatlas` file and lists its sprites:

| Function | Description |
| --- | --- |
| `eng_sprite_atlas_inspector_open` | Load an atlas file and get a preview of its image |
| `eng_sprite_atlas_inspector_get_info` | Atlas size and sprite count |
| `eng_sprite_atlas_inspector_get_sprite` | One sprite's name, handle, and rectangle |
| `eng_sprite_atlas_inspector_close` | Release the atlas |

## File Format

An `.ifbatlas` file is a header followed by the atlas pixels (`sprite-atlas.cpp`):

| Field | Size | Description |
| --- | --- | --- |
| Verification string | 14 bytes | `IFBSPRITEATLAS` |
| Sprite count (n) | 4 bytes | |
| Atlas width (w) | 4 bytes | In pixels |
| Atlas height (h) | 4 bytes | In pixels |
| Handles | n × 4 bytes | Each sprite's `hnd_sprite` |
| Names | n × 16 bytes | Each sprite's name, `IFB_CONFIG_ASSET_NAME_SIZE` bytes each |
| Indexes | n × 12 bytes | Width, height, and offset of each sprite. The offset is the pixel index of the sprite's top left corner: `y * w + x`. |
| Pixels | w × h × 4 bytes | RGBA, starting from the top row |

Loading checks the verification string and that every sprite fits inside the atlas. A failed build or load leaves the atlas empty.

The atlas image isn't flipped when it's uploaded, so the top row of the atlas is `v = 0` in the shaders.

## Memory

Sprites use their own memory (`sprite-memory.cpp`) instead of arenas, because atlas images are large. The `sprites` reservation is split into `IFB_CONFIG_SPRITE_MEM_BLOCK_CAPACITY` (4) blocks of `IFB_CONFIG_SPRITE_MEM_BLOCK_SIZE` (64 MB) each, and each block is used as a stack.

These take one block each while they're in use:

- Every loaded atlas
- The atlas builder
- The atlas inspector, while it has an atlas open

There are 4 blocks in total. While the builder and inspector are both open, only 2 atlases can be loaded.

The sprite manager can also have at most `IFB_CONFIG_SPRITE_TABLE_CAPACITY` (4) atlases loaded, and the renderer has the same number of atlas texture slots.
