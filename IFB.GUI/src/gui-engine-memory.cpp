#pragma once

#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include "implot.h"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 MEMORY_HISTORY_COUNT     = 2048;
    static constexpr f32 MEMORY_HISTORY_SECONDS   = 10.0f;
    static constexpr f32 MEMORY_PLOT_HEIGHT       = 220.0f;
    static constexpr f32 MEMORY_PLOT_ROW_HEIGHT   = 22.0f;
    static constexpr f64 MEMORY_BYTES_PER_MB      = (1024.0 * 1024.0);

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // a ring buffer of totals over time, the oldest sample is at head once it wraps
    struct memory_history {
        u32 head;
        u32 count;
        f32 time      [MEMORY_HISTORY_COUNT];
        f32 used      [MEMORY_HISTORY_COUNT];
        f32 committed [MEMORY_HISTORY_COUNT];
    };

    struct memory_totals {
        u64 used;
        u64 committed;
    };

    struct memory_tool_state {
        eng_mem_stats  stats;
        memory_history history;
        bool           is_paused;
    } static _memory_tool;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE memory_totals memory_tool_totals        (const eng_mem_stats& stats);
    IFB_INLINE void          memory_tool_history_push  (memory_history& history, const f32 time, const memory_totals& totals);
    IFB_INLINE void          memory_tool_plot_history  (const memory_history& history, const f32 time);
    IFB_INLINE void          memory_tool_plot_reservations (const eng_mem_stats& stats);
    IFB_INLINE void          memory_tool_plot_arenas   (const eng_mem_stats& stats);
    IFB_INLINE void          memory_tool_table         (const eng_mem_stats& stats);
    IFB_INLINE f64           memory_tool_mb            (const u64 bytes);
    IFB_INLINE f64           memory_tool_percent       (const u64 part, const u64 whole);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_memory(
        bool& is_open) {

        if (!is_open) return;

        auto& state = _memory_tool;
        const f32 time = (f32)ImGui::GetTime();

        // keep sampling while paused so the snapshot doesn't go stale, just don't record it
        if (!state.is_paused) {
            eng_memory_get_stats(state.stats);
            const memory_totals totals = memory_tool_totals(state.stats);
            memory_tool_history_push(state.history, time, totals);
        }

        if (!ImGui::Begin(_gui_str->window_engine_memory, &is_open)) {
            ImGui::End();
            return;
        }

        const eng_mem_stats& stats  = state.stats;
        const memory_totals  totals = memory_tool_totals(stats);

        ImGui::Checkbox("Pause", &state.is_paused);
        ImGui::SameLine();
        ImGui::Text(
            "Used %.2f MB  |  Committed %.2f MB",
            memory_tool_mb(totals.used),
            memory_tool_mb(totals.committed)
        );

        if (ImGui::CollapsingHeader("History", ImGuiTreeNodeFlags_DefaultOpen)) {
            memory_tool_plot_history(state.history, time);
        }

        if (ImGui::CollapsingHeader("Reservations", ImGuiTreeNodeFlags_DefaultOpen)) {
            memory_tool_plot_reservations(stats);
        }

        if (ImGui::CollapsingHeader("Arena Pools", ImGuiTreeNodeFlags_DefaultOpen)) {
            memory_tool_plot_arenas(stats);
        }

        if (ImGui::CollapsingHeader("Details")) {
            memory_tool_table(stats);
        }

        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the arena pool reservations are left out, the arena pools count that memory instead
    IFB_INLINE memory_totals
    memory_tool_totals(
        const eng_mem_stats& stats) {

        memory_totals totals = {};

        for (
            u32 index = 0;
                index < stats.reservation_count;
              ++index) {

            const eng_mem_stats_reservation& res = stats.reservations[index];
            if (res.is_arena_pool) continue;

            // reservation pages are committed when they're pushed
            totals.used      += res.size_used;
            totals.committed += res.size_used;
        }

        totals.used      += stats.arenas.size_used      + stats.arenas_large.size_used;
        totals.committed += stats.arenas.size_committed + stats.arenas_large.size_committed;

        totals.used      += stats.global_stack.size_used;
        totals.committed += stats.global_stack.size_capacity;

        return(totals);
    }

    IFB_INLINE void
    memory_tool_history_push(
        memory_history&      history,
        const f32            time,
        const memory_totals& totals) {

        u32 index;
        if (history.count < MEMORY_HISTORY_COUNT) {
            index = history.count;
            ++history.count;
        }
        else {
            index        = history.head;
            history.head = (history.head + 1) % MEMORY_HISTORY_COUNT;
        }

        history.time      [index] = time;
        history.used      [index] = (f32)memory_tool_mb(totals.used);
        history.committed [index] = (f32)memory_tool_mb(totals.committed);
    }

    IFB_INLINE void
    memory_tool_plot_history(
        const memory_history& history,
        const f32             time) {

        if (!ImPlot::BeginPlot("##memory_history", ImVec2(-1, MEMORY_PLOT_HEIGHT))) return;

        ImPlot::SetupAxes("Seconds", "MB", ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, time - MEMORY_HISTORY_SECONDS, time, ImGuiCond_Always);
        ImPlot::SetupLegend(ImPlotLocation_NorthWest);

        if (history.count > 0) {
            const int count  = (int)history.count;
            const int offset = (int)history.head;
            const int stride = sizeof(f32);

            ImPlot::PlotShaded ("Committed", history.time, history.committed, count, 0.0, 0, offset, stride);
            ImPlot::PlotLine   ("Committed", history.time, history.committed, count,      0, offset, stride);
            ImPlot::PlotLine   ("Used",      history.time, history.used,      count,      0, offset, stride);
        }

        ImPlot::EndPlot();
    }

    // percent used per reservation, the table has the bytes
    IFB_INLINE void
    memory_tool_plot_reservations(
        const eng_mem_stats& stats) {

        // the reservations, then a row for the global stack
        const u32 count_res = stats.reservation_count;
        const u32 count     = count_res + 1;

        static f64         positions [ENG_MEM_STATS_RESERVATION_MAX + 1];
        static const char* labels    [ENG_MEM_STATS_RESERVATION_MAX + 1];
        static f64         used      [ENG_MEM_STATS_RESERVATION_MAX + 1];
        static f64         pool      [ENG_MEM_STATS_RESERVATION_MAX + 1];

        positions [count_res] = (f64)count_res;
        labels    [count_res] = "Global Stack";
        used      [count_res] = memory_tool_percent(stats.global_stack.size_used, stats.global_stack.size_capacity);
        pool      [count_res] = 0.0;

        for (
            u32 index = 0;
                index < count_res;
              ++index) {

            const eng_mem_stats_reservation& res = stats.reservations[index];
            const f64 percent = memory_tool_percent(res.size_used, res.size_capacity);

            positions [index] = (f64)index;
            labels    [index] = res.name;
            used      [index] = res.is_arena_pool ? 0.0     : percent;
            pool      [index] = res.is_arena_pool ? percent : 0.0;
        }

        const f32 height = MEMORY_PLOT_ROW_HEIGHT * (f32)count + 60.0f;
        if (!ImPlot::BeginPlot("##memory_reservations", ImVec2(-1, height))) return;

        ImPlot::SetupAxes("% Used", NULL, 0, ImPlotAxisFlags_Invert);
        ImPlot::SetupAxisLimits (ImAxis_X1, 0.0, 100.0, ImGuiCond_Always);
        ImPlot::SetupAxisLimits (ImAxis_Y1, -0.5, (f64)count - 0.5, ImGuiCond_Always);
        ImPlot::SetupAxisTicks  (ImAxis_Y1, positions, (int)count, labels);
        ImPlot::SetupLegend     (ImPlotLocation_SouthEast);

        ImPlot::PlotBars("Used",                      used, (int)count, 0.67, 0.0, ImPlotBarsFlags_Horizontal);
        ImPlot::PlotBars("Arena pool (see below)",    pool, (int)count, 0.67, 0.0, ImPlotBarsFlags_Horizontal);

        ImPlot::EndPlot();
    }

    // arenas in use and bytes, as a percent of the pool
    IFB_INLINE void
    memory_tool_plot_arenas(
        const eng_mem_stats& stats) {

        static const char* labels    [] = {"Arenas", "Arenas Large"};
        static const f64   positions [] = {0.0, 1.0};

        const eng_mem_stats_arena_pool& pool_sm = stats.arenas;
        const eng_mem_stats_arena_pool& pool_lg = stats.arenas_large;

        const f64 in_use    [] = {
            memory_tool_percent(pool_sm.arena_count_used, pool_sm.arena_count_total),
            memory_tool_percent(pool_lg.arena_count_used, pool_lg.arena_count_total)
        };
        const f64 committed [] = {
            memory_tool_percent(pool_sm.size_committed, pool_sm.size_capacity),
            memory_tool_percent(pool_lg.size_committed, pool_lg.size_capacity)
        };
        const f64 used      [] = {
            memory_tool_percent(pool_sm.size_used, pool_sm.size_capacity),
            memory_tool_percent(pool_lg.size_used, pool_lg.size_capacity)
        };

        if (!ImPlot::BeginPlot("##memory_arenas", ImVec2(-1, 180.0f))) return;

        ImPlot::SetupAxes("% of Pool", NULL, 0, ImPlotAxisFlags_Invert);
        ImPlot::SetupAxisLimits (ImAxis_X1, 0.0, 100.0, ImGuiCond_Always);
        ImPlot::SetupAxisLimits (ImAxis_Y1, -0.5, 1.5, ImGuiCond_Always);
        ImPlot::SetupAxisTicks  (ImAxis_Y1, positions, 2, labels);
        ImPlot::SetupLegend     (ImPlotLocation_SouthEast);

        // three bars per pool, offset around the tick
        ImPlot::PlotBars("Arenas in use", in_use,    2, 0.25, -0.25, ImPlotBarsFlags_Horizontal);
        ImPlot::PlotBars("Committed",     committed, 2, 0.25,  0.0,  ImPlotBarsFlags_Horizontal);
        ImPlot::PlotBars("Used",          used,      2, 0.25,  0.25, ImPlotBarsFlags_Horizontal);

        ImPlot::EndPlot();
    }

    IFB_INLINE void
    memory_tool_table(
        const eng_mem_stats& stats) {

        static const ImGuiTableFlags flags =
            ImGuiTableFlags_Borders   |
            ImGuiTableFlags_RowBg     |
            ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("##memory_reservations_table", 4, flags)) {

            gui_table_column_stretch ("Reservation");
            gui_table_column_fixed   ("Used (MB)",     "0000.00");
            gui_table_column_fixed   ("Capacity (MB)", "0000.00");
            gui_table_column_fixed   ("Used",          "100.0%");
            ImGui::TableHeadersRow();

            for (
                u32 index = 0;
                    index < stats.reservation_count;
                  ++index) {

                const eng_mem_stats_reservation& res = stats.reservations[index];

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(res.name);
                if (res.is_arena_pool) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(arena pool)");
                }
                ImGui::TableNextColumn(); ImGui::Text("%.2f",  memory_tool_mb(res.size_used));
                ImGui::TableNextColumn(); ImGui::Text("%.2f",  memory_tool_mb(res.size_capacity));
                ImGui::TableNextColumn(); ImGui::Text("%.1f%%", memory_tool_percent(res.size_used, res.size_capacity));
            }

            // the global stack is a static block in the exe, not a reservation
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted("Global Stack");
            ImGui::TableNextColumn(); ImGui::Text("%.2f",  memory_tool_mb(stats.global_stack.size_used));
            ImGui::TableNextColumn(); ImGui::Text("%.2f",  memory_tool_mb(stats.global_stack.size_capacity));
            ImGui::TableNextColumn(); ImGui::Text("%.1f%%", memory_tool_percent(stats.global_stack.size_used, stats.global_stack.size_capacity));

            ImGui::EndTable();
        }

        ImGui::Spacing();

        if (ImGui::BeginTable("##memory_arenas_table", 6, flags)) {

            gui_table_column_stretch ("Pool");
            gui_table_column_fixed   ("Arena (KB)",     "000000");
            gui_table_column_fixed   ("In Use",         "00000 / 00000");
            gui_table_column_fixed   ("Used (MB)",      "0000.00");
            gui_table_column_fixed   ("Committed (MB)", "0000.00");
            gui_table_column_fixed   ("Capacity (MB)",  "0000.00");
            ImGui::TableHeadersRow();

            const char*                     names [] = {"Arenas", "Arenas Large"};
            const eng_mem_stats_arena_pool* pools [] = {&stats.arenas, &stats.arenas_large};

            for (
                u32 index = 0;
                    index < 2;
                  ++index) {

                const eng_mem_stats_arena_pool& pool = *pools[index];

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(names[index]);
                ImGui::TableNextColumn(); ImGui::Text("%u",      pool.arena_size / 1024);
                ImGui::TableNextColumn(); ImGui::Text("%u / %u", pool.arena_count_used, pool.arena_count_total);
                ImGui::TableNextColumn(); ImGui::Text("%.2f",    memory_tool_mb(pool.size_used));
                ImGui::TableNextColumn(); ImGui::Text("%.2f",    memory_tool_mb(pool.size_committed));
                ImGui::TableNextColumn(); ImGui::Text("%.2f",    memory_tool_mb(pool.size_capacity));
            }

            ImGui::EndTable();
        }
    }

    IFB_INLINE f64
    memory_tool_mb(
        const u64 bytes) {

        return((f64)bytes / MEMORY_BYTES_PER_MB);
    }

    IFB_INLINE f64
    memory_tool_percent(
        const u64 part,
        const u64 whole) {

        if (whole == 0) return(0.0);
        return(((f64)part / (f64)whole) * 100.0);
    }
};
