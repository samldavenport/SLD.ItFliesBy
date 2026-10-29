#ifndef IFB_HPP
#define IFB_HPP

#include <imgui.h>

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

// defined here instead of ifb-engine.hpp so the headers it includes can use it
#ifdef IFB_ENGINE_STATIC
#   define IFB_ENGINE_API
#elif defined(IFB_ENGINE_DLL_EXPORT)
#   define IFB_ENGINE_API __declspec(dllexport)
#else
#   define IFB_ENGINE_API __declspec(dllimport)
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

    constexpr u32 INVALID_U16       = 0xFFFF;
    constexpr u32 INVALID_U32       = 0xFFFFFFFF;
    constexpr u32 INVALID_INDEX     = INVALID_U32;
    constexpr u32 INVALID_HASH_32   = INVALID_U32;
    constexpr u32 INVALID_HANDLE    = INVALID_U32;
    constexpr u32 INVALID_ID        = INVALID_U32;
    constexpr f32 POS_MAX           =  1.0f;
    constexpr f32 POS_MIN           = -1.0f;

    //--------------------------------------------------------------------
    // HANDLES & IDS
    //--------------------------------------------------------------------

    IFB_U32(handle);
    IFB_U32(entity_id);

    IFB_HANDLE(hnd_arena);
    IFB_HANDLE(hnd_asset);
    IFB_HANDLE(hnd_file);
    IFB_HANDLE(hnd_map);
    IFB_HANDLE(hnd_particle_emitter);
    IFB_HANDLE(hnd_sprite_atlas);
    IFB_HANDLE(hnd_sprite);

    //--------------------------------------------------------------------
    // COMMON TYPES
    //--------------------------------------------------------------------

    using pixel      = color_rgba_u32;
    using origin_3d  = vec3;

    struct text {
        cchar* cstr;
        u32    size;
    };

    struct file_path {
        cchar cstr[IFB_CONFIG_FILE_PATH_SIZE];
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
        asset_type_e_count         = 3
    };

    struct sprite_index;

    struct asset_name {
        cchar cstr [IFB_CONFIG_ASSET_NAME_SIZE];

        inline void
        init(
            const cchar* cstr) {

            assert(cstr);

            (void)cstr_nvar_copy(
                cstr,
                IFB_CONFIG_ASSET_NAME_SIZE,
                this->cstr,
                IFB_CONFIG_ASSET_NAME_SIZE
            );
        }

        inline hnd_asset
        to_handle(void) {

            const hnd_asset hnd = hash_u32((void*)cstr, IFB_CONFIG_ASSET_NAME_SIZE);
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

    struct asset_database_info {
        cchar     name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
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

    struct sprite_atlas_info {
        u32 width;
        u32 height;
        u32 sprite_count;
    };

    struct sprite_atlas_sprite_info {
        cchar      name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
        hnd_sprite hnd;
        u32        x;
        u32        y;
        u32        width;
        u32        height;
    };

    struct sprite_atlas_builder_info {
        cchar     name [IFB_CONFIG_ASSET_NAME_SIZE + 1];
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
    // COMPONENTS
    //--------------------------------------------------------------------

    IFB_U32(component_type);

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
        cmpnt_type_e_sprite           = bit_value(11)
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

    struct cmpnt_particle_emitter {
        hnd_particle_emitter hnd;
    };

    struct cmpnt_sprite {
        hnd_sprite_atlas h_atlas;
        u32              i_sprite;
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

    //--------------------------------------------------------------------
    // ENTITIES
    //--------------------------------------------------------------------

    constexpr u32 ENTITY_ID_INVALID = INVALID_U32;
    constexpr u32 ENTITY_TAG_SIZE   = 16;

    static const component_type ENTITY_ARCHETYPE_SPRITE = (
        cmpnt_type_e_position   |
        cmpnt_type_e_color      |
        cmpnt_type_e_quad       |
        cmpnt_type_e_map_coords |
        cmpnt_type_e_sprite
    );

    static const component_type ENTITY_ARCHETYPE_PHYSICS_PARTICLE = (
        cmpnt_type_e_position      |
        cmpnt_type_e_velocity      |
        cmpnt_type_e_acceleration  |
        cmpnt_type_e_inv_mass      |
        cmpnt_type_e_drag          |
        cmpnt_type_e_term_velocity
    );

    static const component_type ENTITY_ARCHETYPE_PHYSICS_SPRITE = (
        ENTITY_ARCHETYPE_SPRITE |
        ENTITY_ARCHETYPE_PHYSICS_PARTICLE
    );

    struct entity_archetype {

        component_type cmpnt_mask;

        entity_archetype(void) = default;
        entity_archetype(const component_type val) : cmpnt_mask(val) {}

        inline void reset(void) {
            cmpnt_mask = 0;
        }

        inline bool has_any  (const component_type types) const { return((cmpnt_mask & types) !=  0);    }
        inline bool has_all  (const component_type types) const { return((cmpnt_mask & types) == types); }
        inline bool has_none (const component_type types) const { return((cmpnt_mask & types) == 0);     }

        inline void
        add(const component_type types) {

            cmpnt_mask |= types;
        }

        inline void
        remove(const component_type types) {
            cmpnt_mask &= ~types.val;
        }
    };

    struct entity {
        const char*      tag;
        entity_id        id;
        entity_archetype archetype;
        u32              index_sparse;
        u32              index_dense;
    };

    struct quad_entity : entity {
        color_rgba_u32 color;
        dimensions_2d  dims;
        cmpnt_position pos;
        cmpnt_sprite   sprite;
    };

    struct entity_query {
        component_type has_all;
        component_type has_any;
        component_type has_none;
    };

    struct tag {
        cchar cstr[16];

        tag(const cchar* cstr) {
            memset((void*)this->cstr, 0, ENTITY_TAG_SIZE);
            const u32 len = strnlen_s(cstr, ENTITY_TAG_SIZE);
            (void)strncpy_s(this->cstr, cstr, len);
        }
        inline void
        init(
            const cchar* tag_cstr)  {
            assert(cstr != NULL);
            (void)strncpy_s(cstr, 16, tag_cstr, 16);
        }

        inline u32
        hash(void) {
            return(hash_u32((void*)cstr, 16));
        }
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

    struct event_payload {
        byte data[64];
    };
    
    using fptr_event_listener = void * (event_payload*);
};

#endif //IFB_HPP
