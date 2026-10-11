#include "ifb-engine.hpp"
#include "ifb-gui.hpp"

namespace ifb {

    IFB_INTERNAL void
    gui_engine_particle_system(
        bool& is_open) {

        if (!is_open) return;

        if (ImGui::Begin(_gui_str->menu_engine_item_particle_system, &is_open)) {

        }
        ImGui::End();
    }
};

