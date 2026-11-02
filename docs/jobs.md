# Jobs

The job system (`IFB.Engine/src/jobs`, `eng-jobs.hpp`) runs work on a pool of worker threads. Workers sleep until a job is added, so an idle pool doesn't use any CPU time.

The frame loop doesn't use jobs yet, and neither does the game. Everything still runs on the main thread unless something calls `eng_job_add`.

## Adding Jobs

A job is a function and up to 64 bytes of context. The types are in `IFB.Core/include/ifb.hpp`.

```cpp
struct update_ctx {
    particle_data* data;
    u32            count;
};
static_assert(sizeof(update_ctx) <= sizeof(job_ctx), "update_ctx doesn't fit in a job");

static void
update_job(
    const hnd_arena h_arena,
    void*           ctx) {

    auto update = (update_ctx*)ctx;
    // ... work on update->data ...
}

job j = {};
j.proc = update_job;
*(update_ctx*)&j.ctx = {data, count};

const bool did_add = eng_job_add(j);

// ... add more jobs, or do other work ...

eng_job_wait();
```

`data` and `count` stand in for your own data.

### Context

The context is copied when the job is added, so the caller's copy can go out of scope. It's aligned to 16 bytes, and nothing checks that your struct fits, so add a `static_assert` like the one above.

Whatever the context points to has to stay valid until the job finishes, and the pointer the job receives is only valid while it runs.

### Scratch Arena

Each worker owns one [arena](memory.md#arenas), which is reset before each job and passed in as `h_arena`. It holds about 1 MB. Anything a job pushes there is gone once the next job starts, so results have to go through a pointer in the context.

## Public API

| Function | Description |
| --- | --- |
| `eng_job_add` | Add a job to the queue and wake a worker. Returns `false` if the queue is full or the engine is shutting down, and the job won't run. |
| `eng_job_wait` | Block until every added job has finished, no matter who added it. |

Things to know:

- **Don't call `eng_job_wait` from a job.** The job counts as unfinished, so it would wait on itself forever.
- **Check the result of `eng_job_add`.** When the queue is full, the simplest options are to call the job's function directly or to call `eng_job_wait` and try again. A job can't wait, so a job adding more work has to run it directly.
- **Jobs start in the order they were added**, but they can finish in any order. There are no priorities or dependencies between jobs.
- **Waiting blocks the main thread.** It doesn't run jobs while it waits.

### Single Core Machines

With one CPU core there are no workers, so `eng_job_add` runs the job on the calling thread before it returns. It uses an arena set aside for this, saved and reverted around the job, and a job can add another job. Don't assume jobs run in parallel, or that `eng_job_add` returns before the job starts.

## Thread Rules

The main thread owns the game world and everything that talks to the GPU, an audio device, or the window. Workers never make those calls.

### Safe From a Job

| System | Rules |
| --- | --- |
| Jobs | `eng_job_add` from any thread. `eng_job_wait` from anything except a job. |
| Arenas | Any thread can allocate or free an arena, but only one thread can use a given arena at a time. |
| Files | Any thread can open or close a file, but only one thread can use an open file at a time. Don't open a path that's already open, because both opens get the same handle. |
| Text and images | `eng_text_*` and `eng_image_*`, with a file and an arena that belong to the job. |
| Random | `eng_random_*`. Threads share one stream of values, so a job won't get a repeatable sequence. |
| Events | `event_mngr_publish`, from inside the engine. Listeners still run on the main thread during `event_mngr_update`. |
| Your own data | Stacks, index caches, and entity lists have no shared state, so a job can use any instance nothing else is using. |

### Main Thread Only

- OpenGL, audio, and anything else that talks to a device
- The window, input, GUI, and file dialogs
- Entities, components, archetypes, physics, maps, particles, and the camera
- Assets and sprites
- Subscribing and unsubscribing event listeners
- Engine startup and shutdown

Nothing enforces these rules, so breaking one is a data race rather than an error.

### Working on Game Data

To work on game data in a job, have the main thread pass pointers to the data in the job's context, split it so each job has its own range, and wait for the jobs before anything else touches it. The jobs can't call `eng_*` functions that are main-thread only, so the main thread collects what they need first and applies their results after the wait.

## How It Works

| Piece | Source | Description |
| --- | --- | --- |
| **Dispatcher** (`job_dspchr`) | `job-dispatcher.cpp` | The singleton that owns the queue, the pool, the lock, and the two condition variables. It also has the loop each worker runs. |
| **Queue** (`job_queue`) | `job-queue.cpp` | A ring buffer of jobs. It doesn't lock, so callers hold the dispatcher's lock. |
| **Worker pool** (`job_worker_pool`) | `job-worker-pool.cpp` | Creates the workers and joins them on shutdown. Each `job_worker` has a thread and an arena. |

`eng_job_add` and `eng_job_wait` (`eng-jobs.cpp`) are thin wrappers over the dispatcher.

### Flow of a Job

1. `eng_job_add` takes the lock, copies the job into the queue, adds one to `unfinished`, and releases the lock.
2. It wakes one worker with the `work_available` condition variable.
3. The worker takes the lock, copies the job out of the queue, and releases the lock.
4. It resets its arena and runs the job with nothing locked.
5. It takes the lock and subtracts one from `unfinished`. If that was the last one, it wakes everything waiting on `all_done`.

`unfinished` counts jobs that are queued or running, so `eng_job_wait` can't return while a job is still running even when the queue is empty.

### Workers

`job_worker_pool_init` creates one worker per physical core, minus one for the main thread, up to `IFB_CONFIG_THREAD_CAPACITY`. A machine with 8 cores gets 7 workers. The scheduler decides where they run, since the pool doesn't set an ideal core or an affinity.

### Startup and Shutdown

`eng_context_create` calls `job_dspchr_create`, and `eng_context_startup` calls `job_dspchr_startup` after `memory_mngr_startup`, because workers need arenas. The queue is ready before the workers start, and it takes all of the `jobs` reservation.

`eng_context_shutdown` calls `job_dspchr_shutdown` before anything else. Queued jobs still finish, new jobs are refused, and every worker is joined. The game calls it when the main loop ends.

## Limits

| Setting | Value | What happens at the limit |
| --- | --- | --- |
| `jobs` reservation | 64 KB, or 819 jobs at 80 bytes each | `eng_job_add` returns `false` |
| `IFB_CONFIG_THREAD_CAPACITY` | 32 | Extra cores don't get a worker |
| `sizeof(job_ctx)` | 64 bytes | Nothing checks this, so use a `static_assert` |

The reservation size is `SIZE_THREADS` at the top of `game-win32-main.cpp`.

This design suits a moderate number of jobs that each do a fair amount of work. Every add and every worker's pickup goes through one lock, so thousands of tiny jobs per frame would spend real time on it.
