#ifndef RENDERER_INTERNAL_HPP
#define RENDERER_INTERNAL_HPP

#include "sld-opengl.hpp"
#include "ifb.hpp"
#include "quad-internal.hpp"
#include "memory-internal.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 QUAD_VERTEX_COUNT            = 4;
    static constexpr u32 QUAD_ELEMENT_COUNT           = 6;
    static constexpr u32 QUAD_ELEMENT_DATA_SIZE       = sizeof(u32) * QUAD_ELEMENT_COUNT;
    static constexpr u32 RENDERER_ASSET_PREVIEW_COUNT = 4; // one texture for each window that previews assets

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct renderer;
    struct renderer_shader_source;
    struct renderer_tile_vertex;
    struct renderer_tile_shader;
    struct renderer_particle_shader;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct renderer_shader_source {
        const cchar* data;
        u32          size;
    };

    //--------------------------------------------------------------------
    // GLOBALS
    //--------------------------------------------------------------------

    static renderer* _renderer;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL renderer*   renderer_create                           (void);
    IFB_INTERNAL void        renderer_startup                          (reservation* res);
    IFB_INTERNAL void        renderer_shutdown                         (void);
    IFB_INTERNAL void        renderer_update_view_matrix               (void);
    IFB_INTERNAL void        renderer_draw_buffers                     (void);
    IFB_INTERNAL void        renderer_view_projection_xform            (mat4& xform);
    IFB_INTERNAL bool        renderer_push_quad                        (const entity_id id);
    IFB_INTERNAL bool        renderer_push_particle_emitter            (const entity_id id);
    IFB_INTERNAL bool        renderer_upload_sprite_atlas              (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL bool        renderer_release_sprite_atlas             (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL bool        renderer_set_active_sprite_atlas          (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL bool        renderer_set_map                          (const hnd_map h_map);
    IFB_INTERNAL bool        renderer_is_grid_visible                  (void);
    IFB_INTERNAL void        renderer_set_grid_visible                 (const bool is_visible);
    IFB_INTERNAL const vec3& renderer_get_camera_origin                (void);
    IFB_INTERNAL const vec3& renderer_get_camera_target                (void);
    IFB_INTERNAL void        renderer_get_camera_orientation           (orientation& o);
    IFB_INTERNAL void        renderer_get_camera_xform                 (mat4& xform);
    IFB_INTERNAL void        renderer_set_camera_origin                (const vec3& origin);
    IFB_INTERNAL void        renderer_set_camera_target                (const vec3& target);
    IFB_INTERNAL void        renderer_set_viewport_dimensions          (const u32 width, const u32 height);
    IFB_INTERNAL f32         renderer_get_viewport_aspect_ratio        (void);
    IFB_INTERNAL void        renderer_get_viewport_xform               (mat4& xform);
    IFB_INTERNAL f32         renderer_get_viewport_fov                 (void);
    IFB_INTERNAL void        renderer_set_viewport_fov                 (const f32 fov_y);
    IFB_INTERNAL void        renderer_request_particle_view            (const u32 width, const u32 height);
    IFB_INTERNAL u32         renderer_get_particle_view_texture        (void);
    IFB_INTERNAL const vec3& renderer_get_particle_view_camera_origin  (void);
    IFB_INTERNAL const vec3& renderer_get_particle_view_camera_target  (void);
    IFB_INTERNAL void        renderer_set_particle_view_camera_origin  (const vec3& origin);
    IFB_INTERNAL void        renderer_set_particle_view_camera_target  (const vec3& target);
    IFB_INTERNAL u32         renderer_set_asset_preview_texture        (const u32 slot, const u32 width, const u32 height, const byte* pixels);

    struct render_buffers;
    struct render_vertex_buffer;
    struct render_element_buffer;
    struct render_lists;
    struct render_gl;
    struct renderer_viewport;
    struct renderer_camera;
    struct renderer_quad_shader;
    struct renderer_direction_gizmo_shader;
    struct renderer_grid_shader;
    struct renderer_particle_view;
    struct render_sprite_atlas_texture;

    // context
    IFB_INTERNAL void*                        renderer_memory_alloc              (const u32 size);
    IFB_INTERNAL entity_list*                 renderer_create_entity_list        (void);
    IFB_INTERNAL render_sprite_atlas_texture* renderer_find_sprite_atlas_texture (const hnd_sprite_atlas h_atlas);
    IFB_INTERNAL void                         renderer_use_sprite_atlas_texture  (void);

    // camera
    IFB_INTERNAL renderer_camera*      renderer_camera_create              (void);
    IFB_INTERNAL void                  renderer_camera_init                (renderer_camera* camera);
    IFB_INTERNAL const vec3&           renderer_camera_get_origin          (renderer_camera* camera);
    IFB_INTERNAL const vec3&           renderer_camera_get_target          (renderer_camera* camera);
    IFB_INTERNAL void                  renderer_camera_set_origin          (renderer_camera* camera, const vec3& origin);
    IFB_INTERNAL void                  renderer_camera_set_target          (renderer_camera* camera, const vec3& target);
    IFB_INTERNAL void                  renderer_camera_get_orientation     (renderer_camera* camera, orientation& ori);
    IFB_INTERNAL void                  renderer_camera_get_xform           (renderer_camera* camera, mat4& xform);

    // viewport
    IFB_INTERNAL renderer_viewport*    renderer_viewport_create            (void);
    IFB_INTERNAL void                  renderer_viewport_init              (renderer_viewport* viewport);
    IFB_INTERNAL void                  renderer_viewport_set_dimensions    (renderer_viewport* viewport, gl_context* gl, const u32 width, const u32 height);
    IFB_INTERNAL f32                   renderer_viewport_get_aspect_ratio  (renderer_viewport* viewport);
    IFB_INTERNAL void                  renderer_viewport_get_xform         (renderer_viewport* viewport, mat4& xform);
   
    // buffers
    IFB_INTERNAL render_buffers*       render_buffers_create               (void);
    IFB_INTERNAL void                  render_buffers_alloc_and_init       (render_buffers* buffers);
    IFB_INTERNAL void                  render_buffers_render_quads         (render_buffers* buffers, render_lists* lists, renderer_camera* camera);
    IFB_INTERNAL void                  render_buffers_render_tiles         (render_buffers* buffers, render_lists* lists);

    // lists
    IFB_INTERNAL render_lists*         render_lists_create                 (void);
    IFB_INTERNAL void                  render_lists_init                   (render_lists* lists);
    IFB_INTERNAL void                  render_lists_reset                  (render_lists* lists);
    IFB_INTERNAL bool                  render_lists_push_quad              (render_lists* lists, const entity_id id);
    IFB_INTERNAL bool                  render_lists_push_particle_emitter  (render_lists* lists, const entity_id id);
    IFB_INTERNAL bool                  render_lists_set_map                (render_lists* lists, const hnd_map h_map);
    IFB_INTERNAL hnd_map               render_lists_get_map                (render_lists* lists);
    IFB_INTERNAL const entity_list*    render_lists_get_quads              (render_lists* lists);
    IFB_INTERNAL const entity_list*    render_lists_get_particle_emitters  (render_lists* lists);

    // quad
    IFB_INTERNAL renderer_quad_shader* renderer_quad_shader_create         (void);
    IFB_INTERNAL void                  renderer_quad_shader_init           (renderer_quad_shader* shdr, gl_context* gl, render_buffers* buffers, const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void                  renderer_quad_draw                  (renderer_quad_shader* shdr, gl_context* gl, render_buffers* buffers, const mat4& view_proj_xform);

    // grid
    IFB_INTERNAL renderer_grid_shader* renderer_grid_shader_create         (void);
    IFB_INTERNAL void                  renderer_grid_shader_init           (renderer_grid_shader* shdr, gl_context* gl, render_buffers* buffers, const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void                  renderer_grid_draw                  (renderer_grid_shader* shdr, gl_context* gl, render_buffers* buffers, const mat4& view_proj_xform);

    IFB_INTERNAL renderer_direction_gizmo_shader* renderer_direciton_gizmo_shader_create (void);
    IFB_INTERNAL void                             renderer_direciton_gizmo_shader_init   (renderer_direction_gizmo_shader* shdr, gl_context* gl, const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void                             renderer_direction_gizmo_draw          (renderer_direction_gizmo_shader* shdr, gl_context* gl);
    
    // particle view
    IFB_INTERNAL renderer_particle_view* renderer_particle_view_create  (void);
    IFB_INTERNAL void                    renderer_particle_view_init    (renderer_particle_view* view, gl_context* gl, render_buffers* buffers, renderer_quad_shader* shdr_quad);
    IFB_INTERNAL void                    renderer_particle_view_request (renderer_particle_view* view, const u32 width, const u32 height);
    IFB_INTERNAL void                    renderer_particle_view_draw    (renderer_particle_view* view, gl_context* gl, render_buffers* buffers, renderer_quad_shader* shdr_quad, renderer_grid_shader* shdr_grid);

    // tile
    IFB_INTERNAL renderer_tile_shader* renderer_tile_shader_create (void);
    IFB_INTERNAL void                  renderer_tile_shader_init   (renderer_tile_shader* shdr, gl_context* gl, render_buffers* buffers, const renderer_shader_source& src_vertex, const renderer_shader_source& src_fragment);
    IFB_INTERNAL void                  renderer_tile_draw          (renderer_tile_shader* shdr, gl_context* gl, render_buffers* buffers, const hnd_map map_hnd, const mat4& view_proj_xform);
};

#endif //RENDERER_INTERNAL_HPP
