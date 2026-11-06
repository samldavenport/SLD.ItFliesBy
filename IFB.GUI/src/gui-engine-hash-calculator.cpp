#pragma once

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 HASH_CALCULATOR_INPUT_SIZE = 256;
    static constexpr u32 HASH_CALCULATOR_VALUE_SIZE = 40;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // every hash in the project is meow hash, but call sites hash different bytes:
    // either just the string's characters, or the string in a zeroed fixed size buffer
    struct hash_calculator_row {
        const char* label;
        const char* used_by;
        u32         buffer_size; // 0 hashes only the string's characters
        bool        is_upper;
    };

    static const hash_calculator_row _hash_calculator_rows[] = {
        { "Characters",             "cstr_nvar_hash",                                                         0,   false },
        { "Characters, upper case", "Asset dictionary IDs",                                                   0,   true  },
        { "16 byte buffer",         "cstr_c16_hash, entity IDs (tags), asset handles, map and particle emitter handles, Tiled enum strings", 16, false },
        { "8 byte buffer",          "cstr_c8_hash",                                                           8,   false },
        { "32 byte buffer",         "cstr_c32_hash",                                                          32,  false },
        { "64 byte buffer",         "cstr_c64_hash",                                                          64,  false },
        { "128 byte buffer",        "cstr_c128_hash",                                                         128, false },
        { "256 byte buffer",        "cstr_c256_hash",                                                         256, false },
    };

    struct hash_calculator_state {
        char input [HASH_CALCULATOR_INPUT_SIZE];
        bool show_decimal;
    } static _hash_calculator;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void hash_calculator_value_cell (const char* id, const char* value);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_hash_calculator(
        bool& is_open) {

        if (!is_open) return;

        auto& state = _hash_calculator;

        ImGui::SetNextWindowSize(ImVec2(900.0f, 320.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(_gui_str->window_engine_hash_calculator, &is_open)) {

            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##input", "Text to hash", state.input, HASH_CALCULATOR_INPUT_SIZE);
            ImGui::Checkbox("Show u32 as decimal", &state.show_decimal);
            ImGui::SameLine();
            ImGui::TextDisabled("(click a value to copy it)");

            const u32 length = (u32)strnlen(state.input, HASH_CALCULATOR_INPUT_SIZE);

            const ImGuiTableFlags table_flags =
                ImGuiTableFlags_Borders   |
                ImGuiTableFlags_RowBg     |
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollX;

            if (ImGui::BeginTable("##hashes", 6, table_flags)) {

                ImGui::TableSetupColumn("Bytes hashed", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Size",         ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("hash_u32",     ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("hash_u64",     ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("hash_u128",    ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Used by",      ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                const u32 row_count = sizeof(_hash_calculator_rows) / sizeof(hash_calculator_row);
                for (
                    u32 row_index = 0;
                        row_index < row_count;
                      ++row_index) {

                    const hash_calculator_row& row = _hash_calculator_rows[row_index];

                    // fixed size buffers keep as much of the string as fits and are zero after it
                    char      bytes [HASH_CALCULATOR_INPUT_SIZE] = {0};
                    const u32 size_copied = (row.buffer_size != 0 && length > row.buffer_size)
                        ? row.buffer_size
                        : length;
                    for (u32 i = 0; i < size_copied; ++i) {
                        bytes[i] = row.is_upper
                            ? (char)toupper((unsigned char)state.input[i])
                            : state.input[i];
                    }

                    const u32  size         = (row.buffer_size != 0) ? row.buffer_size : length;
                    const bool is_truncated = (row.buffer_size != 0 && length > row.buffer_size);

                    ImGui::PushID((int)row_index);
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(row.label);

                    ImGui::TableNextColumn();
                    ImGui::Text("%u", size);
                    if (is_truncated) {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "(cut)");
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("The text is longer than the buffer, so only the first %u characters are hashed", row.buffer_size);
                        }
                    }

                    // the hash functions need at least one byte
                    if (size == 0) {
                        ImGui::TableNextColumn(); ImGui::TextDisabled("-");
                        ImGui::TableNextColumn(); ImGui::TextDisabled("-");
                        ImGui::TableNextColumn(); ImGui::TextDisabled("-");
                    }
                    else {
                        const u32  h_u32  = hash_u32  ((void*)bytes, size);
                        const u64  h_u64  = hash_u64  ((void*)bytes, size);
                        const u128 h_u128 = hash_u128 ((void*)bytes, size);

                        char value [HASH_CALCULATOR_VALUE_SIZE];

                        ImGui::TableNextColumn();
                        if (state.show_decimal) snprintf(value, sizeof(value), "%u",     h_u32);
                        else                    snprintf(value, sizeof(value), "0x%08X", h_u32);
                        hash_calculator_value_cell("##u32", value);

                        ImGui::TableNextColumn();
                        snprintf(value, sizeof(value), "0x%016llX", (unsigned long long)h_u64);
                        hash_calculator_value_cell("##u64", value);

                        ImGui::TableNextColumn();
                        snprintf(value, sizeof(value), "0x%016llX%016llX",
                            (unsigned long long)h_u128.u64_hi,
                            (unsigned long long)h_u128.u64_lo);
                        hash_calculator_value_cell("##u128", value);
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextWrapped("%s", row.used_by);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE void
    hash_calculator_value_cell(
        const char* id,
        const char* value) {

        ImGui::PushID(id);
        if (ImGui::Selectable(value)) {
            ImGui::SetClipboardText(value);
        }
        ImGui::PopID();
    }
};
