#pragma once

#define IFB_ENGINE_DLL_EXPORT
#define IFB_API_DLL_EXPORT

// only the engine uses stb_image, so its implementation lives here; undefined
// afterwards so the files that include stb_image.h only get the declarations
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

#include "asset.cpp"

#include "collections.cpp"

#include "component-manager.cpp"
#include "component-tables.cpp"

#include "eng-arena.cpp"
#include "eng-asset.cpp"
#include "eng-atype.cpp"
#include "eng-cmpnt.cpp"
#include "eng-camera.cpp"
#include "eng-context.cpp"
#include "eng-entity.cpp"
#include "eng-input.cpp"
#include "eng-window.cpp"
#include "eng-stack.cpp"
#include "eng-file.cpp"
#include "eng-image.cpp"
#include "eng-system.cpp"
#include "eng-text.cpp"
#include "eng-keycode.cpp"
#include "eng-map.cpp"
#include "eng-particle.cpp"
#include "eng-random.cpp"
#include "eng-renderer.cpp"
#include "eng-sprite.cpp"

#include "entity.cpp"
#include "entity-component.cpp"
#include "entity-lookup.cpp"
#include "entity-manager.cpp"
#include "entity-sparse-set.cpp"
#include "entity-testing.cpp"

#include "file-manager.cpp"
#include "file-common.cpp"
// #include "file-ro.cpp"
#include "file-wo.cpp"
#include "file-rw.cpp"

#include "json.cpp"

#include "map-mngr.cpp"
#include "map.cpp"
#include "map-chunk.cpp"

#include "memory-manager.cpp"
#include "memory-arena.cpp"
#include "memory-block.cpp"
#include "memory-reservation.cpp"

#include "particle-mngr.cpp"
#include "particle-emitter.cpp"
#include "particle-update.cpp"
#include "particle-render.cpp"
#include "particle-entity.cpp"

#include "physics-manager.cpp"
#include "physics-force-accumulator.cpp"
#include "physics-entity.cpp"
#include "physics-integrator.cpp"
#include "physics-spring.cpp"
#include "physics-aabb.cpp"
#include "physics-bvh.cpp"
#include "physics-collision-detection.cpp"
#include "physics-collision-list.cpp"
#include "physics-collision-resolver.cpp"

#include "quad.cpp"
#include "quad-tests.cpp"

#include "random-table.cpp"
#include "renderer.cpp"

#include "sprite.cpp"

#include "tiled.cpp"



#include "win32.cpp"


