#ifndef RENDERER_INTERNAL_HPP
#define RENDERER_INTERNAL_HPP

#include "ifb-types.hpp"
#include "ifb-renderer.hpp"
#include "sld-opengl.hpp"

namespace ifb {

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
