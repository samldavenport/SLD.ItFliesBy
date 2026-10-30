# Events

The event system (`IFB.Engine/src/events`, `eng-events.hpp`) lets engine systems tell the game that something happened, without either side knowing about the other. Systems publish events to a queue, and the queue is sent to the subscribed listeners once per pass of the [frame loop](architecture.md#frame-loop).

Events here are engine events. Window messages are handled separately, see [Platform](platform.md#events).

## Listening for Events

A listener is a plain function that takes the payload. Subscribe it with the event type:

```cpp
IFB_INTERNAL void
game_phys_cllsn_handler(
    const event_payload& payload) {

    const entity_id a = payload.data.physics_collision.a;
    const entity_id b = payload.data.physics_collision.b;
}

const bool did_subscribe = eng_event_listener_subscribe(event_type_e_physics_collision, game_phys_cllsn_handler);
assert(did_subscribe);
```

Subscribe after `eng_context_startup`, since that's what creates the listener table. The game does it in `game_context_create_and_init`.

- A listener only gets the payload, not the event type, so a function subscribed to several types can't tell them apart from its arguments.
- Subscribing the same function twice calls it twice per event. Each unsubscribe removes one of them.
- There's no user data pointer, so a listener that needs state has to get it from a global.

## How It Works

| Piece | Source | Description |
| --- | --- | --- |
| **Manager** (`event_mngr`) | `event-mngr.cpp` | The singleton that owns the queue and the listener table, and the `events` reservation they're allocated from. |
| **Queue** (`event_queue`) | `event-queue.cpp` | A fixed array of events (type and payload) and a count. |
| **Listener table** (`event_listener_table`) | `event-listener-table.cpp` | For each event type, a fixed array of function pointers and a count. |

`eng_event_listener_subscribe` and `_unsubscribe` (`eng-events.cpp`) are thin wrappers over the manager, and are the only part of the system the game and GUI can call.

### Flow of an Event

1. A system fills in an `event_payload` and calls `event_mngr_publish`, which copies the type and payload to the end of the queue. Nothing is called yet.
2. `eng_context_run` calls `event_mngr_update` after `phys_mngr_simulate`.
3. `event_queue_execute_and_reset` goes through the queue in the order events were published. For each one, `event_listener_table_execute` calls every listener for that type, one after another.
4. The queue's count is set back to `0`.

Dispatch happens on the same thread as the rest of the frame loop, so a slow listener slows down the whole pass.

### Timing

`event_mngr_update` runs every iteration of the loop, not just on new frames, so the queue is emptied at least as often as physics runs.

| Order | Step |
| --- | --- |
| 1 | Game callback |
| 2 | `phys_mngr_simulate`, which publishes collision events during detection |
| 3 | `event_mngr_update`, which calls the listeners |
| 4 | Particles, then rendering on a new frame |

Listeners see the positions and velocities left by collision resolution, and anything they change is what gets rendered this pass. The next game callback runs after them.

### Listener Removal

Removing a listener moves the last listener in the array into its slot, so listeners aren't guaranteed to run in the order they subscribed.

Don't subscribe or unsubscribe from inside a listener. `event_listener_table_execute` reads the count once before its loop, so changing the table mid-dispatch can skip a listener, call one twice, or call one that was just removed.

## Event Types

Types are defined by `event_type_e` in `IFB.Core/include/ifb.hpp`.

| Type | Payload | Published by |
| --- | --- | --- |
| `event_type_e_physics_collision` | `event_data_physics_collision` (`a`, `b`) | `phys_cllsn_dtctr_run` |
| `event_type_e_key_down`, `_key_up` | None yet | Nothing yet |
| `event_type_e_mouse`, `_gamepad` | None yet | Nothing yet |

### Physics Collisions

After [collision detection](physics.md#collision-detection) finishes, it publishes one event for each pair in the collision list. Pairs are stored as `(lower id, higher id)`, so `a` is always the lower ID.

A collision event doesn't guarantee that the two entities touched:

- Detection uses bounds swept by velocity, and runs before integration, so a pair can be reported and then never overlap.
- Events are published for every pair, including pairs that resolution skips because neither entity can move.
- The list is rebuilt on every run, so a pair that stays in contact publishes an event on every pass. There are no separate "enter" and "exit" events. To react once per contact, keep the previous state yourself and compare.

## Adding an Event Type

1. Add a value to `event_type_e` before `event_type_e_count`. Values have to stay under `IFB_CONFIG_EVENT_TYPE_MAX`.
2. If it carries data, add a struct to the union in `event_payload`. The union is 64 bytes, and a bigger struct makes every queued event bigger.
3. Call `event_mngr_publish` from the engine system. It's `IFB_INTERNAL`, so only the engine can publish. There's no public API for the game or GUI to do it.

## Limits

Everything is a fixed size, set in `IFB.Core/include/ifb-config.hpp` and allocated once at startup.

| Setting | Value | What happens at the limit |
| --- | --- | --- |
| `IFB_CONFIG_EVENT_QUEUE_CAPACITY` | 128 | `event_mngr_publish` returns `false` and the event is dropped |
| `IFB_CONFIG_EVENT_TYPE_MAX` | 64 | Asserts on publish, subscribe, and unsubscribe |
| `IFB_CONFIG_EVENT_MAX_LISTENERS_PER_EVENT` | 16 | `eng_event_listener_subscribe` returns `false` |

> **Note:** collision detection asserts that every publish succeeds, so more than 128 collision pairs in one pass trips that assert. If asserts are compiled out, the extra events are dropped.

## Public API

| Function | Description |
| --- | --- |
| `eng_event_listener_subscribe` | Add a listener for an event type. Returns `false` if that type's listeners are full. |
| `eng_event_listener_unsubscribe` | Remove a listener. Returns `false` if it wasn't subscribed. |
