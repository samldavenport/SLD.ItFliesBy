#ifndef IFB_RENDERER_HPP
#define IFB_RENDERER_HPP

#include <sld-opengl.hpp>
#include "ifb-files.hpp"

#include "ifb-types.hpp"
#include "ifb.hpp"
#include "ifb-quad.hpp"
#include "ifb-map.hpp"
#include "ifb-memory.hpp"
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

};

#endif //IFB_RENDERER_HPP
