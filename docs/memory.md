# Memory

The engine doesn't use the C++ heap (`new`, `malloc`, STL containers). The game reserves virtual memory up front and hands it to the engine, and every system allocates from its own piece of it.

## Memory Map

The game fills in an `eng_mem_map` (`ifb-engine.hpp`) and passes it to `eng_context_create`. In `game-win32-main.cpp`, each entry is a `VirtualAlloc(MEM_RESERVE)` call, so the address space is reserved but no memory is committed yet. The exception is `stack`, which is a static buffer.

| Entry | Size (game) | Used by |
| --- | --- | --- |
| `stack` | 64 KB | The [global stack](#global-stack) |
| `files` | 64 MB | File manager |
| `entities` | 64 MB | Entity manager |
| `components` | 64 MB | Component manager |
| `physics` | 64 MB | Physics manager |
| `tiles` | 64 MB | Map manager |
| `particles` | 64 MB | Particle manager |
| `rendering` | 64 MB | Renderer |
| `arenas` | 64 MB | [Arenas](#arenas) |
| `arenas_large` | 512 MB | [Large arenas](#large-arenas) |
| `assets` | 128 MB | Asset manager |
| `sprites` | 256 MB | [Sprite memory](sprites.md#memory) |
| `core`, `gui`, `quads` | 64 MB each | Reserved by the game, but not used by the engine yet |

`eng_context_startup` turns each used entry into a [reservation](#reservations) and passes it to its system's `*_startup` function.

## Global Stack

The global stack is the `stack` buffer from the memory map. It's a simple bump allocator for things that live for the whole run, like the manager structs, and nothing on it is ever freed.

```cpp
auto mngr = global_alloc<memory_mngr>();
```

It's internal to the engine, and it asserts if it runs out of space.

## Reservations

A reservation (`memory-reservation.cpp`) wraps one range of reserved address space. Systems take memory from the front of it, and pages are committed as they're taken.

| Function | Description |
| --- | --- |
| `reservation_push_bytes` / `_push_pages` | Commit and take the next block of memory, rounded up to whole pages |
| `reservation_push_stack_bytes` / `_pages` / `_all` | Same, but returns the memory as a `stack` |
| `reservation_push_all` | Commit and take everything that's left |
| `reservation_claim_all` | Take everything that's left without committing it. The caller commits what it uses. |
| `reservation_decommit` | Decommit the reservation's memory |

Memory taken from a reservation is never given back individually. Systems size what they need at startup from the values in `ifb-config.hpp`.

## Arenas

An arena is a bump allocator with a fixed capacity, referred to by an `hnd_arena`. Arenas are the main way to get temporary or variable sized memory at runtime, and they're part of the public API.

```cpp
hnd_arena h_arena = eng_arena_alloc();

u32   save = eng_arena_save(h_arena);
void* data = eng_arena_push(h_arena, size);
// ... use data ...
eng_arena_revert(h_arena, save);   // free everything pushed since the save

eng_arena_free(h_arena);           // give the arena back
```

| Function | Description |
| --- | --- |
| `eng_arena_alloc` | Take a free arena. Returns `INVALID_HANDLE` if there are none left. |
| `eng_arena_large_alloc` | Take a free [large arena](#large-arenas) |
| `eng_arena_push` | Take `size` bytes. Returns `NULL` if the arena is full. |
| `eng_arena_save` / `eng_arena_revert` | Save the current position, and later go back to it |
| `eng_arena_reset` | Go back to the start |
| `eng_arena_free` | Return the arena to the free list |
| `eng_arena_size_used` / `_size_free` | How much of the arena is used and free |

Inside the engine, `ifb_arena_push_struct(h_arena, type)` and `ifb_arena_push_array(h_arena, count, type)` push typed memory.

### How Arenas Work

The arena allocator splits its reservation into equal slots of `IFB_CONFIG_ARENA_MEM_GRANULARITY` (1 MB), so with a 64 MB reservation there are 64 arenas. Each slot starts with a small `arena` header, followed by the arena's memory. Free and used arenas are kept in two linked lists, and the handle is the slot's index.

Normal arenas are **fully committed at startup**. Pushing only moves the position forward.

### Large Arenas

Large arenas use the same API, for allocations bigger than a normal arena can hold, like copying a whole sprite atlas image.

- Each slot is `IFB_CONFIG_ARENA_LARGE_MEM_GRANULARITY` (64 MB), so the 512 MB reservation holds 8 large arenas.
- Only the page with each header is committed at startup. The rest is committed as the arena grows, in steps of `IFB_CONFIG_ARENA_LARGE_COMMIT_GRANULARITY` (64 KB).
- Freeing a large arena decommits everything except its header page, so the memory goes back to the OS.
- Large arena handles have the high bit set (`ARENA_HANDLE_LARGE_BIT`), so every arena function works with either kind and can tell them apart.

## Stacks

A `stack` is a bump allocator over memory you provide, with `push`, `pull` (pop bytes off the top), `save`, `revert`, and `reset`.

```cpp
stack* s = eng_stack_arena_create(capacity, h_arena);
void*  p = eng_stack_push(s, size);
```

Stacks can be created in an arena (`eng_stack_arena_create`) or in a block of memory you already have (`eng_stack_memory_create`). Sprite memory blocks and reservations use them internally.

## Other Allocators

| Allocator | Source | Description |
| --- | --- | --- |
| Asset memory | `src/assets/asset-memory.cpp` | Fixed size blocks in small, medium, and large classes. See [Assets](assets.md#runtime-memory). |
| Sprite memory | `src/sprites/sprite-memory.cpp` | A few large blocks, one per loaded atlas. See [Sprites](sprites.md#memory). |
| Block allocator | `src/memory/memory-block.cpp` | Fixed size blocks tracked in a 64-bit mask. It's implemented, but nothing uses it yet. |

## Configuration

| Setting | Default | Description |
| --- | --- | --- |
| `IFB_CONFIG_ARENA_MEM_GRANULARITY` | 1 MB | Size of each normal arena |
| `IFB_CONFIG_ARENA_LARGE_MEM_GRANULARITY` | 64 MB | Size of each large arena |
| `IFB_CONFIG_ARENA_LARGE_COMMIT_GRANULARITY` | 64 KB | How much a large arena commits at a time |
| `IFB_CONFIG_MEMORY_RES_SIZE_*` | | Reservation sizes for some systems |

The memory map sizes the game actually uses are set at the top of `game-win32-main.cpp`.
