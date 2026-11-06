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
| Text | `asset_type_e_text` | `asset_text` | Raw text, such as JSON. Maps are text assets, see [Map Format](map-format.md#loading). |
| Image | `asset_type_e_image` | `asset_image` | RGBA pixels, decoded with stb_image. |
| Sprite atlas | `asset_type_e_sprite_atlas` | `asset_sprite_atlas` | A packed image plus the name, position, and size of each sprite in it. |
| Sound | `asset_type_e_sound` | `asset_sound` | 16 bit PCM at 48 kHz, mono or stereo, converted from a `.wav` when the database is built. |

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
| `eng_asset_dictionary_load_preview` | Load a text, image, or sound preview for the GUI |

## Asset Database

`eng_asset_database_build(path_dictionary, path_database)` reads every file in the dictionary and writes them to a single `.ifbdb` file, with an index (`asset_index`: type, offset, size) for each asset. Sounds go in a [sound bank](#sound-bank) next to it instead.

At startup the engine opens `IFB_CONFIG_ASSET_DATABASE_PATH` (`asset-database.ifbdb`) and its bank (`asset-database.ifbsnd`) from the working directory. CMake copies `assets/*.ifbdb` and `assets/*.ifbsnd` into `build/<preset>/bin`, so **rebuild the database and commit it** after changing the dictionary or any source asset.

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
