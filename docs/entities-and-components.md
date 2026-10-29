# Entities and Components

The engine uses an entity-component layout: an entity is an ID with a set of components, and each component type is stored in its own array.

## Entities

An entity is created from a tag of up to `ENTITY_TAG_SIZE` (16) characters. The `entity_id` is the hash of the tag, so the same tag always maps to the same ID, and tags must be unique.

```cpp
entity_id id = eng_entity_create("CONNOR");
```

The entity manager (`entity_mngr`, `eng-entity.hpp`) stores entities in a **sparse set**:

- The **dense** arrays hold every live entity packed together: its ID, tag, archetype, and index into the sparse array.
- The **sparse** array is a hash table keyed by entity ID (linear probing), holding each entity's dense index.

Iterating over entities walks the dense arrays. Capacity is set by `IFB_CONFIG_ENTITY_CAPACITY`, and the sparse array is sized to stay under `IFB_CONFIG_SPARSE_SET_MAX_LOAD_P100` load.

| Function | Description |
| --- | --- |
| `eng_entity_create` | Create an entity from a tag |
| `eng_entity_destroy_by_id` / `_by_tag` | Destroy an entity |
| `eng_entity_lookup_by_id` / `_by_dense_index` | Get an `entity` (tag, ID, archetype, indexes) |
| `eng_entity_add_components` / `_remove_components` | Change which components an entity has |
| `eng_entity_query` | Fill an `entity_list` with entities matching an `entity_query` |
| `eng_entity_add_force` | Add a force for the next physics step |
| `eng_entity_set_sprite` | Set the sprite component from atlas and sprite names |
| `eng_entity_render` | Draw an entity |

### Queries

An `entity_query` matches against each entity's component mask:

```cpp
entity_query query;
query.has_all  = cmpnt_type_e_position | cmpnt_type_e_sprite;
query.has_any  = 0;
query.has_none = cmpnt_type_e_particle_emitter;

eng_entity_query(list, query);
```

## Components

Component types are bit flags (`cmpnt_type_e`), so an entity's set of components is a single `component_type` mask, stored as its `entity_archetype`.

| Component | Flag | Type | Description |
| --- | --- | --- | --- |
| Position | `cmpnt_type_e_position` | `vec3` | World position. The source of truth for where an entity is. |
| Color | `cmpnt_type_e_color` | `color_rgba_u32` | Tint or solid color. |
| Quad | `cmpnt_type_e_quad` | `cmpnt_quad` | Width and height of the entity's quad. |
| Velocity | `cmpnt_type_e_velocity` | `vec3` | |
| Acceleration | `cmpnt_type_e_acceleration` | `vec3` | |
| Inverse mass | `cmpnt_type_e_inv_mass` | `cmpnt_inv_mass` | |
| Drag | `cmpnt_type_e_drag` | `cmpnt_drag` | |
| Terminal velocity | `cmpnt_type_e_term_velocity` | `vec3` | Maximum speed per axis. |
| Spring | `cmpnt_type_e_spring` | `cmpnt_spring` | Attaches the entity to an anchor entity, with stiffness, damping, and rest length. |
| Map coordinates | `cmpnt_type_e_map_coords` | `cmpnt_map_coords` | The map the entity is on, and its position in tiles (column, level, row). |
| Particle emitter | `cmpnt_type_e_particle_emitter` | `cmpnt_particle_emitter` | An emitter that follows the entity. |
| Sprite | `cmpnt_type_e_sprite` | `cmpnt_sprite` | Atlas handle and sprite index. |

The component manager (`cmpnt_mngr`, `eng-component.hpp`) keeps a `cmpnt_table` with one array per component, indexed by the entity's sparse index.

Every component has a lookup and update function:

```cpp
cmpnt_position pos;
if (eng_cmpnt_lookup_position(id, pos)) {
    pos.x += 1.0f;
    eng_cmpnt_update_position(id, pos);
}
```

## Archetypes

An archetype is a common set of components that can be read and written together in one call.

| Archetype | Mask | Struct | Components |
| --- | --- | --- | --- |
| Sprite | `ENTITY_ARCHETYPE_SPRITE` | `atype_sprite` | position, color, quad, map coords, sprite |
| Physics particle | `ENTITY_ARCHETYPE_PHYSICS_PARTICLE` | `atype_particle` | position, velocity, acceleration, inverse mass, drag, terminal velocity |
| Physics sprite | `ENTITY_ARCHETYPE_PHYSICS_SPRITE` | `atype_physics_sprite` | everything in sprite and physics particle |

```cpp
entity_id id = eng_entity_create("JIG");
eng_entity_add_components(id, ENTITY_ARCHETYPE_PHYSICS_SPRITE);

atype_physics_sprite ps;
eng_atype_lookup_physics_quad(id, ps);
ps.velocity = { 0.0f, 0.0f, 0.0f };
eng_atype_update_physics_sprite(id, ps);
```

## Tools

**Entities > Entity Manager** in the GUI creates entities by tag and lists every entity with its indexes and archetype, and **Entities > Quad Manager** shows the quads being drawn.
