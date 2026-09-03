#include "ifb-gui.hpp"
#include "ifb-game.hpp"
#include "imgui.h"

namespace ifb {

    IFB_INTERNAL void
    gui_game_player_rig(
        bool& is_open) {

        const game_context*    game_ctx   = game_context_get_instance();
        const game_player_rig* player_rig = game_ctx->player_rig;
        const game_map*        map        = game_ctx->map;

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_game_item_player_rig, &is_open)) {
            
            ImGui::SeparatorText("Connor");
          
            const u32 col_property = 0;
            const u32 col_value    = 1;

            // get map coordinates
            map_coords coords;
            const bool did_find = eng_map_get_entity_coords(
                map->eng_hnd,
                player_rig->connor_id,
                coords
            );

            if (ImGui::BeginTable("tbl-player-rig-connor", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
               

                // entity id
                ImGui::TableNextRow        ();
                ImGui::TableSetColumnIndex (col_property);
                ImGui::TextUnformatted     ("Entity ID");
                ImGui::TableSetColumnIndex (col_value);
                ImGui::Text                ("%08X", player_rig->connor_id.val);

                // map coordinates
                ImGui::TableNextRow        ();
                ImGui::TableSetColumnIndex (col_property);
                ImGui::TextUnformatted     ("Map Coordinates");
                ImGui::TableSetColumnIndex (col_value);
                ImGui::Text                ("Row: %d, Col:%d", coords.row, coords.col);
                
                

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
};
