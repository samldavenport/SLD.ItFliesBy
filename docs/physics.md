# Physics

The physics system (`IFB.Engine/src/physics`, `eng-physics.hpp`) moves entities with forces, springs, and drag, and keeps them from overlapping each other and the map. It works directly on the entity components described in [Entities and Components](entities-and-components.md).

## Simulation Step

`phys_mngr_simulate(dt)` runs once per pass of the [frame loop](architecture.md#frame-loop), after the game callback, with the measured delta time in seconds.

| Order | Step | Source | Description |
| --- | --- | --- | --- |
| 1 | Collision detection | `physics-collision-detection.cpp` | Builds a BVH for each map chunk and collects the pairs of entities whose bounds overlap. |
| 2 | Springs | `physics-spring.cpp` | Adds a spring force for every entity with a spring component. |
| 3 | Integration | `physics-integrator.cpp` | Turns the accumulated forces into new velocities and positions, then keeps map entities inside their map. |
| 4 | Collision resolution | `physics-collision-resolver.cpp` | Pushes apart the pairs from step 1 that still overlap after integration. |
| 5 | Reset | `physics-force-accumulator.cpp` | Clears the forces for the next step. |

Detection runs before integration, so it sweeps each moving entity's bounds by `velocity * dt` to catch pairs that will touch during the step.

## Forces

Forces are collected in the **force accumulator** (`phys_frc_accmltr`), which holds one summed `vec3` per entity. Add forces through the public API:

```cpp
eng_entity_add_force(id, { 0.0f, 0.0f, 5.0f });
```

Forces only last for one step, so anything continuous, like movement input, has to be added every frame.

> **Note:** the integrator only processes entities that have a force in the accumulator this step. An entity with velocity but no force this step won't be moved by the integrator.

## Integration

An entity is integrated if it has a force this step and has every component in `ENTITY_ARCHETYPE_PHYSICS_PARTICLE`: position, velocity, acceleration, inverse mass, drag, and terminal velocity.

For each entity:

```
acceleration = force * inv_mass
position    += velocity * dt + acceleration * dt² / 2
velocity     = (velocity + acceleration * dt) * drag^dt
velocity     = clamp(velocity, -term_velocity, term_velocity)   // per axis
```

| Component | Meaning |
| --- | --- |
| `cmpnt_inv_mass` | `1 / mass`. `eng_cmpnt_update_mass` sets it from a mass. |
| `cmpnt_drag` | The fraction of velocity kept after one second. `1.0` is no drag, lower values slow the entity faster. |
| `cmpnt_term_velocity` | The maximum speed on each axis. An axis set to `0` isn't clamped. |

If the entity has a map coordinates component, it's then pushed back inside its map (see [Maps](maps.md#movement-constraints)). The inset is half the entity's quad width, so the whole quad stays on the map.

## Springs

An entity with a position and a `cmpnt_spring` component is pulled toward its `anchor` entity:

```
extension = distance(entity, anchor) - rest_length
force     = direction_to_anchor * (stiffness * extension - damping * velocity_along_spring)
```

The force goes into the accumulator like any other force, so the spring entity also needs the physics particle components to move. The anchor isn't affected. Springs closer than `0.001` to their anchor are skipped.

The game attaches Jig to a `JIG-ANCHOR` entity with a spring, and moves the anchor to make Jig follow along (`game-player-rig.cpp`).

## Collision Detection

Collision is only checked between entities on the same **map chunk**. For each chunk of each map, detection:

1. Gets the entities in the chunk (`map_get_entities_in_chunk`).
2. Builds an AABB for each one that has a position and a quad, swept by its velocity if it has one.
3. Builds a BVH from those boxes and queries it for overlapping pairs.
4. Adds the pairs to the collision list. Pairs are stored as `(lower id, higher id)`, so each pair is only listed once.

All of this memory comes from a temporary arena that's freed at the end of detection.

Detection then publishes an `event_type_e_physics_collision` event for each pair in the list. See [Events](events.md#physics-collisions).

### Bounding Boxes

An entity's position is the center of its quad. The quad is `width × height`, and it's given a depth of `width` along z so it has volume to overlap with (`phys_aabb_half_extent_quad`).

### BVH

`physics-bvh.cpp` builds a bounding volume hierarchy for each chunk on every step:

- The root holds every entity. Each node is split along the longest axis of its entities' centers, at the median (found with quickselect), so the tree stays balanced and its depth is `log2(n)`.
- Interior nodes have `count == 0` and two children at `child_left_or_first` and `child_left_or_first + 1`. Leaf nodes have `count > 0` and hold entities `child_left_or_first` through `child_left_or_first + count - 1`.
- A tree for `n` entities has at most `2n - 1` nodes.

## Collision Resolution

Resolution moves positions directly and doesn't depend on `dt`. For each pair from detection:

1. Look up both entities. An entity without an inverse mass is treated as immovable. If neither can move, skip the pair.
2. Check that the boxes still overlap after integration, and find the axis with the smallest overlap.
3. Push both entities apart along that axis, split by inverse mass, so the lighter entity moves more.
4. If they're moving toward each other on that axis, cancel the closing velocity so they stop at the contact.
5. Push map entities back inside their map, then write the position and velocity back to the components.

There's no bounce. Entities stop at the contact point.

## Configuration

| Setting | Description |
| --- | --- |
| `IFB_CONFIG_ENTITY_CAPACITY` | The maximum number of entities the integrator, spring calculator, and collision list can hold |

The physics system allocates its memory from the `physics` reservation in the memory map.
