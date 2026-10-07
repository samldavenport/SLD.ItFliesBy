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
# ENGINE
#--------------------------------------------------------------------

set(IFB_ENGINE_SOURCE_DIRS
    src/collections
    src/components
    src/engine
    src/entities
    src/files
    src/json
    src/map
    src/memory
    src/particle
    src/physics
    src/quads
    src/renderer
    src/tiled
    src/win32
)

# unity build: eng.cpp includes the other source files
set(IFB_ENGINE_UNITY_GLOBS "")
foreach(DIR ${IFB_ENGINE_SOURCE_DIRS})
    list(APPEND IFB_ENGINE_UNITY_GLOBS ${DIR}/*.cpp ${DIR}/*.hpp)
endforeach()
file(GLOB IFB_ENGINE_UNITY_SOURCES CONFIGURE_DEPENDS ${IFB_ENGINE_UNITY_GLOBS})
file(GLOB IFB_HEADERS              CONFIGURE_DEPENDS include/*.hpp)
list(REMOVE_ITEM IFB_ENGINE_UNITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/eng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/win32-main.cpp
)

add_library(ifb_engine SHARED)
add_library(IFB::Engine ALIAS ifb_engine)

target_sources(ifb_engine PRIVATE
    src/engine/eng.cpp
    ${IFB_ENGINE_UNITY_SOURCES}
    ${IFB_HEADERS}
)
set_source_files_properties(${IFB_ENGINE_UNITY_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)

target_include_directories(ifb_engine
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${IFB_ENGINE_SOURCE_DIRS}
            ${Stb_INCLUDE_DIR}
)

# ifb.hpp includes the sld and imgui headers, so consumers need them too
target_link_libraries(ifb_engine
    PUBLIC  SLD::Core SLD::OpenGL SLD::Math imgui::imgui
    PRIVATE SLD::Strings rapidjson user32 gdi32
)

#--------------------------------------------------------------------
# WIN32
#--------------------------------------------------------------------

# unity build: win32-main.cpp includes the other source files
file(GLOB IFB_WIN32_UNITY_SOURCES CONFIGURE_DEPENDS
    src/collections/*.cpp
    src/game/*.cpp
    src/gui/*.cpp
)

add_executable(ifb WIN32)

target_sources(ifb PRIVATE
    src/win32/win32-main.cpp
    ${IFB_WIN32_UNITY_SOURCES}
)
set_source_files_properties(${IFB_WIN32_UNITY_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)

target_include_directories(ifb PRIVATE
    src/collections
    src/game
    src/gui
    src/win32
    ${Stb_INCLUDE_DIR}
)

target_link_libraries(ifb PRIVATE IFB::Engine user32 kernel32)

#--------------------------------------------------------------------
# COMMON
#--------------------------------------------------------------------

set_target_properties(ifb_engine PROPERTIES OUTPUT_NAME "ItFliesBy.Engine")
set_target_properties(ifb        PROPERTIES OUTPUT_NAME "ItFliesBy")

foreach(TARGET ifb_engine ifb)
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

# the engine loads shaders and json from the working directory, so they sit next to the exe
file(GLOB IFB_ASSETS CONFIGURE_DEPENDS
    assets/shaders/*.glsl
    assets/json/*.json
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
