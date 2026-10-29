# Particles

The particle system (`IFB.Engine/src/particle`, `eng-particle.hpp`) runs **emitters** that spawn, move, and fade small colored quads. Particles are simulated by the particle system itself, separately from entity [physics](physics.md), and don't collide with anything.

## Emitter Types

| Type | `particle_emitter_type_e` | Props | Behavior |
| --- | --- | --- | --- |
| Parabolic (fountain) | `particle_emitter_type_e_parabolic` | `particle_parabolic_props` | Launches particles in a cone around a velocity, and gravity bends their paths. Spawns until it's stopped. |
| Radial (explosion) | `particle_emitter_type_e_radial` | `particle_radial_props` | Launches particles outward in every direction in straight lines. Spawns each particle once per start, or until stopped if `is_continuous` is set. |

### Parabolic Props

| Field | Description |
| --- | --- |
| `velocity` | Launch direction and speed, in units per second. Can't be zero. |
| `gravity` | Acceleration applied to every live particle, in units per second². |
| `spread_degrees` | Half angle of the launch cone, from `0` (a straight line) to `180` (any direction). |

### Radial Props

| Field | Description |
| --- | --- |
| `speed` | Launch speed in units per second. Must be greater than zero. |
| `is_continuous` | `false` spawns `count` particles once per start. `true` keeps spawning until the emitter is stopped. |

A radial particle moves directly away from the emitter's position. A particle that spawns exactly on the center gets a random direction.

## Common Properties

Both create functions take the same base properties:

| Parameter | Description |
| --- | --- |
| `name` | Name shown in the GUI |
| `count` | Number of particles in the emitter, up to `IFB_CONFIG_PARTICLE_BUFFER_COUNT` (128) |
| `lifespan_ms` | How long each particle lives |
| `spawn_delay_ms` | Time between spawns |
| `spawn_radius` | Particles spawn at a random point within this distance of the emitter's position, spread evenly through the sphere. `0` spawns them all at the position. |
| `width`, `height` | Size of each particle quad |
| `color_primary`, `color_secondary` | Each particle fades from the primary color when it spawns to the secondary color when it dies |

## Using an Emitter

```cpp
particle_parabolic_props props;
props.velocity       = { 0.0f, 1.0f, 0.0f };
props.gravity        = { 0.0f, -2.0f, 0.0f };
props.spread_degrees = 15.0f;

hnd_particle_emitter h_emitter = eng_particle_emitter_create_parabolic(
    "fountain", 128, 1000.0f, 10.0f, 0.02f, 0.01f, 0.01f,
    color_primary, color_secondary, props
);

eng_particle_emitter_start (h_emitter, position);
eng_particle_emitter_stop  (h_emitter);
eng_particle_emitter_destroy(h_emitter);
```

| Function | Description |
| --- | --- |
| `eng_particle_emitter_create_parabolic` / `_create_radial` | Create an idle emitter. Returns `INVALID_HANDLE` if there's no free slot. |
| `eng_particle_emitter_start` | Start spawning at a position. The first particle spawns on the next update. |
| `eng_particle_emitter_stop` | Stop spawning. Live particles finish their lives, then the emitter goes idle. |
| `eng_particle_emitter_destroy` | Free the emitter's slot. |
| `eng_particle_emitter_get_info` | Get the emitter's properties and state, including its live particle count. |
| `eng_particle_emitter_update` | Change the base properties while the emitter runs. The name, type, and state are ignored. Nothing changes if any value is invalid. |
| `eng_particle_emitter_get/set_props_parabolic`, `_radial` | Read or change the type props. Fails if the type doesn't match. |

Notes on live editing:

- Changing `count` restarts the emitter with no particles, because the old particles were laid out for the old count.
- Lowering `lifespan_ms` shortens the remaining life of the live particles to fit.

## Lifecycle

An emitter is either **idle** or **active**. The particle manager keeps a list of active emitters for each type, and only active emitters are updated.

1. **Create**: the emitter takes a free slot and is idle.
2. **Start**: the emitter becomes active and starts spawning.
3. **Stop**, or a non-continuous radial emitter spawns its last particle: spawning ends, but the emitter stays active while particles are alive.
4. When its last particle dies, the emitter goes idle again and can be started again.

## Update

`particle_mngr_update_emitters(dt_ms)` runs every pass of the [frame loop](architecture.md#frame-loop), after physics. For each active emitter:

1. **Age** every live particle by `dt`. A particle whose life reaches zero dies. The rest blend their color from primary to secondary based on how much of their life is used up.
2. **Move** every live particle by its velocity. Parabolic emitters add gravity to the velocity first.
3. **Spawn** new particles as long as enough time has passed for another `spawn_delay_ms`. Several particles can spawn in one update if the frame was long.
4. Go idle if the emitter has stopped spawning and has no live particles.

Particles are stored in a ring. Every particle in an emitter has the same lifespan, so they die in the order they spawned. If the next slot in the ring is still alive, every particle is alive, and the emitter waits for a slot to open instead of replacing a live particle.

## Emitters on Entities

An entity with a position and a `cmpnt_particle_emitter` component carries an emitter with it. Each frame, before particles update, `particle_mngr_follow_entities` moves each of these emitters to its entity's position. Only new particles spawn at the new position, so the live particles trail behind a moving entity.

## Rendering

Particles are drawn as solid colored quads that face the camera. They don't use sprites.

To draw an entity's particles, call `eng_entity_render` on the entity. It pushes the emitter to the renderer along with the entity's quad. Particles go into the same vertex buffer as the entity quads, after them, so they render on top. If the buffer fills up, the rest of the particles aren't drawn.

> **Note:** `eng_particle_emitter_render` isn't implemented yet and always returns `false`. For now, emitters are only drawn in the main view through an entity.

### Particle View

The particle view is an offscreen render of every active emitter with its own camera, used by the **Engine > Particle System** window in the GUI. It only draws on frames where it's requested:

```cpp
eng_particle_view_request(width, height);
u32 texture = eng_particle_view_get_texture();   // an OpenGL texture id for ImGui::Image
```

Its camera is set separately from the main camera with `eng_particle_view_set_camera_origin` / `_target`.

## Memory

Every emitter gets a full buffer of `IFB_CONFIG_PARTICLE_BUFFER_COUNT` particles, whatever its `count` is. The number of emitters (`eng_particle_emitter_get_capacity`) isn't set directly. It's the most emitters that fit in the `particles` reservation in the memory map.
