# Physics

The physics system (`IFB.Engine/src/physics`, `eng-physics.hpp`) moves entities with forces, springs, and drag, and keeps them from overlapping each other and the map. It works directly on the entity components described in [Entities and Components](entities-and-components.md).

## Simulation Step

`phys_mngr_simulate(dt)` runs once per pass of the [frame loop](architecture.md#frame-loop), after the game callback, with the measured delta time in seconds.

| Order | Step | Source | Description |
| --- | --- | --- | --- |
| 0 | Swap contact lists | `physics-manager.cpp` | The last step's contacts become the previous list, so phases can be found by comparing the two. |
| 1 | Collision detection | `physics-collision-detection.cpp` | Builds a BVH for each map chunk and collects the pairs of entities whose bounds overlap and whose [masks](#colliders) allow it. |
| 2 | Springs | `physics-spring.cpp` | Adds a spring force for every entity with a spring component. |
| 3 | Integration | `physics-integrator.cpp` | Turns the accumulated forces into new velocities and positions, then keeps map entities inside their map. |
| 4 | Collision resolution | `physics-collision-resolver.cpp` | Records a contact for each pair from step 1 that still overlaps after integration, and pushes the solid ones apart. |
| 5 | Reset | `physics-force-accumulator.cpp` | Clears the forces for the next step. |
| 6 | Contact dispatch | `physics-contact-list.cpp`, `physics-contact-dispatch.cpp` | Gives each contact a phase, calls the collider procs, and publishes a collision event. See [Contacts](#contacts). |

Detection runs before integration, so it sweeps each moving entity's bounds by `velocity * dt` to catch pairs that will touch during the step. Those pairs are only candidates. Only the contacts from resolution are reported.

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
2. Builds an AABB for each one that is updating and has a position and a [body box](#bounding-boxes), swept by its velocity if it has one. It also reads each entity's [collider](#colliders) masks.
3. Builds a BVH from those boxes and queries it for pairs whose masks accept each other and whose boxes overlap. The mask check runs first, since it's cheaper.
4. Adds the pairs to the collision list. Pairs are stored as `(lower id, higher id)`, so each pair is only listed once.

All of this memory comes from a temporary arena that's freed at the end of detection. Detection doesn't publish anything. The collision list only feeds resolution.

### Bounding Boxes

An entity's body box comes from `phys_aabb_get_body_box`:

- With a **bounds** component (`cmpnt_bounds`), the box is `width × height`, centered at the position plus `(x, y)`. It's given a depth of `width` along z.
- Otherwise, with a **quad**, the box is centered on the position. The quad is `width × height`, and it's given a depth of `width` along z so it has volume to overlap with (`phys_aabb_half_extent_quad`).
- An entity with neither isn't part of collision.

### BVH

`physics-bvh.cpp` builds a bounding volume hierarchy for each chunk on every step:

- The root holds every entity. Each node is split along the longest axis of its entities' centers, at the median (found with quickselect), so the tree stays balanced and its depth is `log2(n)`.
- Interior nodes have `count == 0` and two children at `child_left_or_first` and `child_left_or_first + 1`. Leaf nodes have `count > 0` and hold entities `child_left_or_first` through `child_left_or_first + count - 1`.
- A tree for `n` entities has at most `2n - 1` nodes.

## Collision Resolution

Resolution moves positions directly and doesn't depend on `dt`. For each pair from detection:

1. Look up both entities. An entity without an inverse mass, or tagged with `cmpnt_type_e_immovable`, is treated as immovable. The tag only affects collisions, the integrator still moves the entity by its inverse mass.
2. Check that the boxes still overlap after integration, and find the axis with the smallest overlap.
3. Record a [contact](#contacts) for the pair. This happens before anything below decides not to push, so triggers and pairs that can't move are still reported.
4. If either entity is a trigger, or neither can move, stop here.
5. Push both entities apart along that axis, split by inverse mass, so the lighter entity moves more.
6. If they're moving toward each other on that axis, cancel the closing velocity so they stop at the contact.
7. Push map entities back inside their map, then write the position and velocity back to the components.

There's no bounce. Entities stop at the contact point.

## Colliders

A `cmpnt_collider` component (`cmpnt_type_e_collider`) controls what an entity collides with and lets it react to its own collisions.

| Field | Description |
| --- | --- |
| `proc` | Called with the entity's own contacts after the step. `NULL` for none. |
| `mask_category` | The bits that say what this entity is. |
| `mask_collides_with` | The bits for the categories this entity collides with. |
| `is_trigger` | A trigger reports its contacts, but never pushes or gets pushed. |

A pair is only considered when both sides accept each other:

```
(a.mask_collides_with & b.mask_category) != 0 &&
(b.mask_collides_with & a.mask_category) != 0
```

An entity without a collider uses `PHYS_CLLSN_MASK_ALL` (`0xFFFFFFFF`) for both masks and is solid, so it collides with everything that accepts it. Its category has every bit set, so it's only filtered out by a collider whose `mask_collides_with` is `0`. Masks start to matter once both sides of a pair have colliders.

```cpp
cmpnt_collider clldr;
clldr.proc               = box_on_collision;
clldr.mask_category      = bit_value(1); // a prop
clldr.mask_collides_with = bit_value(0); // only the player
clldr.is_trigger         = false;

eng_entity_add_components(id, cmpnt_type_e_collider);
eng_cmpnt_update_collider(id, clldr);
```

> **Note:** adding a component doesn't clear its slot, so always write the whole collider after adding it. Otherwise it can keep a `proc` from an entity that used the slot before.

## Contacts

A contact is a pair whose boxes actually overlap after integration. Resolution records them in the current **contact list** (`phys_cntct_list`), with `a` as the lower ID:

| Field | Description |
| --- | --- |
| `normal` | A unit vector along the axis of least overlap, pointing from `a` to `b`. |
| `depth` | How far they overlap on that axis, `(half_a + half_b) - abs(center_b - center_a)`. |

The physics manager keeps two lists, this step's and the last step's, and swaps them at the start of each step. After the step, `phys_cntct_list_walk_phases` compares them and gives each contact a phase (`phys_cllsn_phase_e`):

| Phase | When | Data |
| --- | --- | --- |
| `begin` | Touching this step, not the last | This step's normal and depth |
| `stay` | Touching this step and the last | This step's normal and depth |
| `end` | Touching the last step, not this one | The last step's normal, with a depth of `0` |

So a pair that stays in contact gets one `begin`, a `stay` every step after that, and one `end` when it separates. If two entities separate for even one step, touching again is a new `begin`.

### Dispatch

For each contact and its phase, `phys_cntct_dispatch`:

1. Publishes an `event_type_e_physics_collision` [event](events.md#physics-collisions) with `a`, `b`, the normal, depth, and phase.
2. Calls `a`'s collider proc, with `self = a`, `other = b`, and the normal as is.
3. Calls `b`'s collider proc, with `self = b`, `other = a`, and the normal flipped, so it always points away from `self`.

```cpp
IFB_INTERNAL void
box_on_collision(
    const phys_cllsn_contact& contact) {

    if (contact.phase != phys_cllsn_phase_e_begin) return;
    // contact.self is the box, contact.other is what hit it
}
```

The procs run right away, after the step, while the events wait for `event_mngr_update`. Each entity is looked up again before its proc is called, so a proc can destroy entities. Forces a proc adds land in the next step.

## Configuration

| Setting | Description |
| --- | --- |
| `IFB_CONFIG_ENTITY_CAPACITY` | The maximum number of entities the integrator, spring calculator, collision list, and each contact list can hold. Resolution asserts if a contact list fills up. |

The physics system allocates its memory from the `physics` reservation in the memory map.
