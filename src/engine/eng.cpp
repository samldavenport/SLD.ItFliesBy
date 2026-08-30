#pragma once

#define IFB_ENGINE_DLL_EXPORT
#define IFB_API_DLL_EXPORT

#include "collection-array-list.cpp"
#include "collection-sparse-set.cpp"
#include "collection-stack.cpp"
#include "collection-entity-list.cpp"
#include "collection-component-table.cpp"
#include "collection-index-cache.cpp"

#include "component-manager.cpp"
#include "component-tables.cpp"

#include "eng-arena.cpp"
#include "eng-camera.cpp"
#include "eng-context.cpp"
#include "eng-entity.cpp"
#include "eng-input.cpp"
#include "eng-window.cpp"
#include "eng-stack.cpp"
#include "eng-gui.cpp"
#include "eng-file.cpp"
#include "eng-image.cpp"
#include "eng-system.cpp"
#include "eng-keycode.cpp"
#include "eng-tile-map.cpp"

#include "entity.cpp"
#include "entity-component.cpp"
#include "entity-lookup.cpp"
#include "entity-manager.cpp"
#include "entity-sparse-set.cpp"
#include "entity-testing.cpp"

#include "file-manager.cpp"
#include "file-common.cpp"
#include "file-ro.cpp"
#include "file-wo.cpp"
#include "file-rw.cpp"

#include "gui.cpp"

#include "json.cpp"

#include "memory-manager.cpp"
#include "memory-arena.cpp"
#include "memory-block.cpp"

#include "physics-manager.cpp"
#include "physics-accumulator.cpp"
#include "physics-entity.cpp"
#include "physics-integrator.cpp"
#include "physics-spring.cpp"

#include "quad.cpp"
#include "quad-manager.cpp"
#include "quad-tests.cpp"

#include "renderer.cpp"

#include "tiled.cpp"

#include "tile.cpp"
#include "tile-mngr.cpp"
#include "tile-map.cpp"

#include "win32-file.cpp"
#include "win32-graphics.cpp"
#include "win32-input.cpp"
#include "win32-memory.cpp"
#include "win32-monitor.cpp"
#include "win32-system.cpp"
#include "win32-window.cpp"
