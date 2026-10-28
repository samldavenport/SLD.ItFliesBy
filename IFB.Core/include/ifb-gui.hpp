#ifndef GUI_HPP
#define GUI_HPP

#include "ifb.hpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr f32 GUI_DRAG_FLOAT_SPEED =  0.001f;
    static constexpr f32 GUI_DRAG_FLOAT_MIN   = -1.000f;
    static constexpr f32 GUI_DRAG_FLOAT_MAX   =  1.000f;

    //--------------------------------------------------------------------
    // STRUCTURED TYPES
    //--------------------------------------------------------------------

    struct gui_strings;
    struct game_context;

    //--------------------------------------------------------------------
    // METHODS
    //--------------------------------------------------------------------

    // context (gui_open and gui_render are called by the game, so they're not internal)
                 void  gui_open               (void);
    IFB_INTERNAL void  gui_close              (void);
                 void  gui_render             (const game_context* game_ctx);
    IFB_INTERNAL void  gui_main_window        (const game_context* game_ctx);

    // engine
    IFB_INTERNAL void  gui_engine_imgui_demo             (bool& is_open);
    IFB_INTERNAL void  gui_engine_system                 (bool& is_open);
    IFB_INTERNAL void  gui_engine_particle_system        (bool& is_open);
    IFB_INTERNAL void  gui_engine_sprite_atlas_builder   (bool& is_open);
    IFB_INTERNAL void  gui_engine_sprite_atlas_inspector (bool& is_open);

    // game
    IFB_INTERNAL void  gui_game_player_rig       (bool& is_open, const game_context* game_ctx);
    IFB_INTERNAL void  gui_game_asset_dictionary (bool& is_open);
    IFB_INTERNAL void  gui_game_asset_database   (bool& is_open);

    // entities
    IFB_INTERNAL void  gui_entity_manager     (bool& is_open);
    IFB_INTERNAL void  gui_quad_manager       (bool& is_open);

    // renderer
    IFB_INTERNAL void  gui_renderer_camera    (bool& is_open);


    // controls
    IFB_INTERNAL void  gui_control_vec3       (const char* name, vec3& v);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    struct gui_strings {
        inline static const char* menu_engine                                        = "Engine";
        inline static const char* menu_engine_item_imgui_demo                        = "ImGui Demo";
        inline static const char* menu_engine_item_system                            = "System";
        inline static const char* menu_engine_menu_sprite_tools                      = "Sprite Tools";
        inline static const char* menu_engine_menu_sprite_tools_item_atlas_builder   = "Sprite Atlas Builder";
        inline static const char* menu_engine_menu_sprite_tools_item_atlas_inspector = "Sprite Atlas Inspector";
        inline static const char* menu_engine_item_particle_system                   = "Particle System";
        inline static const char* menu_entities                                      = "Entities";
        inline static const char* menu_entity_item_manager                           = "Entity Manager";
        inline static const char* menu_renderer                                      = "Renderer";
        inline static const char* menu_renderer_item_camera                          = "Camera";
        inline static const char* menu_renderer_item_grid                            = "Grid";
        inline static const char* window_renderer_camera                             = "Renderer Camera";
        inline static const char* camera_origin                                      = "Camera Origin";
        inline static const char* camera_target                                      = "Camera Target";
        inline static const char* menu_entity_item_quad_mngr                         = "Quad Manager";
        inline static const char* menu_game                                          = "Game";
        inline static const char* menu_game_item_player_rig                          = "Player Rig";
        inline static const char* menu_game_menu_assets                              = "Assets";
        inline static const char* menu_game_menu_assets_item_dictionary              = "Asset Dictionary";
        inline static const char* menu_game_menu_assets_item_database                = "Asset Database";
    } static * _gui_str;
};

#endif //GUI_HPP
