#pragma once

#include <cstring>
#include "ifb.hpp"
#include "ifb-config.hpp"
#include "eng-context.hpp"
#include "eng-entity.hpp"
#include "eng-events.hpp"
#include "eng-memory.hpp"
#include "eng-particle.hpp"
#include "eng-scene.hpp"

namespace ifb {

    constexpr const cchar* SCENE_DEFAULT_NAME = "default";

    //--------------------------------------------------------------------
    // INLINE METHODS
    //--------------------------------------------------------------------

    inline scene_mngr::stack&
    scene_stack_next(
        void) {

        return(_scene_mngr->stack_next);
    }

    inline void
    scene_stack_init(
        scene_mngr::stack& stack,
        const u32          capacity) {

        stack.hnd   = global_alloc<hnd_scene>      (capacity);
        stack.flags = global_alloc<scene_flags_u32>(capacity);
        stack.count = 0;
        assert(stack.hnd);
        assert(stack.flags);

        for (
            u32 index = 0;
                index < capacity;
              ++index) {

            stack.hnd   [index] = INVALID_HANDLE;
            stack.flags [index] = scene_flag_e_none;
        }
    }

    inline bool
    scene_stack_contains(
        const scene_mngr::stack& stack,
        const hnd_scene          h_scene) {

        for (
            u32 index = 0;
                index < stack.count;
              ++index) {

            if (stack.hnd[index] == h_scene) return(true);
        }
        return(false);
    }

    inline bool
    scene_stack_equals(
        const scene_mngr::stack& a,
        const scene_mngr::stack& b) {

        if (a.count != b.count) return(false);

        for (
            u32 index = 0;
                index < a.count;
              ++index) {

            if (a.hnd   [index] != b.hnd   [index]) return(false);
            if (a.flags [index] != b.flags [index]) return(false);
        }
        return(true);
    }

    inline void
    scene_stack_copy(
        scene_mngr::stack&       dst,
        const scene_mngr::stack& src) {

        for (
            u32 index = 0;
                index < src.count;
              ++index) {

            dst.hnd   [index] = src.hnd   [index];
            dst.flags [index] = src.flags [index];
        }
        dst.count = src.count;
    }

    // takes the scene off of the stack, wherever it is, keeping the order of the others
    inline void
    scene_stack_remove(
        scene_mngr::stack& stack,
        const hnd_scene    h_scene) {

        u32 write = 0;
        for (
            u32 read = 0;
                read < stack.count;
              ++read) {

            if (stack.hnd[read] == h_scene) continue;

            stack.hnd   [write] = stack.hnd   [read];
            stack.flags [write] = stack.flags [read];
            ++write;
        }
        stack.count = write;
    }

    inline hnd_scene
    scene_stack_top(
        const scene_mngr::stack& stack) {

        const hnd_scene h_top = (stack.count > 0)
            ? stack.hnd[stack.count - 1]
            : hnd_scene(INVALID_HANDLE);
        return(h_top);
    }

    // walks down from the top of the running stack
    // a scene runs if every scene above it lets the scenes below it run
    inline void
    scene_stack_refresh(
        void) {

        for (
            u32 index = 0;
                index < _scene_mngr->capacity;
              ++index) {

            _scene_mngr->tbl.is_updating  [index] = false;
            _scene_mngr->tbl.is_rendering [index] = false;
        }

        const scene_mngr::stack& stack = _scene_mngr->stack_running;

        bool is_updating  = true;
        bool is_rendering = true;
        for (
            u32 stack_index = stack.count;
                stack_index > 0;
              --stack_index) {

            const hnd_scene       h_scene = stack.hnd   [stack_index - 1];
            const scene_flags_u32 flags   = stack.flags [stack_index - 1];

            _scene_mngr->tbl.is_updating  [h_scene.val] = is_updating;
            _scene_mngr->tbl.is_rendering [h_scene.val] = is_rendering;

            is_updating  = is_updating  && ((flags.val & scene_flag_e_update_below) != 0);
            is_rendering = is_rendering && ((flags.val & scene_flag_e_render_below) != 0);
        }
    }

    inline void
    scene_parallax_clear(
        scene_parallax& plx) {

        memset(&plx, 0, sizeof(scene_parallax));
    }

    // releases everything the scene owns, the scene stays alive
    inline void
    scene_release_owned(
        const hnd_scene h_scene) {

        (void)entity_destroy_by_scene        (h_scene);
        (void)particle_mngr_destroy_by_scene (h_scene);
        arena_reset          (_scene_mngr->tbl.arena    [h_scene.val]);
        scene_parallax_clear (_scene_mngr->tbl.parallax [h_scene.val]);
    }

    //--------------------------------------------------------------------
    // SCENE MANAGER
    //--------------------------------------------------------------------

    IFB_INTERNAL scene_mngr*
    scene_mngr_create(
        void) {

        const config& cfg = config_instance();

        _scene_mngr = global_alloc<scene_mngr>();
        assert(_scene_mngr);

        const u32 capacity = cfg.scene_capacity;
        _scene_mngr->capacity               = capacity;
        _scene_mngr->count                  = 0;
        _scene_mngr->parallax_revision      = 0;
        _scene_mngr->h_bound                = INVALID_HANDLE;
        _scene_mngr->h_default              = INVALID_HANDLE;
        _scene_mngr->tbl.hnd                = global_alloc<hnd_scene>     (capacity);
        _scene_mngr->tbl.name               = global_alloc<tag>           (capacity);
        _scene_mngr->tbl.arena              = global_alloc<hnd_arena>     (capacity);
        _scene_mngr->tbl.parallax           = global_alloc<scene_parallax>(capacity);
        _scene_mngr->tbl.is_updating        = global_alloc<bool>          (capacity);
        _scene_mngr->tbl.is_rendering       = global_alloc<bool>          (capacity);
        _scene_mngr->tbl.is_pending_destroy = global_alloc<bool>          (capacity);
        _scene_mngr->tbl.is_pending_reset   = global_alloc<bool>          (capacity);
        assert(_scene_mngr->tbl.hnd);
        assert(_scene_mngr->tbl.name);
        assert(_scene_mngr->tbl.arena);
        assert(_scene_mngr->tbl.parallax);
        assert(_scene_mngr->tbl.is_updating);
        assert(_scene_mngr->tbl.is_rendering);
        assert(_scene_mngr->tbl.is_pending_destroy);
        assert(_scene_mngr->tbl.is_pending_reset);

        for (
            u32 index = 0;
                index < capacity;
              ++index) {

            _scene_mngr->tbl.hnd                [index] = INVALID_HANDLE;
            _scene_mngr->tbl.arena              [index] = INVALID_HANDLE;
            _scene_mngr->tbl.is_updating        [index] = false;
            _scene_mngr->tbl.is_rendering       [index] = false;
            _scene_mngr->tbl.is_pending_destroy [index] = false;
            _scene_mngr->tbl.is_pending_reset   [index] = false;
            _scene_mngr->tbl.name               [index].init("");
            scene_parallax_clear(_scene_mngr->tbl.parallax[index]);
        }

        scene_stack_init(_scene_mngr->stack_running, capacity);
        scene_stack_init(_scene_mngr->stack_next,    capacity);

        return(_scene_mngr);
    }

    IFB_INTERNAL void
    scene_mngr_validate(
        void) {

        assert(
            _scene_mngr                                                  &&
            _scene_mngr->tbl.hnd                != NULL                  &&
            _scene_mngr->tbl.name               != NULL                  &&
            _scene_mngr->tbl.arena              != NULL                  &&
            _scene_mngr->tbl.parallax           != NULL                  &&
            _scene_mngr->tbl.is_updating        != NULL                  &&
            _scene_mngr->tbl.is_rendering       != NULL                  &&
            _scene_mngr->tbl.is_pending_destroy != NULL                  &&
            _scene_mngr->tbl.is_pending_reset   != NULL                  &&
            _scene_mngr->stack_running.hnd      != NULL                  &&
            _scene_mngr->stack_next.hnd         != NULL                  &&
            _scene_mngr->capacity               != 0                     &&
            _scene_mngr->count                  <= _scene_mngr->capacity &&
            _scene_mngr->stack_running.count    <= _scene_mngr->capacity &&
            _scene_mngr->stack_next.count       <= _scene_mngr->capacity
        );
    }

    // the arenas need to be ready, so this goes after the memory manager
    IFB_INTERNAL void
    scene_mngr_startup(
        void) {

        scene_mngr_validate();

        // the default scene is the bottom of the stack
        // entities created before the game makes its own scenes belong to it
        _scene_mngr->h_default = scene_create(SCENE_DEFAULT_NAME);
        assert(_scene_mngr->h_default != INVALID_HANDLE);

        const bool did_push = scene_push(_scene_mngr->h_default, scene_flag_e_none);
        assert(did_push);

        // the default scene runs from the first frame
        scene_mngr_frame_start();
    }

    // applies everything requested since the last frame started
    // resets, then destroys, then the stack
    // if anything changed, the queued events are dropped, they can name entities that are gone
    IFB_INTERNAL void
    scene_mngr_frame_start(
        void) {

        scene_mngr_validate();

        bool did_change = false;

        for (
            u32 index = 0;
                index < _scene_mngr->capacity;
              ++index) {

            if (!_scene_mngr->tbl.is_pending_reset[index]) continue;
            _scene_mngr->tbl.is_pending_reset[index] = false;

            // a scene that is about to be destroyed gets released below
            if (_scene_mngr->tbl.is_pending_destroy[index]) continue;

            scene_release_owned(index);
            did_change = true;
        }

        for (
            u32 index = 0;
                index < _scene_mngr->capacity;
              ++index) {

            if (!_scene_mngr->tbl.is_pending_destroy[index]) continue;
            _scene_mngr->tbl.is_pending_destroy[index] = false;

            const hnd_scene h_scene = index;
            scene_release_owned(h_scene);
            scene_stack_remove(_scene_mngr->stack_running, h_scene);

            arena_free(_scene_mngr->tbl.arena[index]);
            _scene_mngr->tbl.hnd   [index] = INVALID_HANDLE;
            _scene_mngr->tbl.arena [index] = INVALID_HANDLE;
            _scene_mngr->tbl.name  [index].init("");
            --_scene_mngr->count;
            did_change = true;
        }

        if (!scene_stack_equals(_scene_mngr->stack_running, _scene_mngr->stack_next)) {
            scene_stack_copy(_scene_mngr->stack_running, _scene_mngr->stack_next);
            did_change = true;
        }

        if (did_change) {
            scene_stack_refresh();
            event_mngr_flush();
        }
    }

    IFB_INTERNAL u32
    scene_mngr_get_count(
        void) {

        scene_mngr_validate();
        return(_scene_mngr->count);
    }

    //--------------------------------------------------------------------
    // SCENE
    //--------------------------------------------------------------------

    IFB_INTERNAL hnd_scene
    scene_create(
        const cchar* name) {

        scene_mngr_validate();
        assert(name);

        // find a free slot
        // a scene waiting to be destroyed keeps its slot until the next frame
        u32 index = 0;
        for (
            ;
            index < _scene_mngr->capacity;
          ++index) {

            if (_scene_mngr->tbl.hnd[index] == INVALID_HANDLE) {
                break;
            }
        }
        if (index == _scene_mngr->capacity) {
            return(INVALID_HANDLE);
        }

        // every scene gets its own arena
        const hnd_arena h_arena = arena_alloc();
        if (h_arena == INVALID_HANDLE) {
            return(INVALID_HANDLE);
        }

        // a new scene isn't on the stack, so it doesn't run yet
        const hnd_scene h_scene = index;
        _scene_mngr->tbl.hnd                [index] = h_scene;
        _scene_mngr->tbl.arena              [index] = h_arena;
        _scene_mngr->tbl.is_updating        [index] = false;
        _scene_mngr->tbl.is_rendering       [index] = false;
        _scene_mngr->tbl.is_pending_destroy [index] = false;
        _scene_mngr->tbl.is_pending_reset   [index] = false;
        _scene_mngr->tbl.name               [index].init(name);
        scene_parallax_clear(_scene_mngr->tbl.parallax[index]);
        ++_scene_mngr->count;

        return(h_scene);
    }

    // the scene comes off the stack right away, and is released at the start of the next frame
    // the default scene can't be destroyed
    // if the scene was bound, the top of the stack is bound in its place
    IFB_INTERNAL bool
    scene_destroy(
        const hnd_scene h_scene) {

        if (!scene_is_valid(h_scene)) {
            return(false);
        }
        assert(h_scene != _scene_mngr->h_default);

        _scene_mngr->tbl.is_pending_destroy[h_scene.val] = true;
        scene_stack_remove(scene_stack_next(), h_scene);

        if (_scene_mngr->h_bound == h_scene) {
            _scene_mngr->h_bound = scene_get_top();
        }
        return(true);
    }

    // everything the scene owns is released at the start of the next frame
    // the scene stays alive and keeps its place on the stack
    // anything created in it before then is released too
    IFB_INTERNAL bool
    scene_reset(
        const hnd_scene h_scene) {

        if (!scene_is_valid(h_scene)) {
            return(false);
        }

        _scene_mngr->tbl.is_pending_reset[h_scene.val] = true;
        return(true);
    }

    // a scene waiting to be destroyed is still valid until the next frame, so its handle can still be used
    IFB_INTERNAL bool
    scene_is_valid(
        const hnd_scene h_scene) {

        scene_mngr_validate();

        const bool is_valid = (
            h_scene.val < _scene_mngr->capacity &&
            _scene_mngr->tbl.hnd[h_scene.val] == h_scene
        );
        return(is_valid);
    }

    IFB_INTERNAL void*
    scene_alloc(
        const hnd_scene h_scene,
        const u32       size) {

        if (!scene_is_valid(h_scene)) {
            return(NULL);
        }

        void* mem = arena_push(_scene_mngr->tbl.arena[h_scene.val], size);
        return(mem);
    }

    IFB_INTERNAL hnd_arena
    scene_get_arena(
        const hnd_scene h_scene) {

        const hnd_arena h_arena = scene_is_valid(h_scene)
            ? _scene_mngr->tbl.arena[h_scene.val]
            : hnd_arena(INVALID_HANDLE);
        return(h_arena);
    }

    IFB_INTERNAL const cchar*
    scene_get_name(
        const hnd_scene h_scene) {

        const cchar* name = scene_is_valid(h_scene)
            ? _scene_mngr->tbl.name[h_scene.val].cstr
            : NULL;
        return(name);
    }

    //--------------------------------------------------------------------
    // BINDING
    //--------------------------------------------------------------------

    IFB_INTERNAL bool
    scene_bind(
        const hnd_scene h_scene) {

        if (!scene_is_valid(h_scene)) {
            return(false);
        }

        _scene_mngr->h_bound = h_scene;
        return(true);
    }

    IFB_INTERNAL hnd_scene
    scene_get_bound(
        void) {

        scene_mngr_validate();
        return(_scene_mngr->h_bound);
    }

    IFB_INTERNAL hnd_scene
    scene_get_default(
        void) {

        scene_mngr_validate();
        return(_scene_mngr->h_default);
    }

    //--------------------------------------------------------------------
    // STACK
    //--------------------------------------------------------------------

    // puts the scene on top of the stack and binds it
    // a scene can only be on the stack once
    IFB_INTERNAL bool
    scene_push(
        const hnd_scene       h_scene,
        const scene_flags_u32 flags) {

        if (!scene_is_valid(h_scene))                         return(false);
        if (_scene_mngr->tbl.is_pending_destroy[h_scene.val]) return(false);
        if (scene_is_on_stack(h_scene))                       return(false);

        scene_mngr::stack& stack = scene_stack_next();
        assert(stack.count < _scene_mngr->capacity);

        stack.hnd   [stack.count] = h_scene;
        stack.flags [stack.count] = flags;
        ++stack.count;

        _scene_mngr->h_bound = h_scene;
        return(true);
    }

    // takes the top scene off of the stack and binds the new top
    // the scene isn't destroyed, it keeps everything it owns and can be pushed again
    // the default scene stays on the bottom
    IFB_INTERNAL bool
    scene_pop(
        void) {

        scene_mngr_validate();

        scene_mngr::stack& stack = scene_stack_next();
        if (stack.count <= 1) {
            return(false);
        }

        --stack.count;
        stack.hnd   [stack.count] = INVALID_HANDLE;
        stack.flags [stack.count] = scene_flag_e_none;

        _scene_mngr->h_bound = scene_get_top();
        return(true);
    }

    // replaces the top scene with this one, and destroys the old top
    // over the default scene, it just pushes
    IFB_INTERNAL bool
    scene_transition(
        const hnd_scene       h_scene,
        const scene_flags_u32 flags) {

        if (!scene_is_valid(h_scene))                         return(false);
        if (_scene_mngr->tbl.is_pending_destroy[h_scene.val]) return(false);
        if (scene_is_on_stack(h_scene))                       return(false);

        const hnd_scene h_top = scene_get_top();
        if (h_top != _scene_mngr->h_default) {
            const bool did_destroy = scene_destroy(h_top);
            assert(did_destroy);
        }

        const bool did_push = scene_push(h_scene, flags);
        assert(did_push);
        return(did_push);
    }

    // the top of the stack as of the last request, it runs from the start of the next frame
    IFB_INTERNAL hnd_scene
    scene_get_top(
        void) {

        scene_mngr_validate();
        return(scene_stack_top(scene_stack_next()));
    }

    IFB_INTERNAL u32
    scene_get_stack_count(
        void) {

        scene_mngr_validate();
        return(scene_stack_next().count);
    }

    IFB_INTERNAL bool
    scene_is_on_stack(
        const hnd_scene h_scene) {

        scene_mngr_validate();
        return(scene_stack_contains(scene_stack_next(), h_scene));
    }

    // whether the scene's entities update this frame
    IFB_INTERNAL bool
    scene_is_updating(
        const hnd_scene h_scene) {

        const bool is_updating = (
            scene_is_valid(h_scene) &&
            _scene_mngr->tbl.is_updating[h_scene.val]
        );
        return(is_updating);
    }

    // whether the scene's entities render this frame
    IFB_INTERNAL bool
    scene_is_rendering(
        const hnd_scene h_scene) {

        const bool is_rendering = (
            scene_is_valid(h_scene) &&
            _scene_mngr->tbl.is_rendering[h_scene.val]
        );
        return(is_rendering);
    }

    //--------------------------------------------------------------------
    // PARALLAX
    //--------------------------------------------------------------------

    // the names are image assets, back to front
    IFB_INTERNAL bool
    scene_set_parallax(
        const hnd_scene h_scene,
        const cchar*    back,
        const cchar*    mid,
        const cchar*    front) {

        assert(back && mid && front);

        if (!scene_is_valid(h_scene)) {
            return(false);
        }

        constexpr u32 name_size = config::asset_name_size;
        const bool    does_fit  = (
            strnlen_s(back,  name_size + 1) <= name_size &&
            strnlen_s(mid,   name_size + 1) <= name_size &&
            strnlen_s(front, name_size + 1) <= name_size
        );
        if (!does_fit) {
            return(false);
        }

        scene_parallax& plx = _scene_mngr->tbl.parallax[h_scene.val];
        scene_parallax_clear(plx);
        (void)strncpy_s(plx.back,  back,  name_size);
        (void)strncpy_s(plx.mid,   mid,   name_size);
        (void)strncpy_s(plx.front, front, name_size);

        // a new revision every time, so the renderer knows to load the layers again
        plx.revision = ++_scene_mngr->parallax_revision;
        return(true);
    }

    IFB_INTERNAL bool
    scene_clear_parallax(
        const hnd_scene h_scene) {

        if (!scene_is_valid(h_scene)) {
            return(false);
        }

        scene_parallax_clear(_scene_mngr->tbl.parallax[h_scene.val]);
        return(true);
    }

    // the parallax of the highest rendering scene that has one, NULL if none of them do
    IFB_INTERNAL const scene_parallax*
    scene_get_parallax_drawn(
        void) {

        scene_mngr_validate();

        const scene_mngr::stack& stack = _scene_mngr->stack_running;
        for (
            u32 stack_index = stack.count;
                stack_index > 0;
              --stack_index) {

            const hnd_scene h_scene = stack.hnd[stack_index - 1];
            if (!_scene_mngr->tbl.is_rendering[h_scene.val]) break;

            const scene_parallax& plx = _scene_mngr->tbl.parallax[h_scene.val];
            if (plx.revision != 0) {
                return(&plx);
            }
        }
        return(NULL);
    }
};
