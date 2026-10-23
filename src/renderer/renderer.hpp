#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <sld-opengl.hpp>
#include <files.hpp>

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "quad.hpp"
#include "map.hpp"
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
    IFB_INTERNAL void        renderer_request_particle_view            (const u32 width, const u32 height);
    IFB_INTERNAL u32         renderer_get_particle_view_texture        (void);
    IFB_INTERNAL const vec3& renderer_get_particle_view_camera_origin  (void);
    IFB_INTERNAL const vec3& renderer_get_particle_view_camera_target  (void);
    IFB_INTERNAL void        renderer_set_particle_view_camera_origin  (const vec3& origin);
    IFB_INTERNAL void        renderer_set_particle_view_camera_target  (const vec3& target);
    IFB_INTERNAL u32         renderer_set_asset_preview_texture        (const u32 slot, const u32 width, const u32 height, const byte* pixels);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------
    
    struct renderer_shader_source {
        const cchar* data;
        u32          size;
    };

};

#endif //RENDERER_HPP
