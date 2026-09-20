#ifndef IFB_ENGINE_HPP
#define IFB_ENGINE_HPP

#include "ifb-entity.hpp"
#include "sld.hpp"
#include "ifb-types.hpp"
#include "ifb-platform.hpp"
#include "ifb-input.hpp"
#include "ifb-collections.hpp"
#include <imgui.h>
#ifdef IFB_ENGINE_STATIC
#   define IFB_ENGINE_API
#elif defined(IFB_ENGINE_DLL_EXPORT)
#   define IFB_ENGINE_API __declspec(dllexport)
#else
#   define IFB_ENGINE_API __declspec(dllimport)
#endif

using namespace sld;

#define ifb_eng_arena_push_struct(a, type, count) (type*)eng_arena_push(a, sizeof(type) * count)


namespace ifb {
    
    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------
    
    struct eng_system_info;
    struct eng_context;
    struct eng_mem;
    struct eng_mem_map;
    struct eng_game_context;

    //--------------------------------------------------------------------
    // FUNCTION POINTERS 
    //--------------------------------------------------------------------

  typedef bool (*fptr_eng_game)   (eng_game_context* game_ctx); 
  typedef void (*fptr_eng_render) (void); 

    //--------------------------------------------------------------------
    // CONTEXT
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context*
    eng_context_create(
        const eng_mem_map* mem_map,
        fptr_eng_game      game_callback,
        fptr_eng_render    render_callback
    );

    IFB_ENGINE_API void          eng_context_startup   (void);
    IFB_ENGINE_API bool          eng_context_run       (void);
    IFB_ENGINE_API void          eng_context_shutdown  (void);
    IFB_ENGINE_API void          eng_context_destroy   (eng_context* ctx);
    IFB_ENGINE_API ImGuiContext* eng_context_get_imgui (void);

    //--------------------------------------------------------------------
    // WINDOW
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_window_open      (void);
    IFB_ENGINE_API void eng_window_did_close (void);
    IFB_ENGINE_API void eng_window_set_pos   (const u32 pos_x, const u32 window_pos_y);
    IFB_ENGINE_API void eng_window_set_size  (const u32 width, const u32 height);
    
    //--------------------------------------------------------------------
    // GUI
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_gui_open  (void);
    IFB_ENGINE_API void eng_gui_close (void);

    //--------------------------------------------------------------------
    // INPUT
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_input_set_key_down           (const input_keycode key_ptr);
    IFB_ENGINE_API void eng_input_set_key_up             (const input_keycode key_ptr);
    IFB_ENGINE_API bool eng_input_is_key_down            (const input_keycode keycode);
    IFB_ENGINE_API void eng_input_mouse_click_left_down  (void);
    IFB_ENGINE_API void eng_input_mouse_click_left_up    (void);
    IFB_ENGINE_API void eng_input_mouse_click_right_down (void);
    IFB_ENGINE_API void eng_input_mouse_click_right_up   (void);
    IFB_ENGINE_API void eng_input_mouse_move             (const u32 x, const u32 y);

    //--------------------------------------------------------------------
    // FILES
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API hnd_file     eng_file_ro_create_new    (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_ro_open_existing (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_ro_open_always   (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_ro_overwrite     (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_wo_create_new    (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_wo_open_existing (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_wo_open_always   (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_wo_overwrite     (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_rw_create_new    (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_rw_open_existing (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_rw_open_always   (const cchar* path);
    IFB_ENGINE_API hnd_file     eng_file_rw_overwrite     (const cchar* path);
    IFB_ENGINE_API void         eng_file_close            (const hnd_file hnd);
    IFB_ENGINE_API u32          eng_file_get_size         (const hnd_file hnd);
    IFB_ENGINE_API void         eng_file_set_cursor       (const hnd_file hnd, const u32 cursor);
    IFB_ENGINE_API const cchar* eng_file_read             (const hnd_file hnd, const u32 buffer_size);
    IFB_ENGINE_API u32          eng_file_write            (const hnd_file hnd, const u32 buffer_size, const byte* buffer_ptr);

    //--------------------------------------------------------------------
    // CAMERA
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_camera_get_origin (vec3& origin);
    IFB_ENGINE_API void eng_camera_get_target (vec3& target);
    IFB_ENGINE_API void eng_camera_set_origin (const vec3& origin);
    IFB_ENGINE_API void eng_camera_set_target (const vec3& target);
    IFB_ENGINE_API void eng_camera_get_xform  (mat4& xform);

    //--------------------------------------------------------------------
    // ENTITIES
    //--------------------------------------------------------------------

    IFB_ENGINE_API entity_id    eng_entity_create                (const cchar*    tag);
    IFB_ENGINE_API bool         eng_entity_destroy_by_id         (const entity_id id);
    IFB_ENGINE_API bool         eng_entity_destroy_by_tag        (const cchar*    tag);
    IFB_ENGINE_API u32          eng_entity_get_count             (void);
    IFB_ENGINE_API const cchar* eng_entity_get_tag               (const entity_id id);
    IFB_ENGINE_API bool         eng_entity_add_components        (const entity_id id, const component_type    type);
    IFB_ENGINE_API bool         eng_entity_remove_components     (const entity_id id, const component_type    type);
    IFB_ENGINE_API bool         eng_entity_lookup_by_dense_index (const u32 dense_index, entity& e);
    IFB_ENGINE_API bool         eng_entity_query                 (entity_list* list, const entity_query query);
    IFB_ENGINE_API bool         eng_entity_quad_lookup           (quad_entity& q, const entity_id id);
    IFB_ENGINE_API bool         eng_entity_render                (const entity_id id);
    IFB_ENGINE_API bool         eng_entity_add_force             (const entity_id id, const vec3& force);

    //--------------------------------------------------------------------
    // COMPONENTS 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool         eng_cmpnt_lookup_position       (const entity_id id, cmpnt_position&      pos);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_velocity       (const entity_id id, cmpnt_velocity&      vel);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_acceleration   (const entity_id id, cmpnt_acceleration&  acc);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_term_velocity  (const entity_id id, cmpnt_term_velocity& tv);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_inv_mass       (const entity_id id, f32&                 inv_mass);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_mass           (const entity_id id, f32&                 mass);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_drag           (const entity_id id, f32&                 drag);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_quad           (const entity_id id, cmpnt_quad&          q);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_color          (const entity_id id, cmpnt_color&         color);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_spring         (const entity_id id, cmpnt_spring&        spr);
    IFB_ENGINE_API bool         eng_cmpnt_lookup_map_coords     (const entity_id id, cmpnt_map_coords&          coords);

    IFB_ENGINE_API bool         eng_cmpnt_update_position       (const entity_id id, const cmpnt_position&      pos);
    IFB_ENGINE_API bool         eng_cmpnt_update_velocity       (const entity_id id, const cmpnt_velocity&      vel);
    IFB_ENGINE_API bool         eng_cmpnt_update_acceleration   (const entity_id id, const cmpnt_acceleration&  acc);
    IFB_ENGINE_API bool         eng_cmpnt_update_term_velocity  (const entity_id id, const cmpnt_term_velocity& tv);
    IFB_ENGINE_API bool         eng_cmpnt_update_inv_mass       (const entity_id id, const f32                  inv_mass);
    IFB_ENGINE_API bool         eng_cmpnt_update_mass           (const entity_id id, const f32                  mass);
    IFB_ENGINE_API bool         eng_cmpnt_update_drag           (const entity_id id, const f32                  drag);
    IFB_ENGINE_API bool         eng_cmpnt_update_quad           (const entity_id id, const atype_quad&      quad);
    IFB_ENGINE_API bool         eng_cmpnt_update_color          (const entity_id id, const cmpnt_color&         color);
    IFB_ENGINE_API bool         eng_cmpnt_update_spring         (const entity_id id, const cmpnt_spring&        spr);
    IFB_ENGINE_API bool         eng_cmpnt_update_map_coords     (const entity_id id, const cmpnt_map_coords&    coords);

    //--------------------------------------------------------------------
    // TILE MAPS 
    //--------------------------------------------------------------------
   
    IFB_ENGINE_API hnd_map
    eng_map_create(
        const cchar* name,
        const u32    count_rows,
        const u32    count_cols
    ); 
    IFB_ENGINE_API u32 
    eng_map_chunk_create(
        const hnd_map       map_hnd,
        const u32           count_rows,
        const u32           count_cols,
        const u32           origin_row,
        const u32           origin_col,
        const map_color_u32 base_color
    );
    IFB_ENGINE_API bool
    eng_map_get_entity_coords(
        const hnd_map  map,
        const entity_id   eid,
        cmpnt_map_coords& coords
    );
    IFB_ENGINE_API void eng_map_destroy (const hnd_map map_hnd);
    IFB_ENGINE_API bool eng_map_render  (const hnd_map map_hnd);

    //--------------------------------------------------------------------
    // ARENAS
    //--------------------------------------------------------------------

    IFB_ENGINE_API hnd_arena eng_arena_alloc     (void);
    IFB_ENGINE_API void      eng_arena_free      (const hnd_arena arena);
    IFB_ENGINE_API void      eng_arena_reset     (const hnd_arena arena);
    IFB_ENGINE_API u32       eng_arena_save      (const hnd_arena arena);
    IFB_ENGINE_API u32       eng_arena_size_free (const hnd_arena arena);
    IFB_ENGINE_API u32       eng_arena_size_used (const hnd_arena arena);
    IFB_ENGINE_API void      eng_arena_revert    (const hnd_arena arena, const u32 save);
    IFB_ENGINE_API void      eng_arena_commit    (const hnd_arena arena, const u32 save);
    IFB_ENGINE_API void*     eng_arena_push      (const hnd_arena arena, const u32 size);

    //--------------------------------------------------------------------
    // IMAGES
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32          eng_image_size           (const hnd_file img_file_hnd);
    IFB_ENGINE_API const image* eng_image_load_to_arena  (const hnd_file img_file_hnd, const hnd_arena arena_hnd);
    IFB_ENGINE_API const image* eng_image_load_to_memory (const hnd_file img_file_hnd, const memory& mem);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct eng_system_info {
        struct {
            u32              count;
            pfm_monitor_info primary;
            pfm_monitor_area working_area;
        } monitor;
        struct {
            u64 timestamp_curr;
            u64 timestamp_prev;
            u64 frequency;
        } time;
        pfm_memory_info mem_info;
    };

    struct eng_mem_map {
        memory stack;
        memory files;
        memory core;
        memory rendering;
        memory entities;
        memory gui;
        memory arenas;
        memory components;
        memory quads;
        memory physics;
        memory tiles;
    };

    struct eng_game_context {
        int x;
    };
}
#endif  //IFB_ENGINE_HPP
