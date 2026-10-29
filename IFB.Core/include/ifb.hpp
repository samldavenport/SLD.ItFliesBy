#ifndef IFB_HPP
#define IFB_HPP

#include <imgui.h>

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>

#ifdef IFB_STATIC
#   define IFB_API
#elif defined(IFB_API_DLL_EXPORT)
#   define IFB_API __declspec(dllexport)
#else
#   define IFB_API __declspec(dllimport)
#endif

// defined here instead of ifb-engine.hpp so the engine headers it includes (like eng-collections.hpp) can use it
#ifdef IFB_ENGINE_STATIC
#   define IFB_ENGINE_API
#elif defined(IFB_ENGINE_DLL_EXPORT)
#   define IFB_ENGINE_API __declspec(dllexport)
#else
#   define IFB_ENGINE_API __declspec(dllimport)
#endif

using namespace sld;

#define IFB_U32(type)     struct type : sld::strong_u32 { using strong_u32::strong_u32; };
#define IFB_U16(type)     struct type : sld::strong_u16 { using strong_u16::strong_u16; };
#define IFB_U8(type)      struct type : sld::strong_u8  { using strong_u8::strong_u8;   };
#define IFB_FLAGS32(type) struct type : flags           { using flags;                  };
#define IFB_HANDLE(hnd)   struct hnd  : handle          { using handle::handle;         };

#define IFB_INTERNAL static
#define IFB_INLINE   inline

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
    // STRONG TYPES 
    //--------------------------------------------------------------------

    IFB_U32(entity_id);
    
    IFB_U32(handle);

    IFB_HANDLE(hnd_arena);
    IFB_HANDLE(hnd_asset);
    IFB_HANDLE(hnd_file);
    IFB_HANDLE(hnd_map);
    IFB_HANDLE(hnd_particle_emitter);
    IFB_HANDLE(hnd_sprite_atlas);
    IFB_HANDLE(hnd_sprite);
    
    //--------------------------------------------------------------------
    // MISC TYPES 
    //--------------------------------------------------------------------
    
    struct text {
        cchar* cstr;
        u32    size;
    };

    struct dimensions_2d {
        f32 width;
        f32 height;
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
    using  cmpnt_term_velocity = vec3;
    using  cmpnt_acceleration  = vec3;

    struct cmpnt_inv_mass {
        f32 normal_val;
    };

    struct cmpnt_drag {
        f32 normal_val;
    };

    struct cmpnt_quad {
        f32 width;
        f32 height;
    };

    struct cmpnt_spring {
        entity_id id;
        entity_id anchor;
        f32       stiffness;
        f32       damping;
        f32       rest_length;
    };
    
    struct cmpnt_particle_emitter {
        hnd_particle_emitter hnd;
    };

    struct cmpnt_sprite {
        hnd_sprite_atlas h_atlas;
        u32              i_sprite;
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

    struct entity_list;


    //--------------------------------------------------------------------
    // PARTICLES 
    //--------------------------------------------------------------------
    
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

    enum particle_emitter_type_e {
        particle_emitter_type_e_parabolic = 0,
        particle_emitter_type_e_radial    = 1,
        particle_emitter_type_e_count     = 2
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
    
};


#endif //IFB_HPP
