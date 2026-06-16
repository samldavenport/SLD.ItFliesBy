#ifndef IFB_HPP
#define IFB_HPP

#include <sld.hpp>
#include <sld-opengl.hpp>
#include <sld-math.hpp>

#include "ifb-config.hpp"
#include "ifb-platform.hpp"
#include "ifb-engine.hpp"
#include "ifb-input.hpp"

#define IFB_INTERNAL static

#ifdef IFB_STATIC
#   define IFB_API
#elif defined(IFB_API_DLL_EXPORT)
#   define IFB_API __declspec(dllexport)
#else
#   define IFB_API __declspec(dllimport)
#endif


using namespace sld;

namespace ifb {

    struct sparse_array;
    struct quad;
    struct dimensions_2d;
    struct dimensions_3d;

    using position_3d = vec3;

    constexpr u32 INVALID_INDEX   = 0xFFFFFFFF;
    constexpr u32 INVALID_HASH_32 = 0xFFFFFFFF;
    constexpr u32 QUAD_DATA_SIZE  = sizeof(vec3) + sizeof(color_rgba_f32);

    IFB_API u32           sparse_array_memory_requirement (const u32 capacity, const u32 val_size, const f32 max_load_p100);
    IFB_API sparse_array* sparse_array_memory_init        (const u32 capacity, const u32 val_size, const f32 max_load_p100, const u32 key_size, const u32 mem_size, void* mem_ptr);
    IFB_API bool          sparse_array_is_valid           (const sparse_array* sa);
    IFB_API void          sparse_array_assert_valid       (const sparse_array* sa);
    IFB_API void*         sparse_array_lookup             (const sparse_array* sa, const cchar8* key);
    IFB_API bool          sparse_array_insert             (sparse_array* sa, const cchar8* key, const void* val);
    IFB_API bool          sparse_array_remove             (sparse_array* sa, const cchar8* key);

    struct dimensions_2d {
        u32 width;
        u32 height;
    };

    struct dimensions_3d {
        u32 width;
        u32 height;
        u32 depth;
    };

    struct quad {
        position_3d    position;
        dimensions_2d  dimensions;
        color_rgba_u32 color;
    };
};

#endif //IFB_HPP