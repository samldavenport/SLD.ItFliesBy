# Jobs

The job system (`IFB.Engine/src/jobs`, `eng-jobs.hpp`) runs work on a pool of worker threads. Workers sleep until a job is added.

## Adding Jobs

A job is a function and up to 64 bytes of context, which is copied when the job is added. Each job gets a scratch arena that's reset before the job runs, so anything that has to outlive the job needs to go through a pointer in its context.

```cpp
struct update_ctx {
    particle* particles;
    u32       count;
};
static_assert(sizeof(update_ctx) <= sizeof(job_ctx), "update_ctx doesn't fit in a job");

static void
update_job(
    const hnd_arena h_arena,
    void*           ctx) {

    auto update = (update_ctx*)ctx;
}

job j = {};
j.proc = update_job;
*(update_ctx*)&j.ctx = {particles, count};

if (!eng_job_add(j)) {
    update_job(h_arena, &j.ctx);
}

eng_job_wait();
```

`eng_job_add` returns `false` if the queue is full or the engine is shutting down. `eng_job_wait` blocks until every added job has finished, so don't call it from a job, because that job would be waiting on itself.

With one CPU core there are no workers, so `eng_job_add` runs the job before it returns.

## Thread Rules

The main thread owns the game world and everything that talks to the GPU, the audio device, or the window. Workers never make those calls.

### Safe From a Job

| System | Rules |
| --- | --- |
| Jobs | `eng_job_add` from any thread. `eng_job_wait` from anything except a job. |
| Arenas | Any thread can allocate or free an arena, but only one thread can use a given arena at a time. |
| Files | Any thread can open or close a file, but only one thread can use an open file at a time. Don't open the same path twice at once, because both opens get the same handle. |
| Text and images | `eng_text_*` and `eng_image_*`, with a file and an arena that belong to the job. |
| Random | `eng_random_*`. Values are shared between threads, so a job won't get a repeatable sequence. |
| Events | `event_mngr_publish`. Listeners still run on the main thread during `event_mngr_update`. |
| Your own data | Stacks, index caches, and entity lists have no shared state, so a job can use any instance nothing else is using. |

### Main Thread Only

- OpenGL, audio, and anything else that talks to a device
- The window, input, GUI, and file dialogs
- Entities, components, archetypes, physics, maps, particles, and the camera
- Assets and sprites
- Subscribing and unsubscribing event listeners
- Engine startup and shutdown

To work on game data in a job, have the main thread pass pointers to the data in the job's context, split it so each job has its own range, and wait for the jobs before anything else touches it.

## Shutdown

`eng_context_shutdown` calls `job_dspchr_shutdown` before anything else. Queued jobs still finish, new jobs are refused, and every worker is joined.
