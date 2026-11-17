# Physics

The physics system (`IFB.Engine/src/physics`, `eng-physics.hpp`) moves entities with forces, springs, and drag, and keeps them from overlapping each other and the map. It works directly on the entity components described in [Entities and Components](entities-and-components.md).

## Simulation Step

`phys_mngr_simulate(dt)` runs once per simulation step of the [frame loop](architecture.md#frame-loop), after the game callback, with the fixed step length `config::sim_step_s` in seconds.

| Order | Step | Source | Description |
| --- | --- | --- | --- |
| 0 | Swap contact lists | `physics-manager.cpp` | The last step's contacts become the previous list, so phases can be found by comparing the two. |
| 1 | Collision detection | `physics-collision-detection.cpp`, `physics-bvh.cpp` | Builds a BVH for each map and collects the pairs of entities whose bounds overlap and whose [masks](#colliders) allow it. |
| 2 | Springs | `physics-spring.cpp` | Adds a spring force for every entity with a spring component. |
| 3 | Integration | `physics-integrator.cpp` | Turns the accumulated forces into new velocities and positions, then keeps map entities inside their map. |
| 4 | Collision resolution | `physics-collision-resolver.cpp` | Records a contact for each pair from step 1 that still overlaps after integration, pushes the solid ones apart, and bounces them by their [restitution](#restitution). |
| 5 | Reset | `physics-force-accumulator.cpp` | Clears the forces for the next step. |
| 6 | Contact dispatch | `physics-contact-list.cpp`, `physics-contact-dispatch.cpp` | Gives each contact a phase, calls the collider procs, and publishes a collision event. See [Contacts](#contacts). |

Detection runs before integration, so it sweeps each moving entity's bounds by `velocity * dt` to catch pairs that will touch during the step. Those pairs are only candidates. Only the contacts from resolution are reported.

## Forces

Forces are collected in the **force accumulator** (`phys_frc_accmltr`), which holds one summed `vec3` per entity. Add forces through the public API:

```cpp
eng_entity_add_force(id, { 0.0f, 0.0f, 5.0f });
```

Forces only last for one step, so anything continuous, like movement input, has to be added every frame.

The accumulator has one slot per entity, indexed by the entity's sparse index, and every slot is zeroed at the end of the step.

## Integration

Every entity that has every component in `ENTITY_ARCHETYPE_PHYSICS_PARTICLE` (position, velocity, acceleration, inverse mass, drag, and terminal velocity) and whose [scene is updating](scenes.md#what-stops) is integrated each step, whether or not it was given a force. An entity that stops being pushed keeps moving, and its drag slows it down.

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

Most entities share a few drag values, so `drag^dt` is cached for the last few drag values seen in a step instead of being worked out for every entity.

If the entity has a map coordinates component, it's then pushed back inside its map (see [Maps](maps.md#movement-constraints)). The inset is half the width of its [body box](#bounding-boxes), measured from the box's center, so the whole box stays on the map.

## Springs

An entity with a position and a `cmpnt_spring` component is pulled toward its `anchor` entity:

```
extension = distance(entity, anchor) - rest_length
force     = direction_to_anchor * (stiffness * extension - damping * velocity_along_spring)
```

The force goes into the accumulator like any other force, so the spring entity also needs the physics particle components to move. The anchor isn't affected. Springs closer than `0.001` to their anchor are skipped.

The game attaches Jig to a `JIG-ANCHOR` entity with a spring, and moves the anchor to make Jig follow along (`game-player-rig.cpp`).

## Collision Detection

Collision is only checked between entities on the same **map**. Detection:

1. Makes one pass over every entity, and builds an AABB for each one that is updating and has a position, map coordinates, and a [body box](#bounding-boxes). The box is grown by half of [`PHYS_CNTCT_SKIN`](#contact-skin) on each side, and swept by the entity's velocity if it has one. It also reads each entity's [collider](#colliders) masks and the map it's on.
2. For each map with at least two of those entities, builds a BVH from their boxes and queries it for pairs whose masks accept each other and whose boxes overlap. The mask check runs first, since it's cheaper.
3. Adds the pairs to the collision list as `(lower id, higher id)`.

Every entity is in exactly one BVH, and the query only takes a pair from the side with the lower index, so each pair is found once and the list doesn't check for duplicates. Pairs that cross a chunk border are found like any other.

All of this memory comes from a temporary arena that's freed at the end of detection. Detection doesn't publish anything. The collision list only feeds resolution.

The collision list holds `IFB_CONFIG_ENTITY_CAPACITY` pairs. A pair that doesn't fit is dropped and never resolved, and it's counted in the [stats](#stats) as `count_pairs_dropped`.

### Bounding Boxes

An entity's body box comes from `phys_aabb_get_body_box`:

- With a **bounds** component (`cmpnt_bounds`), the box is `width × height × depth`, centered at the position plus `(x, y)`. Set `depth` to `width` for a box as deep as it's wide, like the characters. A thin `depth` or `width` makes a slab, like the game's scene triggers along walls.
- Otherwise, with a **quad**, the box is centered on the position. The quad is `width × height`, and it's given a depth of `width` along z so it has volume to overlap with (`phys_aabb_half_extent_quad`).
- An entity with neither isn't part of collision.

### BVH

`physics-bvh.cpp` builds a bounding volume hierarchy for each map on every step:

- The root holds every entity. Each node with more than `PHYS_BVH_LEAF_MAX` (4) entities is split along the longest axis of its entities' centers, at the median, so the tree stays balanced and its depth is `log2(n)`. The median is found with a three way quickselect, so entities whose centers line up, like quads in a row or resting on the same floor, don't make it slow.
- The build sorts small `phys_bvh_ref`s (a center and an index) instead of the ids, boxes, and masks, then gathers the entities into leaf order in one pass at the end.
- Interior nodes have `count == 0` and two children at `child_left_or_first` and `child_left_or_first + 1`. Leaf nodes have `count > 0` and hold entities `child_left_or_first` through `child_left_or_first + count - 1`.
- A tree for `n` entities has at most `2n - 1` nodes.

#### Querying on Worker Threads

`phys_bvh_query_pairs` only reads the tree, so it splits the entities into ranges and queries them as [jobs](jobs.md):

- With no worker threads, or fewer than 128 entities (`PHYS_BVH_QUERY_JOB_MIN_ENTITIES`), the whole query runs on the main thread.
- Otherwise there are up to 4 ranges per thread (the workers plus the main thread), and no range is smaller than 64 entities. Having more ranges than threads keeps a crowded range from holding everyone up.
- The main thread queries the first range itself while the workers do the rest, then waits on a [job counter](jobs.md#waiting-on-a-batch) for just those jobs. If the job queue is full, that range runs on the main thread instead.
- Each range writes its pairs to its own buffer in the detection arena, and the buffers are added to the collision list in range order, so the list comes out the same as it would from one thread.

## Collision Resolution

Resolution moves positions directly and doesn't depend on `dt`. For each pair from detection:

1. Look up both entities. An entity without an inverse mass, or tagged with `cmpnt_type_e_immovable`, is treated as immovable. The tag only affects collisions, the integrator still moves the entity by its inverse mass.
2. Check that the boxes still touch after integration, overlapping or within the [skin](#contact-skin) of each other, and find the axis with the smallest overlap.
3. Record a [contact](#contacts) for the pair. This happens before anything below decides not to push, so triggers and pairs that can't move are still reported.
4. If either entity is a trigger, the boxes are only touching in the skin, or neither can move, stop here.
5. Push both entities apart along that axis, split by inverse mass, so the lighter entity moves more.
6. If they're moving toward each other on that axis, cancel the closing velocity, then give back the [restitution](#restitution)'s share of it so they bounce apart.
7. Push map entities back inside their map, then write the position and velocity back to the components.

### Restitution

`cmpnt_collider::restitution` is how much of the closing speed a collision gives back: `0` stops dead at the contact, and `1` is a perfect bounce. A pair bounces with the larger of its two values, clamped to `0`–`1`, so a bouncy entity bounces off of anything. An entity without a collider has a restitution of `0`.

```
impulse = -(1 + restitution) * closing_velocity / (inv_mass_a + inv_mass_b)
```

### Contact Skin

Pushing a pair apart leaves the boxes exactly touching, with an overlap of `0`. Without some tolerance, an entity standing still against another would report `end` on the next step, and `begin` again whenever it leaned back in.

`PHYS_CNTCT_SKIN` (`eng-physics.hpp`, `0.005`, about a pixel and a half on a character) is that tolerance. Boxes within the skin of each other on every axis count as touching, and get a contact with a depth of `0`. Only a real overlap is pushed apart, so the skin never adds a gap between entities.

## Colliders

A `cmpnt_collider` component (`cmpnt_type_e_collider`) controls what an entity collides with and lets it react to its own collisions.

| Field | Description |
| --- | --- |
| `proc` | Called with the entity's own contacts after the step. `NULL` for none. |
| `mask_category` | The bits that say what this entity is. |
| `mask_collides_with` | The bits for the categories this entity collides with. |
| `is_trigger` | A trigger reports its contacts, but never pushes or gets pushed. |
| `restitution` | How bouncy the entity is, from `0` to `1`. See [Restitution](#restitution). |

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
clldr.restitution        = 0.0f;

eng_entity_add_components(id, cmpnt_type_e_collider);
eng_cmpnt_update_collider(id, clldr);
```

> **Note:** adding a component doesn't clear its slot, so always write the whole collider after adding it. Otherwise it can keep a `proc` from an entity that used the slot before.

## Contacts

A contact is a pair whose boxes actually overlap after integration. Resolution records them in the current **contact list** (`phys_cntct_list`), with `a` as the lower ID:

| Field | Description |
| --- | --- |
| `normal` | A unit vector along the axis of least overlap, pointing from `a` to `b`. |
| `depth` | How far they overlap on that axis, `(half_a + half_b) - abs(center_b - center_a)`. `0` when they're only touching in the skin. |

The physics manager keeps two lists, this step's and the last step's, and swaps them at the start of each step. After the step, `phys_cntct_list_walk_phases` compares them and gives each contact a phase (`phys_cllsn_phase_e`):

| Phase | When | Data |
| --- | --- | --- |
| `begin` | Touching this step, not the last | This step's normal and depth |
| `stay` | Touching this step and the last | This step's normal and depth |
| `end` | Touching the last step, not this one | The last step's normal, with a depth of `0` |

So a pair that stays in contact gets one `begin`, a `stay` every step after that, and one `end` when it separates. If two entities separate for even one step, touching again is a new `begin`.

### Dispatch

For each contact and its phase, `phys_cntct_dispatch`:

1. Publishes an `event_type_e_physics_collision` [event](events.md#physics-collisions) with `a`, `b`, the normal, depth, and phase. If the event queue is full, the event is dropped and counted in the [stats](#stats) as `count_events_dropped`. The procs still run.
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

## Stats

`eng_physics_get_stats(stats)` fills an `eng_physics_stats` (`ifb-engine.hpp`) with what the last step did, for tools like the [physics test](scenes.md#the-physics-test):

| Field | Description |
| --- | --- |
| `count_pairs`, `capacity_pairs` | The candidate pairs from detection, and how many the collision list holds |
| `count_pairs_dropped` | Pairs that didn't fit in the collision list, so they were never resolved |
| `count_contacts` | Pairs that actually touched after integration |
| `count_resolved` | Contacts that were pushed apart |
| `count_events_dropped` | Collision events that didn't fit in the event queue. The procs still ran for them. |
| `ms_detect`, `ms_springs`, `ms_integrate`, `ms_resolve`, `ms_dispatch`, `ms_total` | How long each part of the step took, in milliseconds |

## Configuration

| Setting | Description |
| --- | --- |
| `IFB_CONFIG_ENTITY_CAPACITY` | The maximum number of entities the integrator, spring calculator, collision list, and each contact list can hold. Collision pairs past it are dropped and counted, and resolution asserts if a contact list fills up. |

The physics system allocates its memory from the `physics` reservation in the memory map.
