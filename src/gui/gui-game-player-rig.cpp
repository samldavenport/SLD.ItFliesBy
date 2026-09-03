#include "ifb-gui.hpp"
#include "ifb-game.hpp"
#include "imgui.h"

namespace ifb {

    IFB_INTERNAL void
    gui_game_player_rig(
        bool& is_open) {

        // const game_context*    game_ctx   = game_context_get_instance();
        // const game_player_rig* player_rig = game_ctx->player_rig;
        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_game_item_player_rig, &is_open)) {
            
            ImGui::SeparatorText("Connor");
           
            if (ImGui::BeginTable("tbl-player-rig-connor", 2, ImGuiTableFlags_Borders)) {
                
                ImGui::TableSetupColumn("Property");
                ImGui::TableSetupColumn("Value");
                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
};
