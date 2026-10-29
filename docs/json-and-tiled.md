# JSON and Tiled

The engine reads JSON with [RapidJSON](https://rapidjson.org/), through a thin read-only wrapper (`IFB.Engine/src/json`, `eng-json.hpp`). On top of that is a partial parser for [Tiled](https://www.mapeditor.org/) map files (`IFB.Engine/src/tiled`, `eng-tiled.hpp`).

Both are internal to the engine. Neither has a public `eng_*` API.

## JSON

### Documents

```cpp
const json_doc* doc = json_doc_create(h_arena, size, data);

const json_arr* images = json_doc_get_arr(doc, "image");
const u32       count  = json_arr_get_count(images);
for (u32 i = 0; i < count; ++i) {
    const json_obj* entry = json_arr_get_obj(images, i);
    const cchar*    name  = json_obj_get_cstr(entry, "name");
    const cchar*    path  = json_obj_get_cstr(entry, "path");
}
```

`json_doc_create` parses `size` bytes of JSON text into a document. All of the document's memory comes from the arena you pass in, so the document and every string and value read from it stay valid until that arena is reverted, reset, or freed.

The wrapper has three types:

| Type | Get members by | Functions |
| --- | --- | --- |
| `json_doc` | name | `json_doc_get_obj`, `_arr`, `_cstr`, `_bool`, `_u32`, `_s32`, `_u64`, `_s64`, `_f32`, `_f64` |
| `json_obj` | name | `json_obj_get_obj`, `_arr`, `_cstr`, `_bool`, `_u32`, `_s32`, `_u64`, `_s64`, `_f32`, `_f64` |
| `json_arr` | index | `json_arr_get_count`, `_obj`, `_bool`, `_u32`, `_s32`, `_u64`, `_s64`, `_f32`, `_f64` |

Things to know:

- **Getters expect the member to exist with the right type.** Most of them assert it in debug builds, and some (like `json_doc_get_cstr`) don't check at all. There's no "has member" function yet, so check the shape of the JSON you're reading.
- Arrays have no string getter (`json_arr_get_cstr`).
- The wrapper only reads JSON. Files like the asset dictionary are written by building the JSON text directly (`asset_dictionary_file_list_json_write`).

### Arena Allocator

RapidJSON allocates through `json_allocator` (`json.cpp`), which pushes onto the document's arena:

- `Malloc` pushes onto the arena.
- `Realloc` pushes a new block and copies the old data into it. The old block isn't reclaimed.
- `Free` does nothing (`kNeedFree = false`).

Since nothing is freed until the arena is, parse large documents in an arena you can revert or free when you're done with them.

### Where It's Used

| Caller | Reads |
| --- | --- |
| `asset-dictionary.cpp` | `asset-dictionary.json` (see [Assets](assets.md#asset-dictionary)) |
| `json_test` | `assets/json/test.json`. A test function, not called by the engine. |
| `tiled.cpp` | Tiled map files (below) |

## Tiled

`eng-tiled.hpp` describes the [Tiled JSON map format](https://doc.mapeditor.org/en/stable/reference/json-map-format/): maps, layers, objects, chunks, tilesets, tiles, animation frames, terrains, Wang sets, templates, and custom properties.

```cpp
tiled_map* map = tiled_map_create(h_arena, size, json_text);
```

`tiled_map_create` parses the JSON, gets the map object, and fills in a `tiled_map`, with everything allocated in the arena.

### Status

The Tiled parser is **unfinished and not used yet**. Nothing outside `tiled.cpp` calls it, and engine maps are built in code (see [Maps](maps.md)).

| Part | Status |
| --- | --- |
| Map properties, layers, tilesets, properties, tile offsets, transformations, frames, terrains, points | Parsing functions exist |
| Grid | Stub, returns `NULL` (`TODO`) |
| Object layers, objects, chunks, text, tiles, Wang sets, Wang colors, Wang tiles, templates | Declared in `eng-tiled.hpp`, but not implemented |

Also worth checking before using it: `tiled_map_create` looks for a member called `"map"`, but Tiled's JSON export puts the map's properties at the top level of the file.

### String Enums

Tiled stores values like orientation and render order as strings. The parser hashes each string and compares the hash against the enum values, which are the precomputed hashes:

```cpp
enum tiled_map_orientation_e {
    tiled_map_orientation_e_orthogonal = 0x12BB0B63,
    tiled_map_orientation_e_isometric  = 0x2FC6069C,
    // ...
};
```

`tiled_map_get_hashes`, `tiled_layer_get_hashes`, and `tiled_property_get_hashes` compute the same hashes from the strings.
