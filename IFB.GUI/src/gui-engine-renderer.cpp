#pragma once

#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include "implot.h"
#include <cstdio>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 RENDERER_HISTORY_COUNT   = 2048;
    static constexpr f32 RENDERER_HISTORY_SECONDS = 10.0f;
    static constexpr f32 RENDERER_PLOT_HEIGHT     = 200.0f;
    static constexpr f64 RENDERER_BYTES_PER_KB    = 1024.0;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // a ring buffer of the quad buffer's use over time, the oldest sample is at head once it wraps
    // particles are stacked on top of the entity quads, the same way they fill the buffer
    struct renderer_history {
        u32 head;
        u32 count;
        f32 time      [RENDERER_HISTORY_COUNT];
        f32 entities  [RENDERER_HISTORY_COUNT];
        f32 total     [RENDERER_HISTORY_COUNT];
    };

    struct renderer_tool_state {
        eng_renderer_stats stats;
        renderer_history   history;
        bool               is_paused;
    } static _renderer_tool;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void renderer_tool_history_push (renderer_history& history, const f32 time, const eng_renderer_stats& stats);
    IFB_INLINE void renderer_tool_quads        (const eng_renderer_stats& stats, const renderer_history& history, const f32 time);
    IFB_INLINE void renderer_tool_buffers      (const eng_renderer_stats& stats);
    IFB_INLINE void renderer_tool_textures     (const eng_renderer_stats& stats);
    IFB_INLINE void renderer_tool_views        (const eng_renderer_stats& stats);
    IFB_INLINE void renderer_tool_memory       (const eng_renderer_stats& stats);
    IFB_INLINE void renderer_tool_usage_bar    (const u32 used, const u32 capacity);
    IFB_INLINE u32  renderer_tool_items        (const eng_renderer_stats_buffer& buffer, const u32 size);
    IFB_INLINE f64  renderer_tool_kb           (const u32 bytes);
    IFB_INLINE f32  renderer_tool_fraction     (const u32 part, const u32 whole);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_renderer(
        bool& is_open) {

        if (!is_open) return;

        auto& state = _renderer_tool;
        const f32 time = (f32)ImGui::GetTime();

        if (!state.is_paused) {
            eng_renderer_get_stats(state.stats);
            renderer_tool_history_push(state.history, time, state.stats);
        }

        if (!ImGui::Begin(_gui_str->window_engine_renderer, &is_open)) {
            ImGui::End();
            return;
        }

        const eng_renderer_stats& stats = state.stats;

        ImGui::Checkbox("Pause", &state.is_paused);
        ImGui::SameLine();
        if (ImGui::Button("Reset Peaks")) {
            eng_renderer_reset_stats_peaks();
        }
        ImGui::SameLine();
        ImGui::Text("Frames drawn: %u", stats.frame_count);

        if (ImGui::CollapsingHeader("Quad Buffer", ImGuiTreeNodeFlags_DefaultOpen)) {
            renderer_tool_quads(stats, state.history, time);
        }

        if (ImGui::CollapsingHeader("Buffers", ImGuiTreeNodeFlags_DefaultOpen)) {
            renderer_tool_buffers(stats);
        }

        if (ImGui::CollapsingHeader("Textures")) {
            renderer_tool_textures(stats);
        }

        if (ImGui::CollapsingHeader("Views")) {
            renderer_tool_views(stats);
        }

        if (ImGui::CollapsingHeader("Memory")) {
            renderer_tool_memory(stats);
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    renderer_tool_history_push(
        renderer_history&         history,
        const f32                 time,
        const eng_renderer_stats& stats) {

        u32 index;
        if (history.count < RENDERER_HISTORY_COUNT) {
            index = history.count;
            ++history.count;
        }
        else {
            index        = history.head;
            history.head = (history.head + 1) % RENDERER_HISTORY_COUNT;
        }

        const eng_renderer_stats_buffer& quads = stats.buffers[eng_renderer_buffer_e_quads];
        history.time     [index] = time;
        history.entities [index] = (f32)renderer_tool_items(quads, stats.quads_entities.size_used);
        history.total    [index] = (f32)renderer_tool_items(quads, quads.size_used);
    }

    // how full the quad buffer is, and how the entity quads and particles split it
    IFB_INLINE void
    renderer_tool_quads(
        const eng_renderer_stats& stats,
        const renderer_history&   history,
        const f32                 time) {

        const eng_renderer_stats_buffer& quads     = stats.buffers[eng_renderer_buffer_e_quads];
        const eng_renderer_stats_buffer& entities  = stats.quads_entities;
        const eng_renderer_stats_buffer& particles = stats.quads_particles;
        const u32 count_capacity = renderer_tool_items(quads, quads.size_capacity);

        renderer_tool_usage_bar(quads.size_used, quads.size_capacity);
        ImGui::Text(
            "%u / %u quads  |  %.1f / %.1f KB  |  peak %u",
            renderer_tool_items(quads, quads.size_used),
            count_capacity,
            renderer_tool_kb(quads.size_used),
            renderer_tool_kb(quads.size_capacity),
            renderer_tool_items(quads, quads.size_peak)
        );
        ImGui::Text(
            "Entity quads %u (peak %u)  |  Particles %u (peak %u)",
            renderer_tool_items(quads, entities.size_used),
            renderer_tool_items(quads, entities.size_peak),
            renderer_tool_items(quads, particles.size_used),
            renderer_tool_items(quads, particles.size_peak)
        );

        if (!ImPlot::BeginPlot("##renderer_quad_history", ImVec2(-1, RENDERER_PLOT_HEIGHT))) return;

        ImPlot::SetupAxes("Seconds", "Quads", ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, time - RENDERER_HISTORY_SECONDS, time, ImGuiCond_Always);
        ImPlot::SetupLegend(ImPlotLocation_NorthWest);

        if (history.count > 0) {
            const int count  = (int)history.count;
            const int offset = (int)history.head;
            const int stride = sizeof(f32);
            const ImPlotSpec ring(ImPlotProp_Offset, offset, ImPlotProp_Stride, stride);

            // particles are the band between the entity quads and the total
            ImPlot::PlotShaded ("Particles",    history.time, history.entities, history.total, count, ring);
            ImPlot::PlotShaded ("Entity quads", history.time, history.entities, count, 0.0,        ring);
            ImPlot::PlotLine   ("Total",        history.time, history.total,    count,             ring);
        }

        ImPlot::EndPlot();
    }

    IFB_INLINE void
    renderer_tool_buffers(
        const eng_renderer_stats& stats) {

        static const ImGuiTableFlags flags =
            ImGuiTableFlags_Borders   |
            ImGuiTableFlags_RowBg     |
            ImGuiTableFlags_SizingFixedFit;

        if (!ImGui::BeginTable("##renderer_buffers", 7, flags)) return;

        gui_table_column_fixed   ("Buffer",        "Immediate Screen");
        gui_table_column_fixed   ("Used",          "000000");
        gui_table_column_fixed   ("Peak",          "000000");
        gui_table_column_fixed   ("Capacity",      "000000");
        gui_table_column_fixed   ("Used (KB)",     "0000.0");
        gui_table_column_fixed   ("Capacity (KB)", "0000.0");
        gui_table_column_stretch ("");
        ImGui::TableHeadersRow();

        for (
            u32 index = 0;
                index < eng_renderer_buffer_e_count;
              ++index) {

            const eng_renderer_stats_buffer& buffer = stats.buffers[index];

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(buffer.name);
            ImGui::TableNextColumn(); ImGui::Text("%u",   renderer_tool_items(buffer, buffer.size_used));
            ImGui::TableNextColumn(); ImGui::Text("%u",   renderer_tool_items(buffer, buffer.size_peak));
            ImGui::TableNextColumn(); ImGui::Text("%u",   renderer_tool_items(buffer, buffer.size_capacity));
            ImGui::TableNextColumn(); ImGui::Text("%.1f", renderer_tool_kb(buffer.size_used));
            ImGui::TableNextColumn(); ImGui::Text("%.1f", renderer_tool_kb(buffer.size_capacity));
            ImGui::TableNextColumn(); renderer_tool_usage_bar(buffer.size_used, buffer.size_capacity);
        }

        ImGui::EndTable();
        ImGui::TextDisabled("Counts are quads, or tiles for the tile buffer. The particle view only fills on frames it's open.");
    }

    IFB_INLINE void
    renderer_tool_textures(
        const eng_renderer_stats& stats) {

        ImGui::Text("Sprite atlases on the GPU: %u / %u", stats.sprite_atlases.count, stats.sprite_atlases.capacity);
        if (stats.sprite_atlases.active != INVALID_HANDLE) {
            ImGui::Text("Active sprite atlas: handle %u", stats.sprite_atlases.active.val);
        }
        else {
            ImGui::TextDisabled("No active sprite atlas");
        }

        ImGui::Text("Fonts on the GPU: %u / %u", stats.fonts.count, stats.fonts.capacity);
        ImGui::Text("Active font: %s", stats.fonts.has_active ? "yes" : "none");
    }

    IFB_INLINE void
    renderer_tool_views(
        const eng_renderer_stats& stats) {

        ImGui::Text("Viewport: %u x %u, fov %.1f degrees", stats.viewport.width, stats.viewport.height, stats.viewport.fov_y * (180.0f / 3.14159265f));
        ImGui::Text("Screen layer: %u x %u", stats.screen.width, stats.screen.height);
        ImGui::Text("Particle view: %u x %u", stats.particle_view.width, stats.particle_view.height);
        ImGui::Text("Map view: %u x %u", stats.map_view.width, stats.map_view.height);
        if (stats.map != INVALID_HANDLE) {
            ImGui::Text("Map: handle %u", stats.map.val);
        }
        else {
            ImGui::TextDisabled("No map set");
        }
    }

    IFB_INLINE void
    renderer_tool_memory(
        const eng_renderer_stats& stats) {

        renderer_tool_usage_bar(stats.memory.size_used, stats.memory.size_capacity);
        ImGui::Text(
            "Renderer stack: %.1f / %.1f KB",
            renderer_tool_kb(stats.memory.size_used),
            renderer_tool_kb(stats.memory.size_capacity)
        );
        ImGui::TextDisabled("The buffers and shaders are allocated from it at startup.");
    }

    IFB_INLINE void
    renderer_tool_usage_bar(
        const u32 used,
        const u32 capacity) {

        const f32 fraction = renderer_tool_fraction(used, capacity);

        char overlay[16];
        snprintf(overlay, sizeof(overlay), "%.1f%%", fraction * 100.0f);
        ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);
    }

    IFB_INLINE u32
    renderer_tool_items(
        const eng_renderer_stats_buffer& buffer,
        const u32                        size) {

        if (buffer.item_size == 0) return(0);
        return(size / buffer.item_size);
    }

    IFB_INLINE f64
    renderer_tool_kb(
        const u32 bytes) {

        return((f64)bytes / RENDERER_BYTES_PER_KB);
    }

    IFB_INLINE f32
    renderer_tool_fraction(
        const u32 part,
        const u32 whole) {

        if (whole == 0) return(0.0f);
        return((f32)part / (f32)whole);
    }
};
