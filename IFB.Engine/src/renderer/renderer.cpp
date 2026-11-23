#pragma once

#include "ifb.hpp"
#include "memory-arena.cpp"
#include "memory-reservation.cpp"
#include "eng-renderer.hpp"
#include "render-buffers.cpp"
#include "renderer-quad.cpp"
#include "renderer-immediate.cpp"
#include "renderer-camera.cpp"
#include "renderer-viewport.cpp"
#include "renderer-direction-gizmo.cpp"
#include "renderer-grid.cpp"
#include "renderer-tile.cpp"
#include "renderer-particle.cpp"
#include "renderer-map-view.cpp"
#include "renderer-parallax.cpp"
#include "eng-scene.hpp"
#include "eng-context.hpp"
#include "sld-math-mat4.hpp"
#include "sld-math.hpp"
#include "sld-opengl.hpp"
#include "eng-sprite.hpp"
#include "ifb-engine.hpp"
#include "ifb-platform.hpp"
#include "eng-file.hpp"
#include "eng-memory.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // DEFINITIONS 
    //--------------------------------------------------------------------

    struct render_sprite_atlas_texture {
        hnd_sprite_atlas h_atlas;
        gl_texture       texture_id;
    };

    // fonts are blobs the game owns, so they're found by their address
    struct render_font_texture {
        const font* fnt;
        gl_texture  texture_id;
    };

    struct render_textures {
        render_sprite_atlas_texture sprite_atlas[config::sprite_atlas_table_capacity];
        gl_texture                  sprite_atlas_active;
        hnd_sprite_atlas            sprite_atlas_active_hnd;
        render_font_texture         fonts[RENDERER_FONT_CAPACITY];
        gl_texture                  font_active;
        const font*                 font_active_fnt;
    };

    struct renderer_memory {
        stack* stack;
    };

    // used is what the last drawn frame put in a buffer, peak is the most it has held
    struct renderer_buffer_stat {
        u32 size_used;
        u32 size_peak;
    };

    // recorded while the frame is drawn, the map view and next frame's reset would overwrite the buffers
    struct renderer_stats {
        u32                  frame_count;
        renderer_buffer_stat quads;
        renderer_buffer_stat quads_entities;
        renderer_buffer_stat quads_particles;
        renderer_buffer_stat tiles;
        renderer_buffer_stat immediate_world;
        renderer_buffer_stat immediate_screen;
        renderer_buffer_stat particle_view;
    };

    struct renderer {
        gl_context*                  gl;
        renderer_memory              memory;
        renderer_camera*             cam;            // the game's camera
        renderer_camera*             cam_free;       // drawn instead of cam while is_cam_free, for tools
        bool                         is_cam_free;
        vec3                         cam_origin_prev; // where the game's camera was when the last simulation step started
        vec3                         cam_target_prev;
        bool                         is_cam_prev_valid;
        f32                          game_fov_y;     // the game's fov while the free camera has the viewport's
        renderer_viewport*           viewport;
        vec3                         global_up;
        bool                         is_grid_visible;
        hnd_map                      map;            // stays set until it's changed
        render_buffers*              buffers;
        renderer_particle_view*      particle_view;
        renderer_map_view*           map_view;
        render_textures*             textures;
        renderer_immediate*          immediate;
        struct {
            renderer_quad_shader*            quad;
            renderer_direction_gizmo_shader* direction_gizmo;
            renderer_grid_shader*            grid; 
            renderer_tile_shader*            tile;
            renderer_parallax_shader*        parallax;
        } shader;
        u32 parallax_revision; // the scene parallax the shader has loaded, 0 for none
        renderer_stats stats;
    };

    // the camera the window is drawn with
    inline renderer_camera*
    renderer_view_camera(
        void) {

        return(_renderer->is_cam_free ? _renderer->cam_free : _renderer->cam);
    }

    inline void
    renderer_stats_record(
        renderer_buffer_stat& stat,
        const u32             size) {

        stat.size_used = size;
        if (size > stat.size_peak) {
            stat.size_peak = size;
        }
    }
    
    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------
    
    IFB_INTERNAL renderer*
    renderer_create(
        void) {

        const auto& cfg         = config_instance();
        const u32   block_count = (cfg.memory_size_rendering / cfg.renderer_mem_granularity);

        auto rndr        = global_alloc<renderer> ();
        auto gl          = global_alloc<gl_context>       ();
        auto block_ids   = global_alloc<u32>              (block_count); 
        auto camera      = renderer_camera_create();
        auto camera_free = renderer_camera_create();
        auto viewport    = renderer_viewport_create();
        auto buffers     = render_buffers_create();
        auto textures    = global_alloc<render_textures>();
        assert(
            rndr       != NULL &&
            gl         != NULL &&
            block_ids  != NULL &&
            camera     != NULL &&
            camera_free != NULL &&
            viewport   != NULL &&
            buffers    != NULL &&
            textures   != NULL
        );

        for (
              u32 i_atlas = 0;
                  i_atlas < config::sprite_atlas_table_capacity;
                ++i_atlas) {

            textures->sprite_atlas[i_atlas].h_atlas    = INVALID_HANDLE;
            textures->sprite_atlas[i_atlas].texture_id = GL_ID_INVALID;
        }
        textures->sprite_atlas_active     = GL_ID_INVALID;
        textures->sprite_atlas_active_hnd = INVALID_HANDLE;

        for (
              u32 i_font = 0;
                  i_font < RENDERER_FONT_CAPACITY;
                ++i_font) {

            textures->fonts[i_font].fnt        = NULL;
            textures->fonts[i_font].texture_id = GL_ID_INVALID;
        }
        textures->font_active     = GL_ID_INVALID;
        textures->font_active_fnt = NULL;

        _renderer           = rndr;
        _renderer->gl       = gl;
        _renderer->cam      = camera;
        _renderer->cam_free = camera_free;
        _renderer->viewport = viewport;
        _renderer->map      = INVALID_HANDLE;
        _renderer->buffers  = buffers;
        _renderer->textures = textures;
        _renderer->stats    = {};

        return(_renderer);
    }

    IFB_INTERNAL void
    renderer_startup(
        reservation* res) {

        const auto& cfg = config_instance();

        assert(_renderer != NULL);
        assert(res);

        // create the stack
        _renderer->memory.stack = reservation_push_stack_all(res);

        // NOTE(SAM): the renderer doesn't need to initialize the opengl context
        // we can pass the context to the function and use it that way
        // same for imgui, it can be initialized externally

        // set global up
        _renderer->global_up       = { 0.0f, 1.0f, 0.0f };
        _renderer->is_grid_visible = true;

        // initialize opengl and imgui
        pfm_graphics_init_opengl(_renderer->gl);
        pfm_graphics_init_imgui();

        // initialize the buffers
        // the shaders size their gl buffers from the render buffers,
        // so this needs to happen first
        render_buffers_alloc_and_init (_renderer->buffers);

        // create shaders
        _renderer->shader.quad            = renderer_quad_shader_create();
        _renderer->shader.grid            = renderer_grid_shader_create();
        _renderer->shader.tile            = renderer_tile_shader_create();
        _renderer->shader.direction_gizmo = renderer_direciton_gizmo_shader_create();
        _renderer->shader.parallax        = renderer_parallax_shader_create();
        _renderer->parallax_revision      = 0;

        // intialize camera
        renderer_viewport_init (_renderer->viewport);
        renderer_camera_init   (_renderer->cam);
        renderer_camera_init   (_renderer->cam_free);
        _renderer->is_cam_free       = false;
        _renderer->is_cam_prev_valid = false;
        _renderer->game_fov_y        = _renderer->viewport->fov_y;

        // the window was already sized when it opened, before the renderer could take it
        // the config size includes the borders and title bar, so it's only a fallback
        u32 viewport_width  = cfg.window_start_width;
        u32 viewport_height = cfg.window_start_height;
        u32 client_width    = 0;
        u32 client_height   = 0;
        if (pfm_window_get_client_size(client_width, client_height)) {
            viewport_width  = client_width;
            viewport_height = client_height;
        }
        renderer_viewport_set_dimensions(_renderer->viewport, _renderer->gl, viewport_width, viewport_height);

        // open shader files
        const hnd_file file_hnd_quad_vert    = file_ro_open_existing ("quad-shader-vertex.glsl");
        const hnd_file file_hnd_quad_frag    = file_ro_open_existing ("quad-shader-fragment.glsl");
        const hnd_file file_hnd_dir_giz_vert = file_ro_open_existing ("direction-gizmo-shader-vert.glsl");
        const hnd_file file_hnd_dir_giz_frag = file_ro_open_existing ("direction-gizmo-shader-frag.glsl");
        const hnd_file file_hnd_grid_vert    = file_ro_open_existing ("grid-vert.glsl");
        const hnd_file file_hnd_grid_frag    = file_ro_open_existing ("grid-frag.glsl");
        const hnd_file file_hnd_tile_vert    = file_ro_open_existing ("tile-vert.glsl");
        const hnd_file file_hnd_tile_frag    = file_ro_open_existing ("tile-frag.glsl");
        const hnd_file file_hnd_plx_vert     = file_ro_open_existing ("parallax-vert.glsl");
        const hnd_file file_hnd_plx_frag     = file_ro_open_existing ("parallax-frag.glsl");

        // read quad shaders        
        renderer_shader_source file_src_quad_vert;
        renderer_shader_source file_src_quad_frag;
        file_src_quad_vert.size = file_get_size (file_hnd_quad_vert); 
        file_src_quad_vert.data = file_read     (file_hnd_quad_vert, file_src_quad_vert.size);
        file_src_quad_frag.size = file_get_size (file_hnd_quad_frag);
        file_src_quad_frag.data = file_read     (file_hnd_quad_frag, file_src_quad_frag.size); 
        
        // read direction gizmo shaders
        renderer_shader_source file_src_dir_giz_vert;
        renderer_shader_source file_src_dir_giz_frag;
        file_src_dir_giz_vert.size = file_get_size (file_hnd_dir_giz_vert); 
        file_src_dir_giz_vert.data = file_read     (file_hnd_dir_giz_vert, file_src_dir_giz_vert.size);
        file_src_dir_giz_frag.size = file_get_size (file_hnd_dir_giz_frag);
        file_src_dir_giz_frag.data = file_read     (file_hnd_dir_giz_frag, file_src_dir_giz_frag.size); 

        // read grid shaders
        renderer_shader_source file_src_grid_vert;
        renderer_shader_source file_src_grid_frag;
        file_src_grid_vert.size = file_get_size (file_hnd_grid_vert); 
        file_src_grid_vert.data = file_read     (file_hnd_grid_vert, file_src_grid_vert.size);
        file_src_grid_frag.size = file_get_size (file_hnd_grid_frag);
        file_src_grid_frag.data = file_read     (file_hnd_grid_frag, file_src_grid_frag.size); 
        
        // read grid shaders
        renderer_shader_source file_src_tile_vert;
        renderer_shader_source file_src_tile_frag;
        file_src_tile_vert.size = file_get_size (file_hnd_tile_vert); 
        file_src_tile_vert.data = file_read     (file_hnd_tile_vert, file_src_tile_vert.size);
        file_src_tile_frag.size = file_get_size (file_hnd_tile_frag);
        file_src_tile_frag.data = file_read     (file_hnd_tile_frag, file_src_tile_frag.size);

        // read parallax shaders
        renderer_shader_source file_src_plx_vert;
        renderer_shader_source file_src_plx_frag;
        file_src_plx_vert.size = file_get_size (file_hnd_plx_vert);
        file_src_plx_vert.data = file_read     (file_hnd_plx_vert, file_src_plx_vert.size);
        file_src_plx_frag.size = file_get_size (file_hnd_plx_frag);
        file_src_plx_frag.data = file_read     (file_hnd_plx_frag, file_src_plx_frag.size);

        // initialize shaders
        renderer_quad_shader_init            (_renderer->shader.quad,            _renderer->gl, _renderer->buffers, file_src_quad_vert,    file_src_quad_frag);
        renderer_direciton_gizmo_shader_init (_renderer->shader.direction_gizmo, _renderer->gl,                     file_src_dir_giz_vert, file_src_dir_giz_frag);
        renderer_grid_shader_init            (_renderer->shader.grid,            _renderer->gl, _renderer->buffers, file_src_grid_vert,    file_src_grid_frag);
        renderer_tile_shader_init            (_renderer->shader.tile,            _renderer->gl, _renderer->buffers, file_src_tile_vert,    file_src_tile_frag);
        renderer_parallax_shader_init        (_renderer->shader.parallax,        _renderer->gl,                     file_src_plx_vert,     file_src_plx_frag);

        // the particle view draws with the quad shader, so it goes after the shaders
        _renderer->particle_view = renderer_particle_view_create();
        renderer_particle_view_init(_renderer->particle_view, _renderer->gl, _renderer->buffers, _renderer->shader.quad);

        // the immediate quads draw with the quad shader too
        _renderer->immediate = renderer_immediate_create();
        renderer_immediate_init(_renderer->immediate, _renderer->gl, _renderer->shader.quad);

        // the map view draws with the tile shader, so it goes after the shaders too
        _renderer->map_view = renderer_map_view_create();
        renderer_map_view_init(_renderer->map_view, _renderer->gl);

        // close the shader files
        file_close(file_hnd_quad_vert);
        file_close(file_hnd_quad_frag);
        file_close(file_hnd_dir_giz_vert);
        file_close(file_hnd_dir_giz_frag);
        file_close(file_hnd_grid_vert);
        file_close(file_hnd_grid_frag);
        file_close(file_hnd_tile_vert);
        file_close(file_hnd_tile_frag);
        file_close(file_hnd_plx_vert);
        file_close(file_hnd_plx_frag);
    }

    IFB_INTERNAL void
    renderer_shutdown(
        void) {

        //TODO
    }

    IFB_INTERNAL void*
    renderer_memory_alloc(
        const u32 size) {

        assert(size != 0);

        void* mem = eng_stack_push(_renderer->memory.stack, size);

        return(mem);
    }

    IFB_INTERNAL void 
    renderer_view_projection_xform(
        mat4& xform) {

        // calculate view and projection matrices
        static mat4 proj = mat4_identity();
        static mat4 view = mat4_identity();
        renderer_viewport_get_xform (_renderer->viewport, proj);
        renderer_camera_get_xform   (renderer_view_camera(), view);
       
        // calculate view projection
        xform = mat4_multiply(proj, view);
    }
    
    // called when a simulation step starts, before the game moves the camera
    IFB_INTERNAL void
    renderer_begin_step(
        void) {

        assert(_renderer);

        _renderer->cam_origin_prev   = renderer_camera_get_origin(_renderer->cam);
        _renderer->cam_target_prev   = renderer_camera_get_target(_renderer->cam);
        _renderer->is_cam_prev_valid = true;
    }

    // alpha is how far this frame is between the last step's start and its end, 0 to 1
    // entities and the game's camera are drawn that far between where they were and where they are
    // the free camera isn't, the tools move it every frame
    IFB_INTERNAL void
    renderer_draw_buffers(
        const f32 alpha) {

        IFB_PROFILE_FUNCTION();

        auto* gl      = _renderer->gl;
        const hnd_map h_map = _renderer->map;
        auto* buffers = _renderer->buffers;

        cmpnt_positions_set_render_alpha(alpha);

        // the game's camera is put back once the frame is drawn, so the game and tools only ever see where it really is
        renderer_camera* cam         = _renderer->cam;
        const vec3       cam_origin  = renderer_camera_get_origin(cam);
        const vec3       cam_target  = renderer_camera_get_target(cam);
        const bool       is_cam_lerp = (!_renderer->is_cam_free && _renderer->is_cam_prev_valid);
        if (is_cam_lerp) {
            const vec3& o0 = _renderer->cam_origin_prev;
            const vec3& t0 = _renderer->cam_target_prev;
            const vec3  origin = { o0.x + ((cam_origin.x - o0.x) * alpha), o0.y + ((cam_origin.y - o0.y) * alpha), o0.z + ((cam_origin.z - o0.z) * alpha) };
            const vec3  target = { t0.x + ((cam_target.x - t0.x) * alpha), t0.y + ((cam_target.y - t0.y) * alpha), t0.z + ((cam_target.z - t0.z) * alpha) };
            renderer_camera_set_origin(cam, origin);
            renderer_camera_set_target(cam, target);
        }

        mat4 view_proj_xform;
        renderer_view_projection_xform(view_proj_xform);

        // the backdrop belongs to the highest rendering scene that has one
        renderer_sync_parallax();

        // map, entities, and particles -> buffers
        render_buffers_render_tiles   (buffers, h_map);
        render_buffers_render_quads   (buffers, renderer_view_camera());

        // the map view refills the tile buffer later, so the stats are taken now
        renderer_stats& stats = _renderer->stats;
        renderer_stats_record(stats.tiles,           buffers->tile_vertices->size);
        renderer_stats_record(stats.quads,           buffers->quad_vertices->size);
        renderer_stats_record(stats.quads_entities,  buffers->quad_size_entities);
        renderer_stats_record(stats.quads_particles, buffers->quad_size_particles);

        // buffers -> shaders
        // the parallax backdrop stands on the far edge of the map
        // it goes first so everything draws over it
        f32 backdrop_z = 0.0f;
        if (renderer_get_parallax_backdrop_z(h_map, backdrop_z)) {
            renderer_parallax_shader_draw(
                _renderer->shader.parallax,
                gl,
                view_proj_xform,
                renderer_camera_get_origin(renderer_view_camera()),
                _renderer->viewport->fov_y,
                renderer_viewport_get_aspect_ratio(_renderer->viewport),
                backdrop_z
            );
        }
        //renderer_direction_gizmo_draw (view_proj_xform);
        renderer_tile_draw           (_renderer->shader.tile, gl, buffers, h_map, view_proj_xform);
        if (_renderer->is_grid_visible) {
            renderer_grid_draw        (_renderer->shader.grid, gl, buffers, view_proj_xform);
        }
        renderer_quad_draw            (_renderer->shader.quad, gl, buffers, view_proj_xform);
        renderer_immediate_draw_world (_renderer->immediate, gl, _renderer->shader.quad, view_proj_xform);

        // the screen layer goes over everything in the window
        u32 screen_width  = 0;
        u32 screen_height = 0;
        renderer_get_screen_size(screen_width, screen_height);
        if (screen_width != 0 && screen_height != 0) {
            renderer_immediate_draw_screen(_renderer->immediate, gl, _renderer->shader.quad, (f32)screen_width, (f32)screen_height);
        }

        // the particle view draws offscreen, then puts the window viewport back
        renderer_particle_view_draw(_renderer->particle_view, gl, buffers, _renderer->shader.quad, _renderer->shader.grid);

        // the map view reuses the tile buffer, so it goes after the game's map is drawn
        renderer_map_view_draw(_renderer->map_view, gl, buffers, _renderer->shader.tile, _renderer->shader.grid);
        gl_context_update_viewport(gl, 0, 0, (u32)_renderer->viewport->width, (u32)_renderer->viewport->height);

        // the immediate layers are cleared at the start of the next frame, so they're still full here
        renderer_stats_record(stats.immediate_world,  _renderer->immediate->world.size);
        renderer_stats_record(stats.immediate_screen, _renderer->immediate->screen.size);
        renderer_stats_record(stats.particle_view,    _renderer->particle_view->vertices.size);
        ++stats.frame_count;

        if (is_cam_lerp) {
            renderer_camera_set_origin(cam, cam_origin);
            renderer_camera_set_target(cam, cam_target);
        }
    }

    inline void
    renderer_stats_fill_buffer(
        eng_renderer_stats_buffer&  out,
        const char*                 name,
        const renderer_buffer_stat& stat,
        const u32                   size_capacity,
        const u32                   item_size) {

        out.name          = name;
        out.size_used     = stat.size_used;
        out.size_peak     = stat.size_peak;
        out.size_capacity = size_capacity;
        out.item_size     = item_size;
    }

    IFB_INTERNAL void
    renderer_get_stats(
        eng_renderer_stats& out) {

        assert(_renderer);

        const renderer_stats&   stats    = _renderer->stats;
        const render_buffers*   buffers  = _renderer->buffers;
        const render_textures*  textures = _renderer->textures;
        const u32               quad     = sizeof(quad_vertices);

        out.frame_count = stats.frame_count;

        // buffers
        auto* bufs = out.buffers;
        renderer_stats_fill_buffer(bufs[eng_renderer_buffer_e_quads],            "Quads",            stats.quads,            buffers->quad_vertices->capacity,         quad);
        renderer_stats_fill_buffer(bufs[eng_renderer_buffer_e_tiles],            "Tiles",            stats.tiles,            buffers->tile_vertices->capacity,         sizeof(map_tile));
        renderer_stats_fill_buffer(bufs[eng_renderer_buffer_e_immediate_world],  "Immediate World",  stats.immediate_world,  _renderer->immediate->world.capacity,     quad);
        renderer_stats_fill_buffer(bufs[eng_renderer_buffer_e_immediate_screen], "Immediate Screen", stats.immediate_screen, _renderer->immediate->screen.capacity,    quad);
        renderer_stats_fill_buffer(bufs[eng_renderer_buffer_e_particle_view],    "Particle View",    stats.particle_view,    _renderer->particle_view->vertices.capacity, quad);
        renderer_stats_fill_buffer(out.quads_entities,  "Entity Quads", stats.quads_entities,  buffers->quad_vertices->capacity, quad);
        renderer_stats_fill_buffer(out.quads_particles, "Particles",    stats.quads_particles, buffers->quad_vertices->capacity, quad);

        // memory
        out.memory.size_used     = eng_stack_get_position (_renderer->memory.stack);
        out.memory.size_capacity = eng_stack_get_capacity (_renderer->memory.stack);

        // views
        out.viewport.width  = (u32)_renderer->viewport->width;
        out.viewport.height = (u32)_renderer->viewport->height;
        out.viewport.fov_y  = _renderer->viewport->fov_y;
        renderer_get_screen_size(out.screen.width, out.screen.height);
        out.particle_view.width  = _renderer->particle_view->width;
        out.particle_view.height = _renderer->particle_view->height;
        out.map_view.width       = _renderer->map_view->width;
        out.map_view.height      = _renderer->map_view->height;
        out.map                  = _renderer->map;

        // textures
        out.sprite_atlases.count    = 0;
        out.sprite_atlases.capacity = config::sprite_atlas_table_capacity;
        out.sprite_atlases.active   = textures->sprite_atlas_active_hnd;
        for (
            u32 i_atlas = 0;
                i_atlas < config::sprite_atlas_table_capacity;
              ++i_atlas) {

            out.sprite_atlases.count += (textures->sprite_atlas[i_atlas].texture_id != GL_ID_INVALID) ? 1 : 0;
        }

        out.fonts.count      = 0;
        out.fonts.capacity   = RENDERER_FONT_CAPACITY;
        out.fonts.has_active = (textures->font_active_fnt != NULL);
        for (
            u32 i_font = 0;
                i_font < RENDERER_FONT_CAPACITY;
              ++i_font) {

            out.fonts.count += (textures->fonts[i_font].texture_id != GL_ID_INVALID) ? 1 : 0;
        }
    }

    // the peaks start again from what the last frame used
    IFB_INTERNAL void
    renderer_reset_stats_peaks(
        void) {

        assert(_renderer);

        renderer_buffer_stat* stats[] = {
            &_renderer->stats.quads,
            &_renderer->stats.quads_entities,
            &_renderer->stats.quads_particles,
            &_renderer->stats.tiles,
            &_renderer->stats.immediate_world,
            &_renderer->stats.immediate_screen,
            &_renderer->stats.particle_view,
        };
        for (renderer_buffer_stat* stat : stats) {
            stat->size_peak = stat->size_used;
        }
    }

    // true if the atlas is on the gpu, uploading it again does nothing
    IFB_INTERNAL bool
    renderer_upload_sprite_atlas(
        const hnd_sprite_atlas h_atlas) {

        assert(h_atlas != INVALID_HANDLE);
        assert(_renderer);

        gl_context* gl = _renderer->gl;
        assert(_renderer->textures);

        // already uploaded
        if (renderer_find_sprite_atlas_texture(h_atlas) != NULL) {
            return(true);
        }

        // find a free slot
        render_sprite_atlas_texture* atlas = renderer_find_sprite_atlas_texture(INVALID_HANDLE);
        if (atlas == NULL) return(false);

        // copy the atlas image out of the sprite manager
        const hnd_arena h_arena = arena_large_alloc();
        if (h_arena == INVALID_HANDLE) return(false);

        const image* img = sprite_mngr_get_sprite_atlas_image_by_handle(
            h_atlas,
            h_arena
        );
        if (img == NULL) {
            arena_free(h_arena);
            return(false);
        }

        const gl_texture texture = gl_texture_create(gl);
        bool did_upload = (texture != GL_ID_INVALID);
        if (did_upload) {
            did_upload = gl_texture_set_data_rgba8(
                gl,
                texture,
                img->pixel_width,
                img->pixel_height,
                img->data.bytes
            );
        }
        arena_free(h_arena);

        if (!did_upload) {
            if (texture != GL_ID_INVALID) gl_texture_destroy(gl, texture);
            return(false);
        }

        atlas->texture_id = texture;
        atlas->h_atlas    = h_atlas;
        return(true);
    }

    // false if the atlas was never uploaded
    IFB_INTERNAL bool
    renderer_release_sprite_atlas(
        const hnd_sprite_atlas h_atlas) {

        assert(h_atlas != INVALID_HANDLE);
        assert(_renderer);

        auto textures = _renderer->textures;
        assert(textures);

        render_sprite_atlas_texture* atlas = renderer_find_sprite_atlas_texture(h_atlas);
        if (atlas == NULL) return(false);

        if (textures->sprite_atlas_active == atlas->texture_id) {
            textures->sprite_atlas_active     = GL_ID_INVALID;
            textures->sprite_atlas_active_hnd = INVALID_HANDLE;
        }

        gl_texture_destroy(_renderer->gl, atlas->texture_id);
        atlas->texture_id = GL_ID_INVALID;
        atlas->h_atlas    = INVALID_HANDLE;
        return(true);
    }

    // false if the atlas was never uploaded
    IFB_INTERNAL bool
    renderer_set_active_sprite_atlas(
        const hnd_sprite_atlas h_atlas) {

        assert(h_atlas != INVALID_HANDLE);
        assert(_renderer);

        auto textures = _renderer->textures;
        assert(textures);

        const render_sprite_atlas_texture* atlas = renderer_find_sprite_atlas_texture(h_atlas);
        if (atlas == NULL) return(false);

        textures->sprite_atlas_active     = atlas->texture_id;
        textures->sprite_atlas_active_hnd = h_atlas;
        return(true);
    }

    // the atlas the tile and quad shaders draw from, INVALID_HANDLE if none is active
    IFB_INTERNAL hnd_sprite_atlas
    renderer_get_active_sprite_atlas(
        void) {

        assert(_renderer);
        return(_renderer->textures->sprite_atlas_active_hnd);
    }

    // the atlas's opengl texture, for showing sprites in the gui, 0 if it isn't uploaded
    // the top row of the atlas is v = 0
    IFB_INTERNAL u32
    renderer_get_sprite_atlas_texture(
        const hnd_sprite_atlas h_atlas) {

        assert(_renderer);
        if (h_atlas == INVALID_HANDLE) return(GL_ID_INVALID);

        const render_sprite_atlas_texture* atlas = renderer_find_sprite_atlas_texture(h_atlas);
        return((atlas != NULL) ? atlas->texture_id : GL_ID_INVALID);
    }

    IFB_INTERNAL bool
    renderer_set_map(
        const hnd_map h_map) {

        assert(h_map                      != INVALID_HANDLE);
        assert(_renderer              != NULL);

        // make sure the map exists
        map_dimensions dims;
        const bool did_set = map_get_dimensions(h_map, dims);
        if (did_set) {
            _renderer->map = h_map;
        }
        return(did_set);
    }

    // the far edge of the map, where the parallax backdrop stands
    // the camera looks down -z, so the far edge is the first row
    IFB_INTERNAL bool
    renderer_get_parallax_backdrop_z(
        const hnd_map h_map,
        f32&          backdrop_z) {

        if (h_map == INVALID_HANDLE) return(false);

        vec3 map_origin;
        u32  row_min = 0;
        u32  col_min = 0;
        u32  row_max = 0;
        u32  col_max = 0;
        if (!map_get_origin           (h_map, map_origin))                         return(false);
        if (!map_get_navigable_bounds (h_map, row_min, col_min, row_max, col_max)) return(false);

        // it sits a little behind the edge so it doesn't fight the back wall for depth
        const auto& cfg = config_instance();
        backdrop_z = map_origin.z + ((f32)row_min * cfg.map_tile_unit_size) - PARALLAX_BACKDROP_INSET;
        return(true);
    }

    // loads the layers when the scene parallax that should be drawn changes
    // a revision that fails to load isn't tried again, nothing is drawn until the next one
    IFB_INTERNAL void
    renderer_sync_parallax(
        void) {

        assert(_renderer);
        assert(_renderer->shader.parallax);

        const scene_parallax* plx      = scene_get_parallax_drawn();
        const u32             revision = (plx != NULL) ? plx->revision : 0;
        if (revision == _renderer->parallax_revision) return;

        _renderer->parallax_revision = revision;
        if (plx == NULL) {
            _renderer->shader.parallax->is_set = false;
            return;
        }

        const bool did_set = renderer_set_parallax(plx->back, plx->mid, plx->front);
        if (!did_set) {
            _renderer->shader.parallax->is_set = false;
        }
    }

    // the names are image assets, they all have to be the same size
    IFB_INTERNAL bool
    renderer_set_parallax(
        const cchar* back,
        const cchar* mid,
        const cchar* front) {

        assert(_renderer);
        assert(_renderer->shader.parallax);

        const cchar* names[PARALLAX_LAYER_COUNT] = { back, mid, front };
        const bool   did_set = renderer_parallax_shader_set_layers(
            _renderer->shader.parallax,
            _renderer->gl,
            names
        );
        return(did_set);
    }

    IFB_INTERNAL bool
    renderer_is_grid_visible(
        void) {

        assert(_renderer != NULL);
        return(_renderer->is_grid_visible);
    }

    IFB_INTERNAL void
    renderer_set_grid_visible(
        const bool is_visible) {

        assert(_renderer != NULL);
        _renderer->is_grid_visible = is_visible;
    }
    
    // INVALID_HANDLE finds a free slot
    IFB_INTERNAL render_sprite_atlas_texture*
    renderer_find_sprite_atlas_texture(
        const hnd_sprite_atlas h_atlas) {

        assert(_renderer);
        assert(_renderer->textures);

        auto textures = _renderer->textures;
        for (
              u32 i_atlas = 0;
                  i_atlas < config::sprite_atlas_table_capacity;
                ++i_atlas) {

            if (textures->sprite_atlas[i_atlas].h_atlas == h_atlas) {
                return(&textures->sprite_atlas[i_atlas]);
            }
        }
        return(NULL);
    }

    // binds the active atlas to its texture slot, does nothing if no atlas is active
    IFB_INTERNAL void
    renderer_use_sprite_atlas_texture(
        void) {

        assert(_renderer);
        assert(_renderer->textures);

        const gl_texture texture = _renderer->textures->sprite_atlas_active;
        if (texture == GL_ID_INVALID) return;

        const bool did_set = gl_context_set_texture_active(
            _renderer->gl,
            texture,
            RENDER_GL_TEXTURE_SLOT_SPRITE_ATLAS
        );
        assert(did_set);
        (void)did_set;
    }

    // INVALID finds a free slot
    IFB_INTERNAL render_font_texture*
    renderer_find_font_texture(
        const font* fnt) {

        assert(_renderer);
        assert(_renderer->textures);

        auto textures = _renderer->textures;
        for (
              u32 i_font = 0;
                  i_font < RENDERER_FONT_CAPACITY;
                ++i_font) {

            if (textures->fonts[i_font].fnt == fnt) {
                return(&textures->fonts[i_font]);
            }
        }
        return(NULL);
    }

    // binds the active font and sprite atlas for the quad shader
    // the sprite atlas goes last, so texture slot 0 is the active one afterwards like everything else expects
    IFB_INTERNAL void
    renderer_use_quad_textures(
        void) {

        assert(_renderer);
        assert(_renderer->textures);

        const gl_texture font_texture = _renderer->textures->font_active;
        if (font_texture != GL_ID_INVALID) {
            const bool did_set = gl_context_set_texture_active(
                _renderer->gl,
                font_texture,
                RENDER_GL_TEXTURE_SLOT_FONT_ATLAS
            );
            assert(did_set);
            (void)did_set;
        }

        glActiveTexture(GL_TEXTURE0 + RENDER_GL_TEXTURE_SLOT_SPRITE_ATLAS);
        renderer_use_sprite_atlas_texture();
    }

    //--------------------------------------------------------------------
    // FONT METHODS
    //--------------------------------------------------------------------

    // true if the font's atlas is on the gpu, uploading it again does nothing
    // the atlas is one coverage byte per pixel, so it goes up as a single red channel
    // fonts without oversampling are pixel fonts, they're filtered nearest so they stay sharp
    IFB_INTERNAL bool
    renderer_upload_font(
        const font* fnt) {

        assert(fnt);
        assert(_renderer);

        if (renderer_find_font_texture(fnt) != NULL) return(true);

        render_font_texture* slot = renderer_find_font_texture(NULL);
        if (slot == NULL) return(false);

        gl_context*      gl      = _renderer->gl;
        const gl_texture texture = gl_texture_create(gl);
        if (texture == GL_ID_INVALID) return(false);

        const bool  is_nearest = (fnt->oversample_x == 1 && fnt->oversample_y == 1);
        const GLint filter     = is_nearest ? GL_NEAREST : GL_LINEAR;

        (void)glGetError();
        glBindTexture   (GL_TEXTURE_2D, texture);
        glPixelStorei   (GL_UNPACK_ALIGNMENT, 1); // the rows are tightly packed bytes
        glTexImage2D    (GL_TEXTURE_2D, 0, GL_R8, (GLsizei)fnt->atlas_width, (GLsizei)fnt->atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, fnt->atlas_pixels());
        glPixelStorei   (GL_UNPACK_ALIGNMENT, 4);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glBindTexture   (GL_TEXTURE_2D, GL_ID_INVALID);

        if (glGetError() != GL_NO_ERROR) {
            gl_texture_destroy(gl, texture);
            return(false);
        }

        slot->fnt        = fnt;
        slot->texture_id = texture;
        return(true);
    }

    // false if the font was never uploaded
    IFB_INTERNAL bool
    renderer_release_font(
        const font* fnt) {

        assert(fnt);
        assert(_renderer);

        auto textures = _renderer->textures;
        assert(textures);

        render_font_texture* slot = renderer_find_font_texture(fnt);
        if (slot == NULL) return(false);

        if (textures->font_active_fnt == fnt) {
            textures->font_active     = GL_ID_INVALID;
            textures->font_active_fnt = NULL;
        }

        gl_texture_destroy(_renderer->gl, slot->texture_id);
        slot->fnt        = NULL;
        slot->texture_id = GL_ID_INVALID;
        return(true);
    }

    // false if the font was never uploaded
    IFB_INTERNAL bool
    renderer_set_active_font(
        const font* fnt) {

        assert(fnt);
        assert(_renderer);

        auto textures = _renderer->textures;
        assert(textures);

        const render_font_texture* slot = renderer_find_font_texture(fnt);
        if (slot == NULL) return(false);

        textures->font_active     = slot->texture_id;
        textures->font_active_fnt = fnt;
        return(true);
    }

    // the font text draws with, NULL if none is active
    IFB_INTERNAL const font*
    renderer_get_active_font(
        void) {

        assert(_renderer);
        return(_renderer->textures->font_active_fnt);
    }

    //--------------------------------------------------------------------
    // IMMEDIATE METHODS
    //--------------------------------------------------------------------

    // the game callback can run more than once a frame, so the quads are cleared before
    // each run, and the frame draws whatever the last run pushed
    IFB_INTERNAL void
    renderer_immediate_reset(
        void) {

        assert(_renderer);
        renderer_immediate_clear(_renderer->immediate);
    }

    IFB_INTERNAL bool
    renderer_immediate_push_world(
        const quad_vertices& vertices) {

        assert(_renderer);
        const bool did_push = renderer_immediate_push(&_renderer->immediate->world, vertices);
        return(did_push);
    }

    IFB_INTERNAL bool
    renderer_immediate_push_screen(
        const quad_vertices& vertices) {

        assert(_renderer);
        const bool did_push = renderer_immediate_push(&_renderer->immediate->screen, vertices);
        return(did_push);
    }

    // the window's drawable area in pixels
    IFB_INTERNAL void
    renderer_get_viewport_size(
        u32& width,
        u32& height) {

        assert(_renderer);
        width  = (u32)_renderer->viewport->width;
        height = (u32)_renderer->viewport->height;
    }

    // the pixels the screen layer is laid out in, it's stretched to fill the window
    // 0 x 0 uses the window's size, so one screen pixel is one window pixel
    IFB_INTERNAL void
    renderer_set_screen_size(
        const u32 width,
        const u32 height) {

        assert(_renderer);
        assert((width == 0) == (height == 0));

        _renderer->immediate->screen_width  = width;
        _renderer->immediate->screen_height = height;
    }

    IFB_INTERNAL void
    renderer_get_screen_size(
        u32& width,
        u32& height) {

        assert(_renderer);

        const renderer_immediate* imm = _renderer->immediate;
        if (imm->screen_width != 0 && imm->screen_height != 0) {
            width  = imm->screen_width;
            height = imm->screen_height;
        }
        else {
            width  = (u32)_renderer->viewport->width;
            height = (u32)_renderer->viewport->height;
        }
    }
    
    //--------------------------------------------------------------------
    // CAMERA METHODS 
    //--------------------------------------------------------------------
    
    IFB_INTERNAL const vec3&
    renderer_get_camera_origin(
        void) {

        assert(_renderer);
        return(renderer_camera_get_origin(_renderer->cam));
    }

    IFB_INTERNAL const vec3&
    renderer_get_camera_target(
        void) {

        assert(_renderer);
        return(renderer_camera_get_target(_renderer->cam));
    }

    IFB_INTERNAL void
    renderer_get_camera_orientation(
        orientation& o) {

        assert(_renderer);
        renderer_camera_get_orientation(_renderer->cam, o);
    }

    IFB_INTERNAL void 
    renderer_get_camera_xform(
        mat4& xform) {

        assert(_renderer);
        renderer_camera_get_xform(_renderer->cam, xform);
    }

    IFB_INTERNAL void
    renderer_set_camera_origin(
        const vec3& origin) {

        assert(_renderer);
        renderer_camera_set_origin(_renderer->cam, origin);
    }

    IFB_INTERNAL void
    renderer_set_camera_target(
        const vec3& target) {

        assert(_renderer);
        renderer_camera_set_target(_renderer->cam, target);
    }
    
    //--------------------------------------------------------------------
    // VIEWPORT METHODS 
    //--------------------------------------------------------------------
   
    IFB_INTERNAL void
    renderer_request_particle_view(
        const u32 width,
        const u32 height) {

        assert(_renderer);
        renderer_particle_view_request(_renderer->particle_view, width, height);
    }

    IFB_INTERNAL u32
    renderer_get_particle_view_texture(
        void) {

        assert(_renderer);
        assert(_renderer->particle_view);
        return(_renderer->particle_view->gl.texture);
    }

    IFB_INTERNAL void
    renderer_request_map_view(
        const hnd_map h_map,
        const u32     width,
        const u32     height) {

        assert(_renderer);
        renderer_map_view_request(_renderer->map_view, h_map, width, height);
    }

    IFB_INTERNAL u32
    renderer_get_map_view_texture(
        void) {

        assert(_renderer);
        assert(_renderer->map_view);
        return(_renderer->map_view->gl.texture);
    }

    IFB_INTERNAL void
    renderer_get_map_view_camera(
        vec3& origin,
        vec3& target) {

        assert(_renderer);
        assert(_renderer->map_view);
        origin = renderer_camera_get_origin(_renderer->map_view->camera);
        target = renderer_camera_get_target(_renderer->map_view->camera);
    }

    IFB_INTERNAL void
    renderer_set_map_view_camera(
        const vec3& origin,
        const vec3& target) {

        assert(_renderer);
        assert(_renderer->map_view);
        renderer_camera_set_origin(_renderer->map_view->camera, origin);
        renderer_camera_set_target(_renderer->map_view->camera, target);
    }

    // the view projection of the last size the view was requested at
    IFB_INTERNAL void
    renderer_get_map_view_xform(
        mat4& xform) {

        assert(_renderer);
        assert(_renderer->map_view);
        renderer_map_view_get_view_proj(_renderer->map_view, xform);
    }

    IFB_INTERNAL void
    renderer_set_map_view_grid_visible(
        const bool is_visible) {

        assert(_renderer);
        assert(_renderer->map_view);
        _renderer->map_view->is_grid_visible = is_visible;
    }

    // a texture for the gui to preview an image in each slot, the pixels are rgba8 from the top row down
    // the texture is an opengl texture id, or 0 if the pixels couldn't be copied
    // nearest filtering keeps pixel art and pixel fonts sharp when the gui scales them up
    IFB_INTERNAL u32
    renderer_set_asset_preview_texture(
        const u32   slot,
        const u32   width,
        const u32   height,
        const byte* pixels,
        const bool  is_nearest) {

        static gl_texture textures[RENDERER_ASSET_PREVIEW_COUNT] = { GL_ID_INVALID, GL_ID_INVALID, GL_ID_INVALID, GL_ID_INVALID, GL_ID_INVALID };

        assert(_renderer);
        assert(pixels);
        assert(slot < RENDERER_ASSET_PREVIEW_COUNT);
        if (width == 0 || height == 0) return(GL_ID_INVALID);

        gl_context* gl      = _renderer->gl;
        gl_texture& texture = textures[slot];

        if (texture == GL_ID_INVALID) {
            texture = gl_texture_create(gl);
        }
        if (texture == GL_ID_INVALID) return(GL_ID_INVALID);

        // the storage is the size of the image, then the pixels are copied in
        // setting the storage also sets linear filtering
        if (!gl_texture_set_storage_rgba8(gl, texture, width, height)) return(GL_ID_INVALID);

        glBindTexture   (GL_TEXTURE_2D, texture);
        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        if (is_nearest) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glBindTexture   (GL_TEXTURE_2D, GL_ID_INVALID);

        const bool did_copy = (glGetError() == GL_NO_ERROR);
        return(did_copy ? texture : GL_ID_INVALID);
    }

    IFB_INTERNAL const vec3&
    renderer_get_particle_view_camera_origin(
        void) {

        assert(_renderer);
        assert(_renderer->particle_view);
        return(renderer_camera_get_origin(_renderer->particle_view->camera));
    }

    IFB_INTERNAL const vec3&
    renderer_get_particle_view_camera_target(
        void) {

        assert(_renderer);
        assert(_renderer->particle_view);
        return(renderer_camera_get_target(_renderer->particle_view->camera));
    }

    IFB_INTERNAL void
    renderer_set_particle_view_camera_origin(
        const vec3& origin) {

        assert(_renderer);
        assert(_renderer->particle_view);
        renderer_camera_set_origin(_renderer->particle_view->camera, origin);
    }

    IFB_INTERNAL void
    renderer_set_particle_view_camera_target(
        const vec3& target) {

        assert(_renderer);
        assert(_renderer->particle_view);
        renderer_camera_set_target(_renderer->particle_view->camera, target);
    }

    IFB_INTERNAL void
    renderer_set_viewport_dimensions(
        const u32 width,
        const u32 height) {

        assert(width  != 0);
        assert(height != 0);
        assert(_renderer);
        assert(_renderer->viewport);
       
        renderer_viewport_set_dimensions(
            _renderer->viewport,
            _renderer->gl,
            width, height
        );
    }

    IFB_INTERNAL f32
    renderer_get_viewport_aspect_ratio(
        void) {

        assert(_renderer);
        assert(_renderer->viewport);

        const f32 aspect_ratio = renderer_viewport_get_aspect_ratio(_renderer->viewport);
        return(aspect_ratio);
    }

    IFB_INTERNAL void
    renderer_get_viewport_xform(
        mat4& xform) {

        assert(_renderer);
        assert(_renderer->viewport);

        renderer_viewport_get_xform(_renderer->viewport, xform);
    }

    // the game's vertical field of view, in radians
    // while the free camera is on, the viewport has the free camera's fov, so the game's is kept aside
    IFB_INTERNAL f32
    renderer_get_viewport_fov(
        void) {

        assert(_renderer);
        assert(_renderer->viewport);

        return(_renderer->is_cam_free ? _renderer->game_fov_y : _renderer->viewport->fov_y);
    }

    IFB_INTERNAL void
    renderer_set_viewport_fov(
        const f32 fov_y) {

        assert(_renderer);
        assert(_renderer->viewport);
        assert(fov_y > 0.0f);

        if (_renderer->is_cam_free) {
            _renderer->game_fov_y = fov_y;
        }
        else {
            _renderer->viewport->fov_y = fov_y;
        }
    }

    //--------------------------------------------------------------------
    // FREE CAMERA METHODS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    renderer_free_camera_set_enabled(
        const bool is_enabled) {

        assert(_renderer);
        if (is_enabled == _renderer->is_cam_free) return;

        if (is_enabled) {
            _renderer->game_fov_y  = _renderer->viewport->fov_y;
            *_renderer->cam_free   = *_renderer->cam;
            _renderer->is_cam_free = true;
        }
        else {
            _renderer->viewport->fov_y = _renderer->game_fov_y;
            _renderer->is_cam_free     = false;
        }
    }

    IFB_INTERNAL bool
    renderer_free_camera_is_enabled(
        void) {

        assert(_renderer);
        return(_renderer->is_cam_free);
    }

    // puts the free camera where the game's camera is, it stays on
    IFB_INTERNAL void
    renderer_free_camera_reset(
        void) {

        assert(_renderer);

        *_renderer->cam_free = *_renderer->cam;
        if (_renderer->is_cam_free) {
            _renderer->viewport->fov_y = _renderer->game_fov_y;
        }
    }

    IFB_INTERNAL const vec3&
    renderer_free_camera_get_origin(
        void) {

        assert(_renderer);
        return(renderer_camera_get_origin(_renderer->cam_free));
    }

    IFB_INTERNAL const vec3&
    renderer_free_camera_get_target(
        void) {

        assert(_renderer);
        return(renderer_camera_get_target(_renderer->cam_free));
    }

    IFB_INTERNAL void
    renderer_free_camera_set_origin(
        const vec3& origin) {

        assert(_renderer);
        renderer_camera_set_origin(_renderer->cam_free, origin);
    }

    IFB_INTERNAL void
    renderer_free_camera_set_target(
        const vec3& target) {

        assert(_renderer);
        renderer_camera_set_target(_renderer->cam_free, target);
    }

    IFB_INTERNAL void
    renderer_free_camera_get_xform(
        mat4& xform) {

        assert(_renderer);
        renderer_camera_get_xform(_renderer->cam_free, xform);
    }

    // the free camera's vertical field of view, in radians
    // it only has its own while it's on, otherwise it's the game's
    IFB_INTERNAL f32
    renderer_free_camera_get_fov(
        void) {

        assert(_renderer);
        return(_renderer->viewport->fov_y);
    }

    IFB_INTERNAL void
    renderer_free_camera_set_fov(
        const f32 fov_y) {

        assert(_renderer);
        assert(fov_y > 0.0f);
        if (!_renderer->is_cam_free) return;

        _renderer->viewport->fov_y = fov_y;
    }
};
          
