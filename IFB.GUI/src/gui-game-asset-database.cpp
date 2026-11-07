#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include "gui-game-asset-dictionary.cpp"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 ASSET_DATABASE_PATH_SIZE = IFB_CONFIG_FILE_PATH_SIZE;

    static const char* _asset_database_filter_type = "*.ifbdb";
    static const char* _asset_database_filter_desc = "Asset Database";
    static const char* _asset_database_ext         = ".ifbdb";

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the engine owns the open database, the window owns the paths for the next build
    struct asset_database_state {
        const char*   status;
        char          path_dictionary [ASSET_DATABASE_PATH_SIZE];
        char          path_database   [ASSET_DATABASE_PATH_SIZE];
        char          file_path       [ASSET_DATABASE_PATH_SIZE]; // empty until a database is built or opened
        bool          is_selected;
        bool          is_preview_loaded;
        u32           selected_index;
        asset_preview preview;
    } static _asset_database;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void asset_database_add_extension (char* path, const char* ext);
    IFB_INLINE void asset_database_opened        (const bool did_open, const char* path);
    IFB_INLINE void asset_database_controls_build(void);
    IFB_INLINE void asset_database_controls_view (void);
    IFB_INLINE void asset_database_table         (void);
    IFB_INLINE void asset_database_select        (const u32 index);
    IFB_INLINE void asset_database_preview_asset (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_game_asset_database(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_game_menu_assets_item_database, &is_open)) {

            asset_database_controls_build ();
            asset_database_controls_view  ();
            asset_database_table          ();
            asset_database_preview_asset  ();
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the dialog doesn't add an extension, so a file name without one gets it here
    IFB_INLINE void
    asset_database_add_extension(
        char*       path,
        const char* ext) {

        const char* file_name = path;
        bool        has_ext   = false;
        for (
            const char* c = path;
            *c != 0;
            ++c) {

            if (*c == '\\' || *c == '/') {
                file_name = c + 1;
                has_ext   = false;
            }
            else if (*c == '.') {
                has_ext = true;
            }
        }
        if (!has_ext && file_name[0] != 0) {
            strncat_s(path, ASSET_DATABASE_PATH_SIZE, ext, ASSET_DATABASE_PATH_SIZE - strlen(path) - 1);
        }
    }

    // the assets may have changed, so nothing stays selected
    // a failed build may have closed the database that was open
    IFB_INLINE void
    asset_database_opened(
        const bool  did_open,
        const char* path) {

        auto& state = _asset_database;

        state.is_selected = false;

        if (did_open) {
            strncpy_s(state.file_path, path, ASSET_DATABASE_PATH_SIZE - 1);
        }
        else if (eng_asset_database_get_count() == 0) {
            state.file_path[0] = 0;
        }
    }

    // a dictionary file goes in, a database file comes out
    IFB_INLINE void
    asset_database_controls_build(
        void) {

        auto& state = _asset_database;

        ImGui::SeparatorText("Build");

        ImGui::InputText("Dictionary", state.path_dictionary, ASSET_DATABASE_PATH_SIZE);
        ImGui::SameLine();
        if (ImGui::Button("Browse...##dictionary")) {

            const char* path = eng_file_dialog_open(_asset_dictionary_filter_type, _asset_dictionary_filter_desc);
            if (path) {
                strncpy_s(state.path_dictionary, path, ASSET_DATABASE_PATH_SIZE - 1);
            }
        }

        ImGui::InputText("Database", state.path_database, ASSET_DATABASE_PATH_SIZE);
        ImGui::SameLine();
        if (ImGui::Button("Browse...##database")) {

            const char* path = eng_file_dialog_save(_asset_database_filter_type, _asset_database_filter_desc);
            if (path) {
                strncpy_s(state.path_database, path, ASSET_DATABASE_PATH_SIZE - 1);
                asset_database_add_extension(state.path_database, _asset_database_ext);
            }
        }

        // the dictionary is read from its file, so unsaved changes in the editor aren't built
        if (ImGui::Button("Build")) {

            if (state.path_dictionary[0] == 0 || state.path_database[0] == 0) {
                state.status = "Failed to build the database: it needs a dictionary and a database path";
            }
            else {
                const bool did_build = eng_asset_database_build(state.path_dictionary, state.path_database);
                asset_database_opened(did_build, state.path_database);

                state.status = did_build
                    ? "Built the database"
                    : eng_asset_database_get_error();
            }
        }
    }

    // open and close the database being viewed
    IFB_INLINE void
    asset_database_controls_view(
        void) {

        auto& state = _asset_database;

        ImGui::SeparatorText("Database");

        if (ImGui::Button("Open...")) {

            const char* path = eng_file_dialog_open(_asset_database_filter_type, _asset_database_filter_desc);
            if (path) {
                const bool did_load = eng_asset_database_load(path);
                asset_database_opened(did_load, path);

                state.status = did_load
                    ? "Opened the database"
                    : eng_asset_database_get_error();
            }
        }

        // the database the game is running from, read only, the game keeps using its own copy of it
        ImGui::SameLine();
        if (ImGui::Button("Open Runtime")) {

            char path[ASSET_DATABASE_PATH_SIZE];
            const bool did_get  = eng_asset_runtime_get_path(path, ASSET_DATABASE_PATH_SIZE);
            const bool did_load = did_get && eng_asset_database_load(path);
            asset_database_opened(did_load, path);

            state.status = did_load ? "Opened the runtime database"
                         : did_get  ? eng_asset_database_get_error()
                                    : "Failed to find the runtime database's path";
        }

        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            eng_asset_database_close();
            asset_database_opened(false, NULL);
            state.status = "Closed the database";
        }

        // the engine reads the runtime database from the working directory, the open one replaces it
        ImGui::SameLine();
        ImGui::BeginDisabled(state.file_path[0] == 0 || eng_asset_database_is_runtime());
        if (ImGui::Button("Copy to Runtime")) {
            state.status = eng_asset_database_copy_to_runtime()
                ? "Copied the database to the runtime database"
                : eng_asset_database_get_error();
        }
        ImGui::EndDisabled();

        ImGui::Text(
            "%s (%u assets)%s",
            state.file_path[0] != 0 ? state.file_path : "(none)",
            eng_asset_database_get_count(),
            eng_asset_database_is_runtime() ? ", the runtime database" : ""
        );

        ImGui::Text(
            "Runtime: %s in the working directory (%u assets)",
            config_instance().asset_database_path,
            eng_asset_runtime_get_count()
        );

        if (state.status) {
            ImGui::TextWrapped("%s", state.status);
        }
    }

    // every asset in the database, in the order they are stored
    IFB_INLINE void
    asset_database_table(
        void) {

        auto& state = _asset_database;

        ImGui::SeparatorText("Assets");

        if (ImGui::BeginTable("tbl-asset-database", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {

            gui_table_column_stretch ("Name");
            gui_table_column_fixed   ("Type",   "Sprite Atlas");
            gui_table_column_fixed   ("Handle", "DDDDDDDD");
            gui_table_column_fixed   ("Offset", "0000000000"); // the most digits a u32 has
            gui_table_column_fixed   ("Size",   "0000000000");
            ImGui::TableHeadersRow  ();

            const u32 count = eng_asset_database_get_count();
            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                asset_database_info info;
                if (!eng_asset_database_get_info(index, info)) continue;

                ImGui::PushID(index);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                const bool is_selected = (state.is_selected && state.selected_index == index);
                if (ImGui::Selectable(info.name, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    asset_database_select(index);
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted((info.type < asset_type_e_count) ? _asset_dictionary_type_names[info.type] : "?");

                ImGui::TableNextColumn();
                ImGui::Text("%08X", info.hnd.val);

                ImGui::TableNextColumn();
                ImGui::Text("%u", info.offset);

                ImGui::TableNextColumn();
                ImGui::Text("%u", info.size);

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    // the preview is loaded once, when the asset is selected
    IFB_INLINE void
    asset_database_select(
        const u32 index) {

        auto& state = _asset_database;

        state.is_selected       = true;
        state.selected_index    = index;
        state.is_preview_loaded = eng_asset_database_load_preview(index, state.preview);
    }

    // the contents of the selected asset, read back from the database
    IFB_INLINE void
    asset_database_preview_asset(
        void) {

        auto& state = _asset_database;

        ImGui::SeparatorText("Preview");

        if (!state.is_selected) {
            ImGui::TextUnformatted("No asset selected");
            return;
        }
        if (!state.is_preview_loaded) {
            ImGui::TextUnformatted("Failed to load the asset");
            return;
        }

        gui_asset_preview("asset-database-preview", state.preview);

        // the same way the game plays it, through the asset manager and the runtime database
        if (state.preview.type == asset_type_e_sound) {

            asset_database_info info;
            if (ImGui::Button("Play From Runtime") && eng_asset_database_get_info(state.selected_index, info)) {
                const hnd_audio_voice h_voice = eng_audio_sound_play(info.name, audio_group_e_sfx, 0.5f, false);
                state.status = (h_voice != INVALID_HANDLE)
                    ? "Playing the sound from the runtime database"
                    : "Failed to play the sound, it may not be in the runtime database, or there's no audio device";
            }

            ImGui::SameLine();
            if (ImGui::Button("Stream From Runtime") && eng_asset_database_get_info(state.selected_index, info)) {
                const hnd_audio_voice h_voice = eng_audio_sound_stream(info.name, audio_group_e_sfx, 0.5f, false);
                state.status = (h_voice != INVALID_HANDLE)
                    ? "Streaming the sound from the runtime database"
                    : "Failed to stream the sound, it may not be in the runtime database, every stream may be in use, or there's no audio device";
            }
        }
    }
};
