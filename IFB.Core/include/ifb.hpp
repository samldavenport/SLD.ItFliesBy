#ifndef IFB_HPP
#define IFB_HPP

#include <imgui.h>
#include <implot.h>

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>
#include <sld-strings.hpp>

#include "ifb-config.hpp"

//--------------------------------------------------------------------
// EXPORTS
//--------------------------------------------------------------------

#ifdef IFB_STATIC
#   define IFB_API
#elif defined(IFB_API_DLL_EXPORT)
#   define IFB_API __declspec(dllexport)
#else
#   define IFB_API __declspec(dllimport)
#endif


//--------------------------------------------------------------------
// MACROS
//--------------------------------------------------------------------

using namespace sld;

#define IFB_U32(type)     struct type : sld::strong_u32 { using strong_u32::strong_u32; };
#define IFB_U16(type)     struct type : sld::strong_u16 { using strong_u16::strong_u16; };
#define IFB_U8(type)      struct type : sld::strong_u8  { using strong_u8::strong_u8;   };
#define IFB_FLAGS32(type) struct type : flags           { using flags;                  };
#define IFB_HANDLE(hnd)   struct hnd  : handle          { using handle::handle;         };

#define IFB_INTERNAL static
#define IFB_INLINE   inline

//--------------------------------------------------------------------
// IMGUI
//--------------------------------------------------------------------

#ifdef _WIN32

//imgui win32 callback
extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(
    HWND   hWnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam);

#endif

namespace ifb {

    //====================================================================
    // CORE
    //====================================================================

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    constexpr u32 INVALID_U16     = 0xFFFF;
    constexpr u32 INVALID_U32     = 0xFFFFFFFF;
    constexpr u32 INVALID_INDEX   = INVALID_U32;
    constexpr u32 INVALID_HASH_32 = INVALID_U32;
    constexpr u32 INVALID_HANDLE  = INVALID_U32;
    constexpr u32 INVALID_ID      = INVALID_U32;
    constexpr f32 POS_MAX         =  1.0f;
    constexpr f32 POS_MIN         = -1.0f;
    constexpr u32 TAG_SIZE        = 16;
    
    //--------------------------------------------------------------------
    // HANDLES & IDS
    //--------------------------------------------------------------------

    IFB_U32(handle);
    IFB_U32(entity_id);

    IFB_HANDLE(hnd_arena);
    IFB_HANDLE(hnd_asset);
    IFB_HANDLE(hnd_audio_voice);
    IFB_HANDLE(hnd_file);
    IFB_HANDLE(hnd_map);
    IFB_HANDLE(hnd_particle_emitter);
    IFB_HANDLE(hnd_sprite_atlas);
    IFB_HANDLE(hnd_sprite);
    IFB_HANDLE(hnd_font_atlas);

    //--------------------------------------------------------------------
    // COMMON TYPES
    //--------------------------------------------------------------------

    struct tag {

        cchar cstr[TAG_SIZE];

        tag(const cchar* cstr) {
            memset((void*)this->cstr, 0, TAG_SIZE);
            const u32 len = strnlen_s(cstr, TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }

        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            memset((void*)this->cstr, 0, TAG_SIZE);

            const u32 len = strnlen_s(cstr, TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }

        inline entity_id
        to_id(void) const {

            const entity_id id = hash_u32((void*)this->cstr, TAG_SIZE);
            return(id);
        }
    };

    using pixel      = color_rgba_u32;
    using origin_3d  = vec3;

    struct text {
        cchar* cstr;
        u32    size;
    };

    // x and y are the top left, y goes down
    // on the screen it's in screen pixels, in a sprite it's in the sprite's pixels
    struct rect {
        f32 x;
        f32 y;
        f32 width;
        f32 height;
    };

    struct file_path {
        cchar cstr[config::file_path_size];
    };

    struct dimensions_2d {
        f32 width;
        f32 height;
    };

    struct dimensions_3d {
        f32 width;
        f32 height;
        f32 depth;
    };

    struct orientation {
        vec3 forward;
        vec3 up;
        vec3 right;
    };

    struct image {
        struct {
            union {
                pixel* pixels;
                byte*  bytes;
                void*  vptr;
                addr   address;
            };
            u32 size;
        } data;
        u32 pixel_width;
        u32 pixel_height;
    };

    // the first 16 bytes of the "fmt " chunk
    struct wav_format {
        u16 audio_format;     // format type
        u16 num_channels;     // number of channels
        u32 sample_rate;      // sample rate
        u32 byte_rate;        // (sample_rate * bits_per_sample * num_channels) / 8
        u16 block_align;      // (bits_per_sample * channels) / 8
        u16 bits_per_sample;  // bits per sample
    };

    struct wav_data {
        byte* bytes;
        u32   size;
    };

    struct wav {
        wav_format format;
        wav_data   data;
    };

    //--------------------------------------------------------------------
    // COLLECTIONS
    //--------------------------------------------------------------------

    struct stack;
    struct index_cache;
    struct entity_list;

    //====================================================================
    // MODULES
    //====================================================================

    //--------------------------------------------------------------------
    // ASSETS
    //--------------------------------------------------------------------

    IFB_U32(asset_type_u32);

    enum asset_type_e {
        asset_type_e_text          = 0,
        asset_type_e_image         = 1,
        asset_type_e_sprite_atlas  = 2,
        asset_type_e_sound         = 3,
        asset_type_e_map           = 4,
        asset_type_e_font_atlas    = 5,
        asset_type_e_count         = 6
    };

    struct sprite_index;
    struct font_glyph;

    struct asset_name {
        cchar cstr [config::asset_name_size];

        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            // the whole name is hashed, so the unused bytes need to be zero
            memset((void*)this->cstr, 0, config::asset_name_size);

            (void)cstr_nvar_copy(
                cstr,
                config::asset_name_size,
                this->cstr,
                config::asset_name_size 
            );
        }

        inline hnd_asset
        to_handle(void) {

            const hnd_asset hnd = hash_u32((void*)cstr, config::asset_name_size);
            assert(hnd != INVALID_HANDLE);

            return(hnd);
        }
    };

    struct asset_index {
        asset_type_u32 type;
        u32            offset;
        u32            size;
    };

    struct asset_text {
        hnd_asset hnd;
        u32       size;
        cchar*    cstr;
    };

    // a map file's json, see docs/map-format.md
    // it's stored like text, but in its own group so maps can be found and loaded apart from other json
    struct asset_map {
        hnd_asset hnd;
        u32       size;
        cchar*    json; // null terminated
    };

    struct asset_image {
        hnd_asset       hnd;
        u32             width;
        u32             height;
        color_rgba_u32* pixels;
    };

    struct asset_sprite_atlas {
        hnd_sprite*      array_handles;
        sprite_index*    array_indexes;
        pixel*           array_pixels;
        hnd_sprite_atlas hnd;
        u32              width;
        u32              height;
        u32              sprite_count;
    };

    // always 16 bit pcm, the database converts sounds when it's built
    // the samples go right after the struct, and w.data.bytes points at them
    struct asset_sound {
        hnd_asset hnd;
        wav       w;
    };

    struct asset_database_info {
        cchar     name [config::asset_name_size + 1];
        hnd_asset hnd;
        u32       type;
        u32       offset;
        u32       size;
    };

    struct asset_dictionary_file_info {
        const cchar* name;
        const cchar* path;
        u32          id;
        u32          type;
    };

    struct asset_preview {
        u32          type;
        const cchar* text;
        b32          is_text_truncated; // the file is bigger than the file buffer
        u32          texture;
        u32          width;
        u32          height;
        const wav*   sound;             // valid until the next preview in the same window
    };

    //--------------------------------------------------------------------
    // SPRITES
    //--------------------------------------------------------------------

    struct sprite_index {
        u32 width;
        u32 height;
        u32 offset;
    };

    struct sprite_info {
        asset_name   name;
        hnd_sprite   hnd;
        u32          index;
        u32          pixel_x;
        u32          pixel_y;
        u32          pixel_width;
        u32          pixel_height;
        u32          atlas_width;
        u32          atlas_height;
    };

    // one chunk of a map
    struct map_chunk_info {
        u32              origin_row;
        u32              origin_col;
        u32              count_rows;
        u32              count_cols;
        u32              color;   // map_color_e
        hnd_sprite_atlas h_atlas; // INVALID_HANDLE until a tile or wall in it has a sprite
    };

    // what's on one tile edge of a map, see eng_map_wall_get_info
    struct map_wall_info {
        u32              chunk_index; // the chunk that owns the edge
        hnd_sprite_atlas h_atlas;     // the owning chunk's atlas, the wall's sprite comes from it
        u32              i_sprite;    // INVALID_INDEX for a solid color wall
        u32              color;       // map_color_e
        bool             is_placed;
    };

    // what's on one floor tile of a map
    struct map_tile_info {
        u32              chunk_index; // the chunk the tile is on
        hnd_sprite_atlas h_atlas;     // the chunk's atlas, INVALID_HANDLE until it has a sprite
        u32              i_sprite;    // the sprite's index in the atlas, INVALID_INDEX for a solid color tile
        u32              color;       // map_color_e
        bool             is_textured;
    };

    struct sprite_atlas_info {
        u32 width;
        u32 height;
        u32 sprite_count;
    };

    struct sprite_atlas_sprite_info {
        cchar      name [config::asset_name_size + 1];
        hnd_sprite hnd;
        u32        x;
        u32        y;
        u32        width;
        u32        height;
    };

    struct sprite_atlas_builder_info {
        cchar     name [config::asset_name_size + 1];
        hnd_asset hnd;
        u32       width;
        u32       height;
    };

    //--------------------------------------------------------------------
    // MAP
    //--------------------------------------------------------------------

    IFB_U8(map_tile_flags_u8);
    IFB_U8(map_wall_flags_u8);
    IFB_U32(map_color_u32);

    // row walls lie on the line z = row and run along x, on the min-z edge of tile (row, col)
    // column walls lie on the line x = col and run along z, on the min-x edge of tile (row, col)
    enum map_wall_orientation_e {
        map_wall_orientation_e_row    = 0,
        map_wall_orientation_e_column = 1,
    };

    enum map_color_e {
        map_color_e_red_light    = 0,
        map_color_e_red_dark     = 1,
        map_color_e_orange_light = 2,
        map_color_e_orange_dark  = 3,
        map_color_e_yellow_light = 4,
        map_color_e_yellow_dark  = 5,
        map_color_e_green_light  = 6,
        map_color_e_green_dark   = 7,
        map_color_e_aqua_light   = 8,
        map_color_e_aqua_dark    = 9,
        map_color_e_blue_light   = 10,
        map_color_e_blue_dark    = 11,
        map_color_e_purple_light = 12,
        map_color_e_purple_dark  = 13,
        map_color_e_gray_light   = 14,
        map_color_e_gray_dark    = 15,
        map_color_e_none         = 16,
    };

    struct map_tile {
        union {
            u32 val;
            struct {
                u16 index;
                u8  color;
                map_tile_flags_u8 flags; // map_tile_flag_e
            };
        };
        f32 u_left;
        f32 v_top;
        f32 u_right;
        f32 v_bottom;
    };

    //--------------------------------------------------------------------
    // PARTICLES
    //--------------------------------------------------------------------

    enum particle_emitter_type_e {
        particle_emitter_type_e_parabolic = 0,
        particle_emitter_type_e_radial    = 1,
        particle_emitter_type_e_count     = 2
    };

    struct particle {
        color_rgba_u32 color;
        vec3           position;
    };

    struct particle_emitter_info {
        const cchar*    name;
        u32             type;
        b32             is_active;   // in the list for its type
        b32             is_spawning;
        u32             count_live;
        u32             count;
        f32             lifespan_ms;
        f32             spawn_delay_ms;
        f32             spawn_radius;    // particles spawn within this distance of the position
        f32             height;
        f32             width;
        vec3            position;
        color_rgba_u32  color_primary;
        color_rgba_u32  color_secondary;
    };

    // fountains, velocity and gravity are in units per second
    struct particle_parabolic_props {
        vec3 velocity;       // launch direction and speed
        vec3 gravity;
        f32  spread_degrees; // half angle of the launch cone
    };

    // explosions, speed is in units per second
    struct particle_radial_props {
        f32 speed;
        b32 is_continuous; // spawns until stopped, instead of every particle once per start
    };

    //--------------------------------------------------------------------
    // PHYSICS 
    //--------------------------------------------------------------------

    static constexpr u32 PHYS_CLLSN_MASK_ALL = 0xFFFFFFFF;

    IFB_U32(phys_cllsn_phase_u32);
    enum phys_cllsn_phase_e {
       phys_cllsn_phase_e_begin = 0,
       phys_cllsn_phase_e_stay  = 1,
       phys_cllsn_phase_e_end   = 2
    };

    struct phys_cllsn_contact {
        entity_id            self;
        entity_id            other;
        vec3                 normal;
        f32                  depth;
        phys_cllsn_phase_u32 phase;
    };

    using fptr_phys_cllsn_proc = void (*) (const phys_cllsn_contact& contact);

    //--------------------------------------------------------------------
    // COMPONENTS
    //--------------------------------------------------------------------

    IFB_U32(cmpnt_type_u32);

    enum cmpnt_type_e {
        cmpnt_type_e_none             = 0,
        cmpnt_type_e_position         = bit_value(0),
        cmpnt_type_e_color            = bit_value(1),
        cmpnt_type_e_quad             = bit_value(2),
        cmpnt_type_e_velocity         = bit_value(3),
        cmpnt_type_e_acceleration     = bit_value(4),
        cmpnt_type_e_inv_mass         = bit_value(5),
        cmpnt_type_e_drag             = bit_value(6),
        cmpnt_type_e_term_velocity    = bit_value(7),
        cmpnt_type_e_spring           = bit_value(8),
        cmpnt_type_e_map_coords       = bit_value(9),
        cmpnt_type_e_particle_emitter = bit_value(10),
        cmpnt_type_e_sprite           = bit_value(11),
        cmpnt_type_e_immovable        = bit_value(12),
        cmpnt_type_e_bounds           = bit_value(13),
        cmpnt_type_e_collider         = bit_value(14)
    };

    using  cmpnt_position      = vec3;
    using  cmpnt_color         = color_rgba_u32;
    using  cmpnt_velocity      = vec3;
    using  cmpnt_acceleration  = vec3;
    using  cmpnt_term_velocity = vec3;

    struct cmpnt_quad {
        f32 width;
        f32 height;
    };

    struct cmpnt_inv_mass {
        f32 normal_val;
    };

    struct cmpnt_drag {
        f32 normal_val;
    };

    struct cmpnt_spring {
        entity_id id;
        entity_id anchor;
        f32       stiffness;
        f32       damping;
        f32       rest_length;
    };

    // the position component is the source of truth for where an entity is
    // h_map is the map the entity belongs to, the rest is the position
    // measured in tiles from the map origin: columns along x, levels along y, rows along z
    struct cmpnt_map_coords {
        hnd_map h_map;
        f32     col_x;
        f32     lvl_y;
        f32     row_z;
    };

    // the collision rect, for when the sprite doesn't fill its quad
    // it's in the quad's plane, measured from the position, which is the quad's center
    // x and y are the rect's center, so { 0, 0, quad width, quad height } covers the whole quad
    // the box is as deep along z as the rect is wide
    // entities without one collide with their quad
    struct cmpnt_bounds {
        f32 x;
        f32 y;
        f32 width;
        f32 height;
    };

    struct cmpnt_particle_emitter {
        hnd_particle_emitter hnd;
    };

    struct cmpnt_sprite {
        hnd_sprite_atlas h_atlas;
        u32              i_sprite;
    };

    struct cmpnt_collider {
        fptr_phys_cllsn_proc proc;
        u32                  mask_category;
        u32                  mask_collides_with;
        b32                  is_trigger;
    };

    //--------------------------------------------------------------------
    // ARCHETYPES
    //--------------------------------------------------------------------

    struct atype_sprite {
        cmpnt_position   position;
        cmpnt_color      color;
        cmpnt_quad       quad;
        cmpnt_map_coords map_coords;
        cmpnt_sprite     sprite;
    };

    struct atype_particle {
        cmpnt_position      position;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
    };

    struct atype_physics_sprite {
        cmpnt_position      position;
        cmpnt_color         color;
        cmpnt_quad          quad;
        cmpnt_velocity      velocity;
        cmpnt_acceleration  acceleration;
        cmpnt_inv_mass      inv_mass;
        cmpnt_drag          drag;
        cmpnt_term_velocity term_velocity;
        cmpnt_map_coords    map_coords;
        cmpnt_sprite        sprite;
    };

    static const cmpnt_type_u32 ARCHETYPE_SPRITE = (
        cmpnt_type_e_position   |
        cmpnt_type_e_color      |
        cmpnt_type_e_quad       |
        cmpnt_type_e_map_coords |
        cmpnt_type_e_sprite
    );

    static const cmpnt_type_u32 ARCHETYPE_PHYSICS_PARTICLE = (
        cmpnt_type_e_position      |
        cmpnt_type_e_velocity      |
        cmpnt_type_e_acceleration  |
        cmpnt_type_e_inv_mass      |
        cmpnt_type_e_drag          |
        cmpnt_type_e_term_velocity
    );

    static const cmpnt_type_u32 ARCHETYPE_PHYSICS_SPRITE = (
        ARCHETYPE_SPRITE |
        ARCHETYPE_PHYSICS_PARTICLE
    );

    struct archetype {

        cmpnt_type_u32 cmpnt_mask;

        archetype(void) = default;
        archetype(const cmpnt_type_u32 val) : cmpnt_mask(val) {}

        inline void reset(void) {
            cmpnt_mask = 0;
        }

        inline bool has_any  (const cmpnt_type_u32 types) const { return((cmpnt_mask & types) !=  0);    }
        inline bool has_all  (const cmpnt_type_u32 types) const { return((cmpnt_mask & types) == types); }
        inline bool has_none (const cmpnt_type_u32 types) const { return((cmpnt_mask & types) == 0);     }

        inline void
        add(const cmpnt_type_u32 types) {

            cmpnt_mask |= types;
        }

        inline void
        remove(const cmpnt_type_u32 types) {
            cmpnt_mask &= ~types.val;
        }
    };

    //--------------------------------------------------------------------
    // ENTITIES
    //--------------------------------------------------------------------

    constexpr u32 ENTITY_ID_INVALID = INVALID_U32;
    
    struct entity {
        const char* tag;
        entity_id   id;
        archetype   archetype;
        u32         index_sparse;
        u32         index_dense;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        cmpnt_position pos;
        cmpnt_sprite   sprite;
    };

    struct entity_query {
        cmpnt_type_u32 has_all;
        cmpnt_type_u32 has_any;
        cmpnt_type_u32 has_none;
    };

    
    //--------------------------------------------------------------------
    // EVENTS 
    //--------------------------------------------------------------------
    
    IFB_U32(event_type_u32);
    
    enum event_type_e {
        event_type_e_key_down          = 0,
        event_type_e_key_up            = 1,
        event_type_e_mouse             = 2,
        event_type_e_gamepad           = 3,
        event_type_e_physics_collision = 4, 
        event_type_e_count             = 5,
    };

    // sent once per contact per phase, after the physics step
    // a is the lower id, and the normal points from a to b
    struct event_data_physics_collision {
        entity_id            a;
        entity_id            b;
        vec3                 normal;
        f32                  depth;
        phys_cllsn_phase_u32 phase;
    };

    struct event_payload {
        union {
            byte                         data[64];
            event_data_physics_collision physics_collision;
        } data;
    };

    static_assert(sizeof(event_data_physics_collision) <= sizeof(event_payload::data), "the collision event doesn't fit in the payload");
    
    using fptr_event_listener = void (*) (const event_payload&);

    //--------------------------------------------------------------------
    // JOBS 
    //--------------------------------------------------------------------
    
    using job_proc = void (*) (const hnd_arena h_arena, void* ctx);
    
    // aligned so a job can cast the data to its own context struct
    struct alignas(16) job_ctx {
        byte data[64];
    };
    
    struct job {
        job_proc proc;  
        job_ctx  ctx;
    };

    //--------------------------------------------------------------------
    // AUDIO
    //--------------------------------------------------------------------

    // every voice is in a group, and the group's volume scales the voice's own
    enum audio_group_e {
        audio_group_e_sfx   = 0,
        audio_group_e_music = 1,
        audio_group_e_count = 2
    };

    IFB_U32(audio_group_u32);

    // interleaved samples from -1 to 1, only valid during the frame it was requested in
    // the format is the device's, config::audio_sample_rate and config::audio_channel_count
    struct audio_buffer {
        f32* samples;
        u32  frame_count;
    };

    struct audio_info {
        bool is_device_ready;
        u32  frames_queued;
        u32  frames_queued_min;
        u32  underrun_count;
        f32  frame_gap_ms;
        f32  frame_gap_ms_max;
        u32  voice_count;
        u32  preload_count;
        u32  stream_count;
        u32  stream_chunk_reads; // since the last reset
        u32  stream_underruns;   // frames a stream waited for its chunks, since the last reset
        u32  output_clips;       // samples that went past -1 to 1 and were clamped, since the last reset
    };

    // the position is in frames of the source, which may not be at the device's sample rate
    struct audio_voice_info {
        f64              position;
        u32              frame_count;
        u32              sample_rate;
        f32              volume;
        f32              gain;               // what it's playing at now, its volume times its group's, partway there while it fades
        audio_group_u32  group;
        bool             is_streaming;
        bool             is_paused;
        bool             is_looping;
        bool             is_stopping; // fading out, it's freed when the fade is done
        cchar            name[config::asset_name_size + 1]; // the sound asset, empty for a wav that isn't one
    };

    //--------------------------------------------------------------------
    // SCENES 
    //--------------------------------------------------------------------

    IFB_HANDLE(hnd_scene);

    // set on a scene when it is pushed, they decide what happens to the scenes under it
    IFB_U32(scene_flags_u32);

    enum scene_flag_e {
        scene_flag_e_none         = 0,
        scene_flag_e_update_below = bit_value(0), // the scenes under this one keep updating
        scene_flag_e_render_below = bit_value(1)  // the scenes under this one keep rendering
    };
    
    //--------------------------------------------------------------------
    // FONT 
    //--------------------------------------------------------------------

    // how to bake a font from a ttf
    // one line of wrapped text, a range of the string that was wrapped
    // length counts the characters drawn, the spaces or newline the line broke on aren't in it
    struct font_line {
        u32 start;
        u32 length;
        f32 width;
    };

    struct font_config {
        f32 pixel_height    = 16.0f; // for pixel fonts, a multiple of the size they were designed at (8 for Press Start 2P)
        u32 first_codepoint = 32;    // ' '
        u32 glyph_count     = 95;    // ' ' to '~', the printable ascii characters
        u32 oversample_x    = 1;     // 1 to 8, 1 for pixel fonts, 2 or more makes small smooth fonts look better
        u32 oversample_y    = 1;
    };

    // one glyph's spot in the font's atlas and how to place it, matching stbtt_packedchar
    //
    // with oversampling the glyph is drawn into the atlas bigger than it appears on
    // screen, so the atlas rect and the quad are different sizes. always size the
    // quad with quad_x0 to quad_x1 and quad_y0 to quad_y1, not with the atlas rect
    struct font_glyph {
        u16 atlas_x0;  // the glyph's rect in the atlas, in atlas pixels, x1 and y1 are exclusive
        u16 atlas_y0;
        u16 atlas_x1;
        u16 atlas_y1;
        f32 quad_x0;   // the quad to draw, in screen pixels from the pen position (on the baseline)
        f32 quad_y0;   // negative is above the baseline
        f32 quad_x1;
        f32 quad_y1;
        f32 advance;   // how far the pen moves right after this glyph, before kerning
    };

    // the space to add between two glyphs, on top of the first one's advance
    struct font_kern_pair {
        u32 key;     // font::kern_key(first, second)
        f32 amount;  // in screen pixels, usually negative (the pair moves closer)
    };

    // a font baked at one pixel height when the asset database is built, so
    // loading it is a copy and nothing is parsed at runtime
    //
    // the blob is this struct, then the glyphs, then the kerning pairs, then the
    // atlas pixels. the offsets are from the start of the struct rather than
    // pointers, so the blob works wherever it's loaded without being fixed up
    //
    // to draw a string, start the pen at (x, baseline), and for each codepoint:
    //   draw glyph(c)'s quad at the pen, textured with its atlas rect
    //   pen x += glyph(c)->advance + kerning(c, next c)
    struct font {
        u32 size;              // the whole blob in bytes, this struct included
        f32 pixel_height;      // the height it was baked at
        f32 ascent;            // baseline to the top of the tallest glyph
        f32 descent;           // baseline to the bottom of the lowest glyph, negative
        f32 line_gap;          // extra space between lines
        f32 line_height;       // ascent - descent + line_gap, how far to move down for a new line
        u32 oversample_x;      // atlas pixels per screen pixel, 1 for none (pixel fonts)
        u32 oversample_y;      // above 1, draw the atlas with linear filtering, at 1 nearest is fine
        u32 first_codepoint;   // the glyphs cover first_codepoint to first_codepoint + glyph_count - 1
        u32 glyph_count;
        u32 glyph_offset;      // glyph_count font_glyphs
        u32 kern_pair_count;   // 0 for fonts without kerning, like monospaced ones
        u32 kern_pair_offset;  // kern_pair_count font_kern_pairs, sorted by key
        u32 atlas_width;
        u32 atlas_height;
        u32 atlas_offset;      // atlas_width * atlas_height bytes, one coverage value (0 to 255) per pixel

        // the codepoints as glyph indexes (codepoint - first_codepoint), first in the high 16 bits
        static inline u32
        kern_key(const u32 index_first, const u32 index_second) {
            return((index_first << 16) | (index_second & 0xFFFF));
        }

        inline const font_glyph*
        glyphs(void) const {
            return((const font_glyph*)(((const byte*)this) + glyph_offset));
        }

        inline const font_kern_pair*
        kern_pairs(void) const {
            return((const font_kern_pair*)(((const byte*)this) + kern_pair_offset));
        }

        inline const byte*
        atlas_pixels(void) const {
            return(((const byte*)this) + atlas_offset);
        }

        // NULL if the font doesn't have the codepoint
        inline const font_glyph*
        glyph(const u32 codepoint) const {
            const u32 index = codepoint - first_codepoint; // wraps below first_codepoint
            return((index < glyph_count) ? &glyphs()[index] : NULL);
        }

        // 0 if the pair isn't kerned or either codepoint isn't in the font
        inline f32
        kerning(const u32 codepoint_first, const u32 codepoint_second) const {

            const u32 index_first  = codepoint_first  - first_codepoint;
            const u32 index_second = codepoint_second - first_codepoint;
            if (index_first >= glyph_count || index_second >= glyph_count) return(0.0f);

            // binary search the sorted pairs
            const u32             key   = kern_key(index_first, index_second);
            const font_kern_pair* pairs = kern_pairs();
            u32 lo = 0;
            u32 hi = kern_pair_count;
            while (lo < hi) {
                const u32 mid = lo + ((hi - lo) / 2);
                if      (pairs[mid].key < key) lo = mid + 1;
                else if (pairs[mid].key > key) hi = mid;
                else return(pairs[mid].amount);
            }
            return(0.0f);
        }
    };

    // a font blob as an asset, fnt is the start of the blob and the rest of it follows the struct
    // the blob only has offsets, so the asset can move in memory without fixing any pointers,
    // and it's stored in the database exactly as it's laid out in memory
    struct asset_font_atlas {
        hnd_asset hnd;
        font      fnt; // has to be last, fnt.size bytes start here
    };

    // the bytes in front of the blob, so the asset in memory is this plus fnt.size
    static constexpr u32 ASSET_FONT_ATLAS_HEADER_SIZE = offsetof(asset_font_atlas, fnt);
};

#endif //IFB_HPP
