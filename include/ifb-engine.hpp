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
    // SYSTEM
    //--------------------------------------------------------------------

    IFB_ENGINE_API f32 eng_system_get_delta_time_s (void);

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
    // FILE DIALOG 
    //--------------------------------------------------------------------

    IFB_ENGINE_API const cchar* eng_file_dialog_open      (const cchar* filter_type, const cchar* filter_desc);
    IFB_ENGINE_API const cchar* eng_file_dialog_save      (const cchar* filter_type, const cchar* filter_desc);

    //--------------------------------------------------------------------
    // ASSETS
    //--------------------------------------------------------------------

    // the engine has one dictionary for editing, it starts out empty
    // a file is identified by the hash of its name, so names are unique across every type
    // file paths are relative to the directory of the dictionary, once it has been loaded or saved
    IFB_ENGINE_API bool      eng_asset_dictionary_new            (void);
    IFB_ENGINE_API bool      eng_asset_dictionary_load           (const cchar* path);
    IFB_ENGINE_API bool      eng_asset_dictionary_save           (const cchar* path);
    IFB_ENGINE_API hnd_asset eng_asset_dictionary_add_file       (const u32 type, const cchar* name, const cchar* path);
    IFB_ENGINE_API bool      eng_asset_dictionary_remove_file    (const cchar* name);
    IFB_ENGINE_API u32       eng_asset_dictionary_get_file_count (const u32 type);
    IFB_ENGINE_API bool      eng_asset_dictionary_get_file_info  (const u32 type, const u32 index, asset_dictionary_file_info& info);
    IFB_ENGINE_API bool      eng_asset_dictionary_load_preview   (const u32 id, asset_preview& preview);

    // the engine has one database for viewing, it stays open until another one is loaded or built
    // building reads a dictionary file, so the paths are relative to it, and opens the new database
    // the error says why the last build or load failed
    IFB_ENGINE_API bool         eng_asset_database_build        (const cchar* path_dictionary, const cchar* path_database);
    IFB_ENGINE_API bool         eng_asset_database_load         (const cchar* path);
    IFB_ENGINE_API void         eng_asset_database_close        (void);
    IFB_ENGINE_API const cchar* eng_asset_database_get_error    (void);
    IFB_ENGINE_API u32          eng_asset_database_get_count    (void);
    IFB_ENGINE_API bool         eng_asset_database_get_info     (const u32 index, asset_database_info& info);
    IFB_ENGINE_API bool         eng_asset_database_load_preview (const u32 index, asset_preview& preview);

    //--------------------------------------------------------------------
    // CAMERA
    //--------------------------------------------------------------------

    IFB_ENGINE_API const vec3& eng_camera_get_origin (void);
    IFB_ENGINE_API const vec3& eng_camera_get_target (void);
    IFB_ENGINE_API void        eng_camera_set_origin (const vec3& origin);
    IFB_ENGINE_API void        eng_camera_set_target (const vec3& target);
    IFB_ENGINE_API void        eng_camera_get_xform  (mat4& xform);

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
    IFB_ENGINE_API bool         eng_entity_lookup_by_id          (const entity_id, entity& e);
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
    IFB_ENGINE_API bool         eng_cmpnt_lookup_particle_emitter (const entity_id id, cmpnt_particle_emitter&  emitter);

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
    IFB_ENGINE_API bool         eng_cmpnt_update_particle_emitter (const entity_id id, const cmpnt_particle_emitter& emitter);
  
    //--------------------------------------------------------------------
    // ARCHETYPES 
    //--------------------------------------------------------------------
    
    IFB_ENGINE_API bool eng_atype_lookup_quad         (const entity_id id, atype_quad&         quad);
    IFB_ENGINE_API bool eng_atype_lookup_particle     (const entity_id id, atype_particle&     particle);
    IFB_ENGINE_API bool eng_atype_lookup_physics_quad (const entity_id id, atype_physics_quad& physics_quad);

    IFB_ENGINE_API bool eng_atype_update_quad         (const entity_id id, const atype_quad&         quad);
    IFB_ENGINE_API bool eng_atype_update_particle     (const entity_id id, const atype_particle&     particle);
    IFB_ENGINE_API bool eng_atype_update_physics_quad (const entity_id id, const atype_physics_quad& physics_quad);
  
    //--------------------------------------------------------------------
    // TILE MAPS 
    //--------------------------------------------------------------------
   
    IFB_ENGINE_API hnd_map eng_map_create               (const cchar*  name,  const u32 count_rows, const u32 count_cols); 
    IFB_ENGINE_API u32     eng_map_chunk_create         (const hnd_map h_map, const u32 count_rows, const u32 count_cols, const u32 origin_row, const u32 origin_col, const map_color_u32 base_color);
    IFB_ENGINE_API bool    eng_map_get_entity_coords    (const hnd_map h_map, const entity_id id, cmpnt_map_coords& coords);
    IFB_ENGINE_API void    eng_map_destroy              (const hnd_map h_map);
    IFB_ENGINE_API bool    eng_map_render               (const hnd_map h_map);
    IFB_ENGINE_API bool    eng_map_get_pos_from_coords  (const cmpnt_map_coords& coords, cmpnt_position& pos);
    IFB_ENGINE_API bool    eng_map_get_coords_from_pos  (const cmpnt_position&   pos,    cmpnt_map_coords& coords);
    IFB_ENGINE_API bool    eng_map_set_origin           (const hnd_map h_map, const vec3& origin);
    IFB_ENGINE_API bool    eng_map_get_dimensions       (const hnd_map h_map, u32& count_rows, u32& count_cols);
    IFB_ENGINE_API bool    eng_map_is_in_bounds         (const cmpnt_map_coords& coords);
    IFB_ENGINE_API bool    eng_map_chunk_get_dimensions (const hnd_map h_map, const u32 chunk_index, u32& count_rows, u32& count_cols, u32& origin_row, u32& origin_col);
    IFB_ENGINE_API bool    eng_map_get_navigable_bounds (const hnd_map h_map, u32& row_min, u32& col_min, u32& row_max, u32& col_max);

    //--------------------------------------------------------------------
    // PARTICLES 
    //--------------------------------------------------------------------
  
    IFB_ENGINE_API hnd_particle_emitter
    eng_particle_emitter_create_parabolic(
        const cchar*                    name,
        const u32                       count,
        const f32                       lifespan_ms,
        const f32                       spawn_delay_ms,
        const f32                       spawn_radius,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_parabolic_props& props
    );
    IFB_ENGINE_API hnd_particle_emitter
    eng_particle_emitter_create_radial(
        const cchar*                    name,
        const u32                       count,
        const f32                       lifespan_ms,
        const f32                       spawn_delay_ms,
        const f32                       spawn_radius,
        const f32                       width,
        const f32                       height,
        const color_rgba_u32&           color_primary,
        const color_rgba_u32&           color_secondary,
        const particle_radial_props&    props
    );
    IFB_ENGINE_API bool eng_particle_emitter_destroy (const hnd_particle_emitter h_emitter);
    IFB_ENGINE_API bool eng_particle_emitter_start   (const hnd_particle_emitter h_emitter, const vec3& position);
    IFB_ENGINE_API bool eng_particle_emitter_stop    (const hnd_particle_emitter h_emitter);
    IFB_ENGINE_API bool eng_particle_emitter_render  (const hnd_particle_emitter h_emitter);

    IFB_ENGINE_API u32                  eng_particle_emitter_get_capacity (void);
    IFB_ENGINE_API hnd_particle_emitter eng_particle_emitter_get_hnd      (const u32 index);
    IFB_ENGINE_API bool                 eng_particle_emitter_get_info     (const hnd_particle_emitter h_emitter, particle_emitter_info& info);

    // live editing, an update ignores the name, type, and state in the info
    // nothing changes and false is returned if a value is invalid or the type doesn't match
    IFB_ENGINE_API bool eng_particle_emitter_update              (const hnd_particle_emitter h_emitter, const particle_emitter_info&    info);
    IFB_ENGINE_API bool eng_particle_emitter_get_props_parabolic (const hnd_particle_emitter h_emitter, particle_parabolic_props&       props);
    IFB_ENGINE_API bool eng_particle_emitter_set_props_parabolic (const hnd_particle_emitter h_emitter, const particle_parabolic_props& props);
    IFB_ENGINE_API bool eng_particle_emitter_get_props_radial    (const hnd_particle_emitter h_emitter, particle_radial_props&          props);
    IFB_ENGINE_API bool eng_particle_emitter_set_props_radial    (const hnd_particle_emitter h_emitter, const particle_radial_props&    props);

    // the particle view is an offscreen render of every active emitter
    // a request draws it for one frame, the texture is an opengl texture id
    IFB_ENGINE_API void        eng_particle_view_request           (const u32 width, const u32 height);
    IFB_ENGINE_API u32         eng_particle_view_get_texture       (void);
    IFB_ENGINE_API const vec3& eng_particle_view_get_camera_origin (void);
    IFB_ENGINE_API const vec3& eng_particle_view_get_camera_target (void);
    IFB_ENGINE_API void        eng_particle_view_set_camera_origin (const vec3& origin);
    IFB_ENGINE_API void        eng_particle_view_set_camera_target (const vec3& target);

    //--------------------------------------------------------------------
    // RANDOM
    //--------------------------------------------------------------------

    IFB_ENGINE_API u32 eng_random_u32       (void);
    IFB_ENGINE_API f32 eng_random_f32       (void);
    IFB_ENGINE_API f32 eng_random_f32_range (const f32 min, const f32 max);

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
    IFB_ENGINE_API void*     eng_arena_push      (const hnd_arena arena, const u32 size);

    //--------------------------------------------------------------------
    // TEXT 
    //--------------------------------------------------------------------
  
    IFB_ENGINE_API u32         eng_text_file_size           (const hnd_file h_txt_file);
    IFB_ENGINE_API const text* eng_text_file_load_to_arena  (const hnd_file h_txt_file, const hnd_arena h_arena);
    IFB_ENGINE_API const text* eng_text_file_load_to_memory (const hnd_file h_txt_file, memory& mem);

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
        memory particles;
    };

    struct eng_game_context {
        int x;
    };
}
#endif  //IFB_ENGINE_HPP
