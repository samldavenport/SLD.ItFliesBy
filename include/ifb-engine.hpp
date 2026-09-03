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

  typedef bool (*eng_game_proc)(eng_game_context* game_ctx); 

    //--------------------------------------------------------------------
    // CONTEXT
    //--------------------------------------------------------------------

    IFB_ENGINE_API eng_context*  eng_context_create                       (const eng_mem_map* mem_map, eng_game_proc game_callback);
    IFB_ENGINE_API void          eng_context_startup                      (void);
    IFB_ENGINE_API bool          eng_context_run                          (void);
    IFB_ENGINE_API void          eng_context_shutdown                     (void);
    IFB_ENGINE_API void          eng_context_destroy                      (eng_context* ctx);
    IFB_ENGINE_API ImGuiContext* eng_context_get_imgui                    (void);

    //--------------------------------------------------------------------
    // WINDOW
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_window_open                                  (void);
    IFB_ENGINE_API void eng_window_did_close                             (void);
    IFB_ENGINE_API void eng_window_set_pos                               (const u32 pos_x, const u32 window_pos_y);
    IFB_ENGINE_API void eng_window_set_size                              (const u32 width, const u32 height);
    
    //--------------------------------------------------------------------
    // GUI
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_gui_open  (void);
    IFB_ENGINE_API void eng_gui_close (void);

    //--------------------------------------------------------------------
    // INPUT
    //--------------------------------------------------------------------

    IFB_ENGINE_API void eng_input_set_key_down                           (const input_keycode key_ptr);
    IFB_ENGINE_API void eng_input_set_key_up                             (const input_keycode key_ptr);
    IFB_ENGINE_API bool eng_input_is_key_down                            (const input_keycode keycode);
    IFB_ENGINE_API void eng_input_mouse_click_left_down                  (void);
    IFB_ENGINE_API void eng_input_mouse_click_left_up                    (void);
    IFB_ENGINE_API void eng_input_mouse_click_right_down                 (void);
    IFB_ENGINE_API void eng_input_mouse_click_right_up                   (void);
    IFB_ENGINE_API void eng_input_mouse_move                             (const u32 x, const u32 y);

    //--------------------------------------------------------------------
    // FILES
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API file_handle eng_file_ro_create_new                (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_ro_open_existing             (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_ro_open_always               (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_ro_overwrite                 (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_wo_create_new                (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_wo_open_existing             (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_wo_open_always               (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_wo_overwrite                 (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_rw_create_new                (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_rw_open_existing             (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_rw_open_always               (const cchar* path);
    IFB_ENGINE_API file_handle eng_file_rw_overwrite                 (const cchar* path);
    IFB_ENGINE_API void            eng_file_close                        (const file_handle hnd);
    IFB_ENGINE_API u32             eng_file_get_size                     (const file_handle hnd);
    IFB_ENGINE_API void            eng_file_set_cursor                   (const file_handle hnd, const u32 cursor);
    IFB_ENGINE_API const cchar*    eng_file_read                         (const file_handle hnd, const u32 buffer_size);
    IFB_ENGINE_API u32             eng_file_write                        (const file_handle hnd, const u32 buffer_size, const byte* buffer_ptr);

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
    IFB_ENGINE_API const bool   eng_entity_add_components        (const entity_id id, const component_type    type);
    IFB_ENGINE_API const bool   eng_entity_remove_components     (const entity_id id, const component_type    type);
    IFB_ENGINE_API bool         eng_entity_lookup_by_dense_index (const u32 dense_index, entity& e);
    IFB_ENGINE_API bool         eng_entity_query                 (entity_list* list, const entity_query query);
    IFB_ENGINE_API bool         eng_entity_quad_lookup           (quad_entity& q, const entity_id id);

    IFB_ENGINE_API bool         eng_entity_lookup_position       (const entity_id id, position_3d&      pos);
    IFB_ENGINE_API bool         eng_entity_lookup_velocity       (const entity_id id, velocity_3d&      vel);
    IFB_ENGINE_API bool         eng_entity_lookup_acceleration   (const entity_id id, acceleration_3d&  acc);
    IFB_ENGINE_API bool         eng_entity_lookup_term_velocity  (const entity_id id, term_velocity_3d& tv);
    IFB_ENGINE_API bool         eng_entity_lookup_inv_mass       (const entity_id id, f32&               inv_mass);
    IFB_ENGINE_API bool         eng_entity_lookup_mass           (const entity_id id, f32&               mass);
    IFB_ENGINE_API bool         eng_entity_lookup_drag           (const entity_id id, f32&               drag);
    IFB_ENGINE_API bool         eng_entity_lookup_quad           (const entity_id id, quad&  q);
    IFB_ENGINE_API bool         eng_entity_lookup_color          (const entity_id id, color_rgba_u32&   color);
    IFB_ENGINE_API bool         eng_entity_lookup_spring         (const entity_id id, spring&           spr);

    IFB_ENGINE_API const bool   eng_entity_update_position       (const entity_id id, const position_3d&      pos);
    IFB_ENGINE_API const bool   eng_entity_update_velocity       (const entity_id id, const velocity_3d&      vel);
    IFB_ENGINE_API const bool   eng_entity_update_acceleration   (const entity_id id, const acceleration_3d&  acc);
    IFB_ENGINE_API const bool   eng_entity_update_term_velocity  (const entity_id id, const term_velocity_3d& tv);
    IFB_ENGINE_API const bool   eng_entity_update_inv_mass       (const entity_id id, const f32               inv_mass);
    IFB_ENGINE_API const bool   eng_entity_update_mass           (const entity_id id, const f32               mass);
    IFB_ENGINE_API const bool   eng_entity_update_drag           (const entity_id id, const f32               drag);
    IFB_ENGINE_API const bool   eng_entity_update_quad           (const entity_id id, const quad_archetype&   quad);
    IFB_ENGINE_API const bool   eng_entity_update_color          (const entity_id id, const color_rgba_u32&   color);
    IFB_ENGINE_API const bool   eng_entity_update_spring         (const entity_id id, const spring&           spr);
    IFB_ENGINE_API const bool   eng_entity_render                (const entity_id id);
    IFB_ENGINE_API const bool   eng_entity_add_force             (const entity_id id, const vec3& force);

    //--------------------------------------------------------------------
    // TILE MAPS 
    //--------------------------------------------------------------------
   
    IFB_ENGINE_API map_handle
    eng_map_create(
        const cchar*         name,
        const u32            count_rows,
        const u32            count_cols,
        const s32            offset_row,
        const s32            offset_col,
        const color_rgba_u32 base_color); 

    IFB_ENGINE_API void           eng_map_destroy           (const map_handle map);
    IFB_ENGINE_API void           eng_map_render            (const map_handle map);
    IFB_ENGINE_API void           eng_map_set_colors        (const map_handle map, const u32* row, const u32* col, const color_rgba_u32* color, const u32 count); 
    IFB_ENGINE_API void           eng_map_set_flags         (const map_handle map, const u32* row, const u32* col, const tile_flags_u32* flags, const u32 count); 
    IFB_ENGINE_API bool           eng_map_get_entity_coords (const map_handle map, const entity_id eid, map_coords& coords);

    //--------------------------------------------------------------------
    // IMAGES
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32  image_size (const cchar* path);
    IFB_ENGINE_API bool image_load (const cchar* path);
    
    //--------------------------------------------------------------------
    // ARENAS
    //--------------------------------------------------------------------

    IFB_ENGINE_API arena_handle     eng_arena_alloc     (void);
    IFB_ENGINE_API void             eng_arena_free      (const arena_handle arena);
    IFB_ENGINE_API void             eng_arena_reset     (const arena_handle arena);
    IFB_ENGINE_API u32              eng_arena_save      (const arena_handle arena);
    IFB_ENGINE_API u32              eng_arena_size_free (const arena_handle arena);
    IFB_ENGINE_API u32              eng_arena_size_used (const arena_handle arena);
    IFB_ENGINE_API void             eng_arena_revert    (const arena_handle arena, const u32 save);
    IFB_ENGINE_API void             eng_arena_commit    (const arena_handle arena, const u32 save);
    IFB_ENGINE_API void*            eng_arena_push      (const arena_handle arena, const u32 size);

    //--------------------------------------------------------------------
    // IMAGES
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32          eng_image_size           (const file_handle img_file_hnd);
    IFB_ENGINE_API const image* eng_image_load_to_arena  (const file_handle img_file_hnd, const arena_handle arena_hnd);
    IFB_ENGINE_API const image* eng_image_load_to_memory (const file_handle img_file_hnd, const memory& mem);

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
    };

    struct eng_mem {
        void* ptr;
        u64   size;
    };

    struct eng_mem_map {
        eng_mem stack;
        eng_mem files;
        eng_mem core;
        eng_mem rendering;
        eng_mem entities;
        eng_mem gui;
        eng_mem arenas;
        eng_mem components;
        eng_mem quads;
        eng_mem physics;
        eng_mem tiles;
    };

    struct eng_game_context {
        int x;
    };

}
#endif  //IFB_ENGINE_HPP
