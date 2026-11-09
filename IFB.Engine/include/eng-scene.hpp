#ifndef ENG_SCENE_HPP
#define ENG_SCENE_HPP

#include "ifb.hpp"
#include "ifb-config.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // TYPES
    //--------------------------------------------------------------------

    // the image assets drawn behind the scene, back to front
    struct scene_parallax {
        cchar back  [config::asset_name_size + 1];
        cchar mid   [config::asset_name_size + 1];
        cchar front [config::asset_name_size + 1];
        u32   revision; // 0 when the scene has no parallax, unique every time it is set
    };

    // a scene owns the entities, particle emitters, parallax and memory created while it is bound
    // destroying or resetting a scene releases everything it owns
    // the handle is the scene's slot index
    //
    // the stack decides which scenes run
    // the top scene always updates and renders
    // the scenes under it do too, for as long as every scene above them has the matching flag
    // the default scene is the bottom of the stack, and can't be popped
    //
    // nothing changes in the middle of a frame
    // push, pop, transition, destroy and reset are requests, and binding follows them right away,
    // so entities created after a push belong to the pushed scene
    // what runs, and what gets released, changes at the start of the next frame
    struct scene_mngr {
        struct {
            hnd_scene*      hnd;                // INVALID_HANDLE when the slot is free
            tag*            name;
            hnd_arena*      arena;              // scene lifetime allocations
            scene_parallax* parallax;
            bool*           is_updating;        // rebuilt whenever the running stack changes
            bool*           is_rendering;
            bool*           is_pending_destroy;
            bool*           is_pending_reset;
        } tbl;
        struct stack {
            hnd_scene*       hnd;
            scene_flags_u32* flags;
            u32              count;
        };
        stack     stack_running;      // what runs this frame
        stack     stack_next;         // what runs from the start of the next frame
        u32       capacity;
        u32       count;
        u32       parallax_revision;  // the last revision handed out
        hnd_scene h_bound;            // new entities belong to this scene
        hnd_scene h_default;          // the bottom of the stack
    } static * _scene_mngr;

    //--------------------------------------------------------------------
    // INTERNAL METHODS
    //--------------------------------------------------------------------

    // scene manager
    IFB_INTERNAL scene_mngr*  scene_mngr_create      (void);
    IFB_INTERNAL void         scene_mngr_validate    (void);
    IFB_INTERNAL void         scene_mngr_startup     (void);
    IFB_INTERNAL void         scene_mngr_frame_start (void);
    IFB_INTERNAL u32          scene_mngr_get_count   (void);

    // scene
    IFB_INTERNAL hnd_scene    scene_create           (const cchar* name);
    IFB_INTERNAL bool         scene_destroy          (const hnd_scene h_scene);
    IFB_INTERNAL bool         scene_reset            (const hnd_scene h_scene);
    IFB_INTERNAL bool         scene_is_valid         (const hnd_scene h_scene);
    IFB_INTERNAL void*        scene_alloc            (const hnd_scene h_scene, const u32 size);
    IFB_INTERNAL hnd_arena    scene_get_arena        (const hnd_scene h_scene);
    IFB_INTERNAL const cchar* scene_get_name         (const hnd_scene h_scene);

    // binding
    IFB_INTERNAL bool         scene_bind             (const hnd_scene h_scene);
    IFB_INTERNAL hnd_scene    scene_get_bound        (void);
    IFB_INTERNAL hnd_scene    scene_get_default      (void);

    // stack
    IFB_INTERNAL bool         scene_push             (const hnd_scene h_scene, const scene_flags_u32 flags);
    IFB_INTERNAL bool         scene_pop              (void);
    IFB_INTERNAL bool         scene_transition       (const hnd_scene h_scene, const scene_flags_u32 flags);
    IFB_INTERNAL hnd_scene    scene_get_top          (void);
    IFB_INTERNAL u32          scene_get_stack_count  (void);
    IFB_INTERNAL bool         scene_is_on_stack      (const hnd_scene h_scene);
    IFB_INTERNAL bool         scene_is_updating      (const hnd_scene h_scene);
    IFB_INTERNAL bool         scene_is_rendering     (const hnd_scene h_scene);

    // parallax
    IFB_INTERNAL bool                  scene_set_parallax      (const hnd_scene h_scene, const cchar* back, const cchar* mid, const cchar* front);
    IFB_INTERNAL bool                  scene_clear_parallax    (const hnd_scene h_scene);
    IFB_INTERNAL const scene_parallax* scene_get_parallax_drawn(void);
};

#endif //ENG_SCENE_HPP
