#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb-types.hpp"
#include "imgui.h"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 PARTICLE_SYSTEM_NAME_SIZE      = 16;
    static constexpr f32 PARTICLE_SYSTEM_CONTROLS_WIDTH = 380.0f;

    static const char* _particle_system_type_names[particle_emitter_type_e_count] = {
        "Parabolic",
        "Radial"
    };

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the window owns the values of the next emitter until it is created
    struct particle_system_state {
        bool                     is_init;
        bool                     did_create_fail;
        hnd_particle_emitter     selected;
        char                     name[PARTICLE_SYSTEM_NAME_SIZE];
        s32                      type;
        s32                      count;
        f32                      lifespan_ms;
        f32                      spawn_delay_ms;
        f32                      spawn_radius;
        f32                      width;
        f32                      height;
        f32                      color_primary   [4];
        f32                      color_secondary [4];
        particle_parabolic_props parabolic;
        particle_radial_props    radial;
    } static _particle_system;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void particle_system_preset_fountain  (void);
    IFB_INLINE void particle_system_preset_explosion (void);
    IFB_INLINE void particle_system_controls_create  (void);
    IFB_INLINE void particle_system_controls_table   (void);
    IFB_INLINE void particle_system_controls_edit    (particle_emitter_info& info);
    IFB_INLINE void particle_system_controls_emitter (void);
    IFB_INLINE void particle_system_controls_camera  (void);
    IFB_INLINE void particle_system_viewport         (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_particle_system(
        bool& is_open) {

        if (!is_open) return;

        if (!_particle_system.is_init) {
            _particle_system.is_init  = true;
            _particle_system.selected = INVALID_HANDLE;
            particle_system_preset_fountain();
        }

        if (ImGui::Begin(_gui_str->menu_engine_item_particle_system, &is_open)) {

            // controls on the left
            if (ImGui::BeginChild("particle-system-controls", ImVec2(PARTICLE_SYSTEM_CONTROLS_WIDTH, 0.0f), ImGuiChildFlags_Border)) {

                particle_system_controls_create  ();
                particle_system_controls_table   ();
                particle_system_controls_emitter ();
                particle_system_controls_camera  ();
            }
            ImGui::EndChild();

            // the viewport gets the rest of the window
            ImGui::SameLine();
            if (ImGui::BeginChild("particle-system-viewport", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border)) {

                particle_system_viewport();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE color_rgba_u32
    particle_system_color(
        const f32* rgba) {

        color_rgba_u32 color;
        color.r = (byte)((rgba[0] * 255.0f) + 0.5f);
        color.g = (byte)((rgba[1] * 255.0f) + 0.5f);
        color.b = (byte)((rgba[2] * 255.0f) + 0.5f);
        color.a = (byte)((rgba[3] * 255.0f) + 0.5f);
        return(color);
    }

    IFB_INLINE void
    particle_system_get_color(
        f32*                  rgba,
        const color_rgba_u32& color) {

        rgba[0] = (f32)color.r / 255.0f;
        rgba[1] = (f32)color.g / 255.0f;
        rgba[2] = (f32)color.b / 255.0f;
        rgba[3] = (f32)color.a / 255.0f;
    }

    IFB_INLINE void
    particle_system_set_color(
        f32*      rgba,
        const f32 r,
        const f32 g,
        const f32 b,
        const f32 a) {

        rgba[0] = r;
        rgba[1] = g;
        rgba[2] = b;
        rgba[3] = a;
    }

    IFB_INLINE void
    particle_system_preset_fountain(
        void) {

        auto& state = _particle_system;

        strncpy_s(state.name, "fountain", PARTICLE_SYSTEM_NAME_SIZE - 1);
        state.type                     = particle_emitter_type_e_parabolic;
        state.count                    = 96;
        state.lifespan_ms              = 1500.0f;
        state.spawn_delay_ms           = 15.0f;
        state.spawn_radius             = 0.0f;
        state.width                    = 0.03f;
        state.height                   = 0.03f;
        state.parabolic.velocity       = { 0.0f,  1.6f, 0.0f };
        state.parabolic.gravity        = { 0.0f, -2.0f, 0.0f };
        state.parabolic.spread_degrees = 12.0f;
        particle_system_set_color(state.color_primary,   0.40f, 0.80f, 1.00f, 1.00f);
        particle_system_set_color(state.color_secondary, 0.10f, 0.20f, 0.90f, 0.00f);
    }

    IFB_INLINE void
    particle_system_preset_explosion(
        void) {

        auto& state = _particle_system;

        strncpy_s(state.name, "explosion", PARTICLE_SYSTEM_NAME_SIZE - 1);
        state.type           = particle_emitter_type_e_radial;
        state.count          = 128;
        state.lifespan_ms    = 900.0f;
        state.spawn_delay_ms = 0.0f;
        state.spawn_radius   = 0.0f;
        state.width          = 0.03f;
        state.height         = 0.03f;
        state.radial.speed         = 0.8f;
        state.radial.is_continuous = false;
        particle_system_set_color(state.color_primary,   1.00f, 0.90f, 0.30f, 1.00f);
        particle_system_set_color(state.color_secondary, 0.90f, 0.10f, 0.00f, 0.00f);
    }

    // the values of the next emitter
    IFB_INLINE void
    particle_system_controls_create(
        void) {

        static const auto& cfg = config_instance();

        auto& state = _particle_system;

        ImGui::SeparatorText("New Emitter");

        if (ImGui::Button("Fountain Preset"))  particle_system_preset_fountain();
        ImGui::SameLine();
        if (ImGui::Button("Explosion Preset")) particle_system_preset_explosion();

        ImGui::InputText  ("Name",            state.name, PARTICLE_SYSTEM_NAME_SIZE);
        ImGui::Combo      ("Type",            &state.type, _particle_system_type_names, particle_emitter_type_e_count);
        ImGui::DragInt    ("Count",           &state.count,          1.0f,   1,      cfg.particle_bufffer_count);
        ImGui::DragFloat  ("Lifespan (ms)",   &state.lifespan_ms,    10.0f,  1.0f,   60000.0f, "%.1f");
        ImGui::DragFloat  ("Spawn Delay (ms)",&state.spawn_delay_ms, 1.0f,   0.0f,   10000.0f, "%.1f");
        ImGui::DragFloat  ("Spawn Radius",    &state.spawn_radius,   0.001f, 0.0f,   100.0f,   "%.3f");
        ImGui::DragFloat  ("Width",           &state.width,          0.001f, 0.001f, 10.0f,    "%.3f");
        ImGui::DragFloat  ("Height",          &state.height,         0.001f, 0.001f, 10.0f,    "%.3f");
        ImGui::ColorEdit4 ("Primary Color",   state.color_primary);
        ImGui::ColorEdit4 ("Secondary Color", state.color_secondary);

        // each type has its own properties, in units per second
        if (state.type == particle_emitter_type_e_parabolic) {
            ImGui::DragFloat3 ("Velocity",         state.parabolic.velocity.v,      0.01f, -100.0f, 100.0f, "%.2f");
            ImGui::DragFloat3 ("Gravity",          state.parabolic.gravity.v,       0.01f, -100.0f, 100.0f, "%.2f");
            ImGui::DragFloat  ("Spread (degrees)", &state.parabolic.spread_degrees, 0.5f,   0.0f,   180.0f, "%.1f");
        }
        else {
            ImGui::DragFloat  ("Speed",            &state.radial.speed,             0.01f,  0.01f,  100.0f, "%.2f");
            ImGui::Checkbox   ("Continuous",       (bool*)&state.radial.is_continuous);
        }

        if (ImGui::Button("Create")) {

            const color_rgba_u32 color_primary   = particle_system_color(state.color_primary);
            const color_rgba_u32 color_secondary = particle_system_color(state.color_secondary);

            // the engine asserts on these, so nothing is created without them
            const vec3& velocity = state.parabolic.velocity;
            const bool  is_valid = (
                state.name[0] != 0 &&
                (
                    state.type == particle_emitter_type_e_radial ||
                    velocity.x != 0.0f                           ||
                    velocity.y != 0.0f                           ||
                    velocity.z != 0.0f
                )
            );

            hnd_particle_emitter h_emitter = INVALID_HANDLE;
            if (is_valid && state.type == particle_emitter_type_e_parabolic) {
                h_emitter = eng_particle_emitter_create_parabolic(
                    state.name,
                    state.count,
                    state.lifespan_ms,
                    state.spawn_delay_ms,
                    state.spawn_radius,
                    state.width,
                    state.height,
                    color_primary,
                    color_secondary,
                    state.parabolic
                );
            }
            else if (is_valid) {
                h_emitter = eng_particle_emitter_create_radial(
                    state.name,
                    state.count,
                    state.lifespan_ms,
                    state.spawn_delay_ms,
                    state.spawn_radius,
                    state.width,
                    state.height,
                    color_primary,
                    color_secondary,
                    state.radial
                );
            }

            // the new emitter is selected, so it can be started right away
            state.did_create_fail = (h_emitter == INVALID_HANDLE);
            if (!state.did_create_fail) {
                state.selected = h_emitter;
            }
        }

        if (state.did_create_fail) {
            ImGui::SameLine();
            ImGui::TextUnformatted("Failed: the name is empty or in use, or there are no free emitters");
        }
    }

    // every emitter that exists, click a row to select it
    IFB_INLINE void
    particle_system_controls_table(
        void) {

        auto& state = _particle_system;

        ImGui::SeparatorText("Emitters");

        if (ImGui::BeginTable("tbl-particle-emitters", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

            ImGui::TableSetupColumn ("Name");
            ImGui::TableSetupColumn ("Type");
            ImGui::TableSetupColumn ("State");
            ImGui::TableSetupColumn ("Live");
            ImGui::TableHeadersRow  ();

            const u32 capacity = eng_particle_emitter_get_capacity();
            for (
                u32 index = 0;
                    index < capacity;
                  ++index) {

                const hnd_particle_emitter h_emitter = eng_particle_emitter_get_hnd(index);
                if (h_emitter == INVALID_HANDLE) continue;

                particle_emitter_info info;
                if (!eng_particle_emitter_get_info(h_emitter, info)) continue;

                const char* emitter_state =
                    info.is_spawning ? "Spawning" :
                    info.is_active   ? "Draining" :
                                       "Idle";

                ImGui::PushID(index);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                const bool is_selected = (h_emitter == state.selected);
                if (ImGui::Selectable(info.name, is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    state.selected = h_emitter;
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(_particle_system_type_names[info.type]);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(emitter_state);

                ImGui::TableNextColumn();
                ImGui::Text("%u / %u", info.count_live, info.count);

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    // live editing, the engine values are read every frame and written back when a control changes
    // the engine rejects invalid values, so the control goes back to what the emitter has
    IFB_INLINE void
    particle_system_controls_edit(
        particle_emitter_info& info) {

        static const auto& cfg = config_instance();

        auto& state = _particle_system;

        const ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp;

        s32 count              = info.count;
        f32 color_primary   [4];
        f32 color_secondary [4];
        particle_system_get_color(color_primary,   info.color_primary);
        particle_system_get_color(color_secondary, info.color_secondary);

        // a new count starts the emitter over with no particles
        bool did_change = false;
        did_change |= ImGui::DragFloat3 ("Position",         info.position.v,      0.01f, -100.0f, 100.0f,                     "%.2f", flags);
        did_change |= ImGui::DragInt    ("Count",            &count,               1.0f,   1,      cfg.particle_bufffer_count, "%d",   flags);
        did_change |= ImGui::DragFloat  ("Lifespan (ms)",    &info.lifespan_ms,    10.0f,  1.0f,   60000.0f,                   "%.1f", flags);
        did_change |= ImGui::DragFloat  ("Spawn Delay (ms)", &info.spawn_delay_ms, 1.0f,   0.0f,   10000.0f,                   "%.1f", flags);
        did_change |= ImGui::DragFloat  ("Spawn Radius",     &info.spawn_radius,   0.001f, 0.0f,   100.0f,                     "%.3f", flags);
        did_change |= ImGui::DragFloat  ("Width",            &info.width,          0.001f, 0.001f, 10.0f,                      "%.3f", flags);
        did_change |= ImGui::DragFloat  ("Height",           &info.height,         0.001f, 0.001f, 10.0f,                      "%.3f", flags);
        did_change |= ImGui::ColorEdit4 ("Primary Color",    color_primary);
        did_change |= ImGui::ColorEdit4 ("Secondary Color",  color_secondary);
        if (did_change) {
            info.count           = count;
            info.color_primary   = particle_system_color(color_primary);
            info.color_secondary = particle_system_color(color_secondary);
            eng_particle_emitter_update(state.selected, info);
        }

        // each type has its own properties, in units per second
        particle_parabolic_props parabolic;
        if (eng_particle_emitter_get_props_parabolic(state.selected, parabolic)) {

            bool did_change_props = false;
            did_change_props |= ImGui::DragFloat3 ("Velocity",         parabolic.velocity.v,      0.01f, -100.0f, 100.0f, "%.2f", flags);
            did_change_props |= ImGui::DragFloat3 ("Gravity",          parabolic.gravity.v,       0.01f, -100.0f, 100.0f, "%.2f", flags);
            did_change_props |= ImGui::DragFloat  ("Spread (degrees)", &parabolic.spread_degrees, 0.5f,   0.0f,   180.0f, "%.1f", flags);
            if (did_change_props) {
                eng_particle_emitter_set_props_parabolic(state.selected, parabolic);
            }
        }

        particle_radial_props radial;
        if (eng_particle_emitter_get_props_radial(state.selected, radial)) {

            bool did_change_props = false;
            did_change_props |= ImGui::DragFloat ("Speed",      &radial.speed, 0.01f, 0.01f, 100.0f, "%.2f", flags);
            did_change_props |= ImGui::Checkbox  ("Continuous", (bool*)&radial.is_continuous);
            if (did_change_props) {
                eng_particle_emitter_set_props_radial(state.selected, radial);
            }
        }
    }

    // edit, start, stop, and destroy the selected emitter
    IFB_INLINE void
    particle_system_controls_emitter(
        void) {

        auto& state = _particle_system;

        ImGui::SeparatorText("Selected Emitter");

        // the selected emitter may have been destroyed somewhere else
        particle_emitter_info info;
        const bool is_selected = (
            state.selected != INVALID_HANDLE &&
            eng_particle_emitter_get_info(state.selected, info)
        );
        if (!is_selected) {
            state.selected = INVALID_HANDLE;
            ImGui::TextUnformatted("No emitter selected");
            return;
        }

        ImGui::Text("%s (%s)", info.name, _particle_system_type_names[info.type]);

        // the labels are the same as the new emitter controls
        ImGui::PushID("particle-system-selected");
        particle_system_controls_edit(info);
        ImGui::PopID();

        if (ImGui::Button("Start")) {
            eng_particle_emitter_start(state.selected, info.position);
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            eng_particle_emitter_stop(state.selected);
        }
        ImGui::SameLine();
        if (ImGui::Button("Destroy")) {
            eng_particle_emitter_destroy(state.selected);
            state.selected = INVALID_HANDLE;
        }
    }

    // the viewport has its own camera
    IFB_INLINE void
    particle_system_controls_camera(
        void) {

        ImGui::SeparatorText("Viewport Camera");

        vec3 origin = eng_particle_view_get_camera_origin();
        vec3 target = eng_particle_view_get_camera_target();

        if (ImGui::DragFloat3("Origin", origin.v, 0.01f, -100.0f, 100.0f, "%.2f")) {
            eng_particle_view_set_camera_origin(origin);
        }
        if (ImGui::DragFloat3("Target", target.v, 0.01f, -100.0f, 100.0f, "%.2f")) {
            eng_particle_view_set_camera_target(target);
        }
    }

    // the engine renders the active emitters to a texture the size of the viewport
    IFB_INLINE void
    particle_system_viewport(
        void) {

        const ImVec2 size = ImGui::GetContentRegionAvail();
        if (size.x < 1.0f || size.y < 1.0f) return;

        // the view is only drawn on the frames it is requested
        eng_particle_view_request((u32)size.x, (u32)size.y);

        // opengl textures are bottom up
        const ImTextureID texture = (ImTextureID)(intptr_t)eng_particle_view_get_texture();
        ImGui::Image(texture, size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
    }
};
