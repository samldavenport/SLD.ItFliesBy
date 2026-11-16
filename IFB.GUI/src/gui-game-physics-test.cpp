#pragma once

#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include "implot.h"

namespace ifb {

    // how many frames of step times the plot keeps
    static constexpr u32 GUI_PHYS_TEST_HISTORY_SIZE = 240;

    // a property and its value, as a row of a two column table
    inline void
    gui_game_physics_test_row(
        const char* property,
        const char* fmt,
        ...) {

        ImGui::TableNextRow        ();
        ImGui::TableSetColumnIndex (0);
        ImGui::TextUnformatted     (property);
        ImGui::TableSetColumnIndex (1);

        va_list args;
        va_start(args, fmt);
        ImGui::TextV(fmt, args);
        va_end(args);
    }

    // the physics test's settings and readouts
    // it's always open while the physics test is the active scene, and gone the rest of the time
    // the settings are the scene's, the gui writes them, and the scene picks them up next frame
    IFB_INTERNAL void
    gui_game_physics_test(
        const game_context* game_ctx) {

        if (game_ctx == NULL || game_ctx->phys_test == NULL) return;

        game_phys_test_settings&      settings = game_ctx->phys_test->settings;
        const game_phys_test_readout& readout  = game_ctx->phys_test->readout;

        eng_physics_stats stats;
        eng_physics_get_stats(stats);

        // a rolling history of the step times, for the plot
        static f32 history_total   [GUI_PHYS_TEST_HISTORY_SIZE];
        static f32 history_detect  [GUI_PHYS_TEST_HISTORY_SIZE];
        static f32 history_resolve [GUI_PHYS_TEST_HISTORY_SIZE];
        static u32 history_next  = 0;
        static u32 history_count = 0;
        history_total   [history_next] = stats.ms_total;
        history_detect  [history_next] = stats.ms_detect;
        history_resolve [history_next] = stats.ms_resolve;
        history_next = (history_next + 1) % GUI_PHYS_TEST_HISTORY_SIZE;
        if (history_count < GUI_PHYS_TEST_HISTORY_SIZE) ++history_count;

        // no close button, it's the scene's control panel
        ImGui::SetNextWindowSize(ImVec2(380.0f, 0.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(_gui_str->window_game_physics_test, NULL)) {
            ImGui::End();
            return;
        }

        //----------------------------------------------------------------
        // respawn settings
        //----------------------------------------------------------------

        ImGui::SeparatorText("Spawn");

        int quad_count = (int)settings.quad_count;
        if (ImGui::SliderInt("Quad Count", &quad_count, 1, (int)GAME_PHYS_TEST_QUAD_CAPACITY, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            settings.quad_count = (u32)quad_count;
        }
        ImGui::SliderFloat("Quad Size", &settings.quad_size, 0.01f, 0.2f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Speed Min", &settings.speed_min, 0.0f,  5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Speed Max", &settings.speed_max, 0.0f,  5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (settings.speed_max < settings.speed_min) settings.speed_max = settings.speed_min;

        if (ImGui::Button("Respawn")) {
            settings.is_respawn_requested = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("count, size, and speed apply on respawn");

        //----------------------------------------------------------------
        // live settings
        //----------------------------------------------------------------

        ImGui::SeparatorText("Live");

        ImGui::SliderFloat("Restitution", &settings.restitution, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Mass Min",    &settings.mass_min,    0.1f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Mass Max",    &settings.mass_max,    0.1f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (settings.mass_max < settings.mass_min) settings.mass_max = settings.mass_min;

        //----------------------------------------------------------------
        // the quads
        //----------------------------------------------------------------

        ImGui::SeparatorText("Quads");

        const ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("tbl-phys-test-quads", 2, table_flags)) {

            gui_game_physics_test_row("Spawned", "%u of %u", readout.quad_count, settings.quad_count);
            gui_game_physics_test_row("Moving",  "%u",       readout.quad_count_moving);
            gui_game_physics_test_row("Speed",   "avg %.3f, max %.3f", readout.speed_avg, readout.speed_max);

            // with perfect bounces the energy should hold, the drift is how far the solver is off
            const f32 drift_p100 = (readout.kinetic_energy_start > 0.0f)
                ? (((readout.kinetic_energy / readout.kinetic_energy_start) - 1.0f) * 100.0f)
                : 0.0f;
            gui_game_physics_test_row("Kinetic Energy", "%.4f (%+.1f%%)", readout.kinetic_energy, drift_p100);

            ImGui::TableNextRow        ();
            ImGui::TableSetColumnIndex (0);
            ImGui::TextUnformatted     ("Escaped");
            ImGui::TableSetColumnIndex (1);
            if (readout.escaped_count > 0) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%u", readout.escaped_count);
            else                           ImGui::Text("0");

            ImGui::EndTable();
        }

        //----------------------------------------------------------------
        // the physics step
        //----------------------------------------------------------------

        ImGui::SeparatorText("Physics Step");

        if (ImGui::BeginTable("tbl-phys-test-step", 2, table_flags)) {

            ImGui::TableNextRow        ();
            ImGui::TableSetColumnIndex (0);
            ImGui::TextUnformatted     ("Pairs");
            ImGui::TableSetColumnIndex (1);
            ImGui::Text                ("%u / %u", stats.count_pairs, stats.capacity_pairs);
            if (stats.count_pairs_dropped > 0) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%u dropped", stats.count_pairs_dropped);
            }

            gui_game_physics_test_row("Contacts",  "%u",       stats.count_contacts);
            gui_game_physics_test_row("Resolved",  "%u",       stats.count_resolved);

            // the queue only holds so many events a frame, crowded contacts overflow it
            ImGui::TableNextRow        ();
            ImGui::TableSetColumnIndex (0);
            ImGui::TextUnformatted     ("Events Dropped");
            ImGui::TableSetColumnIndex (1);
            if (stats.count_events_dropped > 0) ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%u", stats.count_events_dropped);
            else                                ImGui::Text("0");
            gui_game_physics_test_row("Detect",    "%.3f ms",  stats.ms_detect);
            gui_game_physics_test_row("Springs",   "%.3f ms",  stats.ms_springs);
            gui_game_physics_test_row("Integrate", "%.3f ms",  stats.ms_integrate);
            gui_game_physics_test_row("Resolve",   "%.3f ms",  stats.ms_resolve);
            gui_game_physics_test_row("Dispatch",  "%.3f ms",  stats.ms_dispatch);
            gui_game_physics_test_row("Total",     "%.3f ms",  stats.ms_total);
            gui_game_physics_test_row("Frame",     "%.1f fps", ImGui::GetIO().Framerate);

            ImGui::EndTable();
        }

        // oldest first, the history is a ring
        const int plot_offset = (int)((history_count < GUI_PHYS_TEST_HISTORY_SIZE) ? 0 : history_next);
        if (ImPlot::BeginPlot("##phys-test-step-ms", ImVec2(-1.0f, 160.0f), ImPlotFlags_NoMouseText)) {
            ImPlot::SetupAxes   ("frame", "ms", ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)GUI_PHYS_TEST_HISTORY_SIZE, ImPlotCond_Always);
            const ImPlotSpec ring(ImPlotProp_Offset, plot_offset);
            ImPlot::PlotLine    ("Total",   history_total,   (int)history_count, 1.0, 0.0, ring);
            ImPlot::PlotLine    ("Detect",  history_detect,  (int)history_count, 1.0, 0.0, ring);
            ImPlot::PlotLine    ("Resolve", history_resolve, (int)history_count, 1.0, 0.0, ring);
            ImPlot::EndPlot     ();
        }

        ImGui::End();
    }
};
