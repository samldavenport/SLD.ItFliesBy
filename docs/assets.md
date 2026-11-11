# Assets

Assets go through two stages: an editable JSON **asset dictionary** that lists source files, and a binary **asset database** built from it that the game loads at runtime.

```
assets/images/*.png ─┐
assets/sprites/*.ifbatlas ─┤
assets/maps/*.json ─┤
assets/json/*.json ─┼──> asset-dictionary.json ──(build)──> asset-database.ifbdb ──> runtime
assets/sounds/*.wav ─┘
```

## Asset Types

| Type | `asset_type_e` | Runtime struct | Description |
| --- | --- | --- | --- |
| Text | `asset_type_e_text` | `asset_text` | Raw text. |
| Map | `asset_type_e_map` | `asset_map` | A [map file](map-format.md)'s JSON. The build checks that it's valid JSON. It's only parsed into a map when it's loaded, because that needs its atlases. |
| Image | `asset_type_e_image` | `asset_image` | RGBA pixels, decoded with stb_image. |
| Sprite atlas | `asset_type_e_sprite_atlas` | `asset_sprite_atlas` | A packed image plus the name, position, and size of each sprite in it. |
| Sound | `asset_type_e_sound` | `asset_sound` | 16 bit PCM at 48 kHz, mono or stereo, converted from a `.wav` when the database is built. |
| Font atlas | `asset_type_e_font_atlas` | `asset_font_atlas` | A font baked from a TTF: line metrics, glyphs, kerning, and a coverage atlas. Stored exactly as it is in memory. |

Each kind of JSON the game loads gets its own type, rather than all of it being text. That way a type can be listed, checked when the database is built, and loaded on its own. Maps are stored the same way as text (a size, then the bytes), but in their own group. Other JSON assets, such as levels, should get their own type the same way.

`asset_type_e` values are stored in the database, so a new type goes at the end, before `asset_type_e_count`.

## Names and Handles

Assets are referred to by name. Names are up to `IFB_CONFIG_ASSET_NAME_SIZE` (32) characters and stored uppercase in the dictionary (e.g. `CONNOR-LEFT`, `CORE-SPRITES`). An asset's `hnd_asset` is the hash of its name, so the same name always gives the same handle.

## Asset Dictionary

`assets/asset-dictionary.json` maps names to file paths, grouped by type:

```json
{
    "text": [],
    "map": [
        { "name": "DEV-MAP", "path": "maps/dev-map.json" }
    ],
    "image": [
        { "name": "GRASS", "path": "images/grass.png" }
    ],
    "sprite_atlas": [
        { "name": "CORE-SPRITES", "path": "sprites/core-sprites.ifbatlas" }
    ],
    "font_atlas": [
        { "name": "UI-FONT", "path": "fonts/ui-font.ifbfont" }
    ],
    "sound": [
        { "name": "PIANO", "path": "sounds/piano.wav" }
    ]
}
```

Edit it in the GUI under **Game > Assets > Asset Dictionary**, or through the API:

| Function | Description |
| --- | --- |
| `eng_asset_dictionary_new` / `_load` / `_save` | Create, load, or save a dictionary |
| `eng_asset_dictionary_add_file` / `_remove_file` | Add or remove an entry |
| `eng_asset_dictionary_get_file_count` / `_get_file_info` | List the entries of a type |
| `eng_asset_dictionary_load_preview` | Load a text, image, or sound preview for the GUI. A map is previewed as its JSON. |

## Asset Database

`eng_asset_database_build(path_dictionary, path_database)` reads every file in the dictionary and writes them to a single `.ifbdb` file, with an index (`asset_index`: type, offset, size) for each asset. Sounds go in a [sound bank](#sound-bank) next to it instead.

The assets are grouped by type: text, maps, images, sprite atlases, then font atlases, each in dictionary order. All of one type sit together in the file, so they can be read in one pass.

At startup the engine opens `IFB_CONFIG_ASSET_DATABASE_PATH` (`asset-database.ifbdb`) and its bank (`asset-database.ifbsnd`) from the working directory. CMake copies `assets/*.ifbdb` and `assets/*.ifbsnd` into `build/<preset>/bin`, so **rebuild the database and commit it** after changing the dictionary or any source asset.

The **Game > Assets > Asset Database** window can build, open, and preview a database. `eng_asset_database_copy_to_runtime` replaces the database the running game is using with the open one.

**Open Runtime** opens the database the game is running from, so you can see what the game actually has. The game keeps its own copy open, so it isn't affected.
- **Opening it by path:** `eng_asset_runtime_get_path` gives the runtime database's absolute path, and the window opens it with that. The file manager finds an open file by the path it was opened with, so opening the database by the relative path the game uses would share the game's file entry.
- **Copy to Runtime is off for it:** `eng_asset_database_is_runtime` says whether the open database is the runtime one (it compares absolute paths, ignoring case and slash direction). Copying the runtime database onto itself would empty it, so `eng_asset_database_copy_to_runtime` refuses, and the window disables the button.

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

To change an existing atlas, **Open...** it in the builder (`eng_sprite_atlas_builder_open`), edit the sprites, **Save** it, and rebuild the database.

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

## Font Atlases

A font atlas (`.ifbfont`) is a font baked from a TTF at one size: its line metrics, a glyph for each codepoint in a range, kerning pairs, and an atlas with one coverage byte per pixel. Make them with **Engine > Tools > Font Viewer**:

1. **Open** a `.ttf`. It's baked with the settings under **Bake**: pixel height, the first codepoint and how many glyphs, and oversampling.
2. Change the settings and **Bake** again until the sample text and the atlas look right.
3. **Save** the font atlas, add it to the asset dictionary as a Font Atlas, and rebuild the database.

The baked font is one block of memory (`struct font` in `ifb.hpp`) that only uses offsets, never pointers. The file, the database, and memory all hold exactly the same bytes, so loading one is a single read with nothing to parse or fix up, and a font atlas in asset memory can be moved without any fix-ups.

At runtime, copy a font atlas onto your own arena by name. The copy is yours, so unlike asset memory it doesn't move or get evicted:

```cpp
font* fnt = eng_font_atlas_load_to_arena("UI-FONT", h_arena);

const font_glyph* g    = fnt->glyph('A');       // NULL if the font doesn't have it
const f32         kern = fnt->kerning('A', 'V'); // 0 if the pair isn't kerned
```

| Function | Description |
| --- | --- |
| `eng_font_load_to_arena` | Bake a TTF into a font with a `font_config` |
| `eng_font_save` | Write a font to an `.ifbfont` file |
| `eng_font_load_file_to_arena` | Read an `.ifbfont` file |
| `eng_font_atlas_load_to_arena` | Copy a font atlas from the runtime database |
| `eng_font_glyph_table_to_image` | The atlas as an RGBA image, white with the coverage as alpha |

Every font that's read, from a file or the database, is checked before it's used: the size matches, the parts are laid out where the header says, every glyph's rectangle is inside the atlas, and the kerning pairs are sorted. A font that fails any of these isn't loaded.

### Font File Format

| Field | Size | Description |
| --- | --- | --- |
| Verification string | 12 bytes | `IFBFONTATLAS` |
| Header | 64 bytes | `struct font`. Its first field is the size of everything from here to the end of the file. |
| Glyphs | n × 28 bytes | `font_glyph`: the rectangle in the atlas, the rectangle to draw relative to the pen, and the advance |
| Kerning pairs | k × 8 bytes | `font_kern_pair`, sorted by key (the two glyph indexes, first in the high 16 bits) |
| Atlas | w × h bytes | One coverage byte per pixel, starting from the top row |

The database stores everything after the verification string.

## Sounds

Sounds are `.wav` files, which have to be uncompressed PCM at 8, 16, 24, or 32 bit. The build converts every sound to the same format, so the game never has to: 16 bit, 48 kHz (the rate the audio device runs at), and mono or stereo. Mono stays mono, and past the first 2 channels of a file are dropped. The rate is changed by blending the 2 closest frames.

The build reads the whole file and writes the converted samples, so both have to fit in one large arena (64 MB), and the converted sound has to fit in the largest asset block (64 MB). That's about 5 minutes of stereo. Longer sounds will need streaming.

Play a sound by name. It's loaded from the runtime database if it isn't in memory yet:

```cpp
hnd_audio_voice h_voice = eng_audio_sound_play("PIANO", audio_group_e_music, 0.5f, false);
```

The voice keeps the sound pinned in asset memory while it plays, so the samples can't move or be evicted under it, and unpins it when the voice is freed. There's nothing to free. See [Audio](audio.md#voices) for what you can do with the voice.

Things to know:

- **The first play loads the sound**, on the main thread, so a long sound can hitch the frame it's first played in. [Preload](#preloading) it to load it ahead of time.
- **Copying a database to the runtime stops every sound that's playing**, without a fade, since the runtime database can't close while anything is pinned. Preloaded sounds are loaded again from the new database.

### Preloading

`eng_audio_sound_preload` loads a sound now, like at startup or during a loading screen, so its first play doesn't wait on the disk. A loaded asset can still be evicted when it hasn't been used in a while (`IFB_CONFIG_ASSET_EXPIRE_FRAMES`) or when asset memory needs the room, so a preloaded sound is pinned and stays in memory until it's unloaded.

```cpp
eng_audio_sound_preload ("BOOP"); // at startup
eng_audio_sound_play    ("BOOP", audio_group_e_sfx, 0.25f, false);
eng_audio_sound_unload  ("BOOP"); // when it isn't needed any more
```

| Function | Description |
| --- | --- |
| `eng_audio_sound_preload` | Load the sound if it isn't in memory, and keep it there. Returns `false` if there's no sound by that name, no room for it, or `IFB_CONFIG_AUDIO_PRELOAD_CAPACITY` (64) sounds are already preloaded. |
| `eng_audio_sound_unload` | Undo one preload. After the last one the sound can be evicted again, it isn't freed right away. A sound that isn't preloaded is ignored. |

Things to know:

- **Preloads are counted.** Preloading a sound twice takes two unloads to let it go, so different parts of the game can preload the same sound without stepping on each other. It's still one copy in memory.
- **There's nothing to clean up at shutdown.** The engine lets go of anything still preloaded.
- **Preloads survive Copy to Runtime.** They're let go while the runtime database is replaced, then loaded from the new one. A preload whose sound isn't in the new database is dropped.
- **Preloaded sounds take memory for as long as they're held**, and a pinned block can't be moved, which keeps its partition from being compacted. Preload the sounds that need to start right away, like short effects, and unload what a level doesn't use.

The **Audio Test** window shows how many sounds are preloaded.
- **Sounds are big.** A minute of stereo is about 11 MB in the bank.

In the GUI, the dictionary previews the `.wav` as it is in the file, and the database previews it as it was converted. **Play From Runtime** in the database window plays it the way the game does, through the runtime database.

### Sound Bank

The samples are kept out of the database, in a bank file next to it with the same name and the `.ifbsnd` extension (`asset-database.ifbdb` goes with `asset-database.ifbsnd`). The database still has an index for every sound, and for sounds the offset and size are in the bank. The bank is only written if the dictionary has sounds.

| Part | Size |
| --- | --- |
| Verification string (`IFBSOUNDBANK`) | 12 bytes |
| Signature | 4 bytes |
| Sounds | Each one is the sample rate, channel count, and frame count (4 bytes each), then the interleaved 16 bit samples |

The signature is a hash of the database's handles and indexes, which change with every build, so a bank only opens with the database it was built with. A bank that's missing, damaged, or from another build is left closed. The database still loads and everything else in it works, but its sounds won't load or play.

**Copy to Runtime** copies the bank too. If the runtime ends up without a bank that matches, the copy reports that the sounds won't play.

The bank has its own file handle, so reading sounds doesn't move the database's cursor. [Streaming](audio.md#streaming) reads sounds straight from it.

The bank is committed with the database, but it's stored with [Git LFS](https://git-lfs.com) (`*.ifbsnd` in `.gitattributes`), so the repo history only holds a small pointer for each version. `scripts/Initialize-Repo.ps1` sets up LFS and downloads the banks. A clone without LFS gets the pointer files instead, which don't pass the bank's verification, so the game runs with its sounds silent.
