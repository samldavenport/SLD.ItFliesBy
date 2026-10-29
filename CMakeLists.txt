cmake_minimum_required(VERSION 3.21)

project(ItFliesBy VERSION 0.1.0 LANGUAGES CXX)

# exceptions are disabled, so drop the /EHsc that cmake adds by default
if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

#--------------------------------------------------------------------
# DEPENDENCIES
#--------------------------------------------------------------------

# the sld libraries are submodules
# SLD.Core goes first so the others use it instead of their own copies
add_subdirectory(SLD.Core)
add_subdirectory(SLD.Strings)
add_subdirectory(SLD.OpenGL)
add_subdirectory(SLD.Math)

# everything else comes from vcpkg (see vcpkg.json)
find_package(imgui     CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(Stb       REQUIRED)

#--------------------------------------------------------------------
# CORE
#--------------------------------------------------------------------

# common structures shared by the engine and the game (header only)
file(GLOB IFB_CORE_HEADERS CONFIGURE_DEPENDS IFB.Core/include/*.hpp)

add_library(ifb_core INTERFACE)
add_library(IFB::Core ALIAS ifb_core)

target_sources(ifb_core INTERFACE ${IFB_CORE_HEADERS})

# ifb.hpp includes the sld, imgui, and stb headers, so consumers need them too
target_include_directories(ifb_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/IFB.Core/include
    ${Stb_INCLUDE_DIR}
)
target_link_libraries(ifb_core INTERFACE
    SLD::Core SLD::OpenGL SLD::Math SLD::Strings imgui::imgui
)

#--------------------------------------------------------------------
# ENGINE
#--------------------------------------------------------------------

set(IFB_ENGINE_SOURCE_DIRS
    IFB.Engine/src/assets
    IFB.Engine/src/collections
    IFB.Engine/src/components
    IFB.Engine/src/engine
    IFB.Engine/src/entities
    IFB.Engine/src/files
    IFB.Engine/src/json
    IFB.Engine/src/map
    IFB.Engine/src/memory
    IFB.Engine/src/particle
    IFB.Engine/src/physics
    IFB.Engine/src/quads
    IFB.Engine/src/random
    IFB.Engine/src/renderer
    IFB.Engine/src/sprites
    IFB.Engine/src/tiled
    IFB.Engine/src/win32
)

# unity build: eng.cpp includes the other source files
set(IFB_ENGINE_UNITY_GLOBS "")
foreach(DIR ${IFB_ENGINE_SOURCE_DIRS})
    list(APPEND IFB_ENGINE_UNITY_GLOBS ${DIR}/*.cpp ${DIR}/*.hpp)
endforeach()
file(GLOB IFB_ENGINE_UNITY_SOURCES CONFIGURE_DEPENDS ${IFB_ENGINE_UNITY_GLOBS})
file(GLOB IFB_HEADERS              CONFIGURE_DEPENDS IFB.Engine/include/*.hpp)
list(REMOVE_ITEM IFB_ENGINE_UNITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/IFB.Engine/src/engine/eng.cpp
)

add_library(ifb_engine SHARED)
add_library(IFB::Engine ALIAS ifb_engine)

target_sources(ifb_engine PRIVATE
    IFB.Engine/src/engine/eng.cpp
    ${IFB_ENGINE_UNITY_SOURCES}
    ${IFB_HEADERS}
)
set_source_files_properties(${IFB_ENGINE_UNITY_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)

target_include_directories(ifb_engine
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/IFB.Engine/include
    PRIVATE ${IFB_ENGINE_SOURCE_DIRS}
            ${Stb_INCLUDE_DIR}
)

target_link_libraries(ifb_engine
    PUBLIC  IFB::Core
    PRIVATE rapidjson user32 gdi32
)

#--------------------------------------------------------------------
# GUI
#--------------------------------------------------------------------

# unity build: gui.cpp includes the other source files
file(GLOB IFB_GUI_UNITY_SOURCES CONFIGURE_DEPENDS IFB.GUI/src/*.cpp)
list(REMOVE_ITEM IFB_GUI_UNITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/IFB.GUI/src/gui.cpp
)

add_library(ifb_gui STATIC)
add_library(IFB::GUI ALIAS ifb_gui)

target_sources(ifb_gui PRIVATE
    IFB.GUI/src/gui.cpp
    ${IFB_GUI_UNITY_SOURCES}
)
set_source_files_properties(${IFB_GUI_UNITY_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)

target_include_directories(ifb_gui PRIVATE
    IFB.GUI/src
    ${Stb_INCLUDE_DIR}
)

target_link_libraries(ifb_gui PUBLIC IFB::Engine)

#--------------------------------------------------------------------
# GAME
#--------------------------------------------------------------------

# unity build: game-win32-main.cpp includes the other source files
file(GLOB IFB_GAME_UNITY_SOURCES CONFIGURE_DEPENDS IFB.Game/src/*.cpp)
list(REMOVE_ITEM IFB_GAME_UNITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/IFB.Game/src/game-win32-main.cpp
)

add_executable(ifb WIN32)

target_sources(ifb PRIVATE
    IFB.Game/src/game-win32-main.cpp
    ${IFB_GAME_UNITY_SOURCES}
)
set_source_files_properties(${IFB_GAME_UNITY_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)

target_include_directories(ifb PRIVATE
    IFB.Game/src
    ${Stb_INCLUDE_DIR}
)

target_link_libraries(ifb PRIVATE IFB::Engine IFB::GUI user32 kernel32)

#--------------------------------------------------------------------
# COMMON
#--------------------------------------------------------------------

set_target_properties(ifb_engine PROPERTIES OUTPUT_NAME "ItFliesBy.Engine")
set_target_properties(ifb_gui    PROPERTIES OUTPUT_NAME "ItFliesBy.GUI")
set_target_properties(ifb        PROPERTIES OUTPUT_NAME "ItFliesBy")

foreach(TARGET ifb_engine ifb_gui ifb)
    target_compile_features(${TARGET} PUBLIC cxx_std_17)
    set_target_properties(${TARGET} PROPERTIES
        CXX_EXTENSIONS           OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )

    target_compile_definitions(${TARGET} PRIVATE _HAS_EXCEPTIONS=0)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${TARGET} PRIVATE /EHs-)
    endif()
endforeach()

#--------------------------------------------------------------------
# ASSETS
#--------------------------------------------------------------------

# the engine loads shaders, json, and the asset database from the working directory, so they sit next to the exe
file(GLOB IFB_ASSETS CONFIGURE_DEPENDS
    assets/shaders/*.glsl
    assets/json/*.json
    assets/*.ifbdb
)

set(IFB_ASSETS_OUT "")
foreach(ASSET ${IFB_ASSETS})
    get_filename_component(ASSET_NAME ${ASSET} NAME)
    set(ASSET_OUT "${CMAKE_BINARY_DIR}/bin/${ASSET_NAME}")
    add_custom_command(
        OUTPUT  ${ASSET_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSET} ${ASSET_OUT}
        DEPENDS ${ASSET}
        COMMENT "Copying ${ASSET_NAME}"
    )
    list(APPEND IFB_ASSETS_OUT ${ASSET_OUT})
endforeach()

add_custom_target(ifb_assets ALL DEPENDS ${IFB_ASSETS_OUT})
add_dependencies(ifb ifb_assets)
