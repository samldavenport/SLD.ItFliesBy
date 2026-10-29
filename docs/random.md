# Random

The random system (`IFB.Engine/src/random`, `eng-random.hpp`) hands out random numbers from a pre-generated table. It's fast and good enough for gameplay and visual effects, but it isn't cryptographically secure.

## Using It

| Function | Returns |
| --- | --- |
| `eng_random_u32` | A random `u32` |
| `eng_random_f32` | A random float in `[0.0, 1.0)` |
| `eng_random_f32_range(min, max)` | A random float in `[min, max)` |

```cpp
const f32 offset_x = eng_random_f32_range(-0.1f, 0.1f);
```

Inside the engine, the same functions are available without the `eng_` prefix (`random_u32`, `random_f32`, `random_f32_range`). The [particle system](particles.md) uses them for spawn positions and launch directions.

## How It Works

The random table holds `IFB_CONFIG_RANDOM_TABLE_COUNT` (1024) `u32` values generated with a 32-bit **xorshift** generator:

```
seed ^= seed << 13
seed ^= seed >> 17
seed ^= seed << 5
```

1. **Startup:** the seed is the system timestamp with its lowest bit set, since xorshift can never have a seed of zero. The table is filled right away.
2. **Each call** returns the next value in the table and moves forward one place.
3. **Refresh:** the table is filled with new values, continuing from where the generator left off, when either:
   - every value has been used, or
   - `IFB_CONFIG_RANDOM_REFRESH_FRAMES` (60) frames have passed since the last refresh. `random_table_update` counts these at the start of each frame.

### Floats

`random_f32` takes the top 24 bits of a `u32` and divides by 2²⁴. A float can hold 24 bits exactly, so every result is evenly spaced and the result is never `1.0`.

`random_f32_range` scales that into the range: `min + (max - min) * random_f32()`.

## Things to Know

- **Results aren't repeatable.** The seed comes from the clock and there's no way to set it, so each run gives different numbers. Getting the same sequence for testing or replays would need a seed setting.
- The table and its values are allocated on the [global stack](memory.md#global-stack) at startup.

## Configuration

| Setting | Default | Description |
| --- | --- | --- |
| `IFB_CONFIG_RANDOM_TABLE_COUNT` | 1024 | Number of values in the table |
| `IFB_CONFIG_RANDOM_REFRESH_FRAMES` | 60 | Frames between refreshes |
