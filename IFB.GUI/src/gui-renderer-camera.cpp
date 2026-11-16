#pragma once

#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "imgui.h"
#include <math.h>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr f32 CAMERA_ORBIT_SPEED    = 0.008f;  // radians per pixel
    static constexpr f32 CAMERA_ZOOM_STEP      = 0.88f;   // distance scale per wheel notch
    static constexpr f32 CAMERA_PITCH_LIMIT    = 1.55f;   // just short of straight up or down, where the view flips
    static constexpr f32 CAMERA_DISTANCE_MIN   = 0.02f;
    static constexpr f32 CAMERA_DISTANCE_MAX   = 100.0f;
    static constexpr f32 CAMERA_DRAG_SPEED     = 0.005f;  // world units per pixel for the number fields
    static constexpr f32 CAMERA_PI             = 3.14159265f;

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the free camera as an orbit around its target, yaw 0 looks down -z like the game
    struct camera_orbit {
        vec3 target;
        f32  yaw;
        f32  pitch;
        f32  distance;
    };

    struct camera_tool_state {
        ImGuiMouseButton drag_button; // the button dragging the camera, -1 when it isn't
        bool             is_panning;
        bool             keep_framing;
    } static _camera_tool = { -1, false, true };

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE camera_orbit camera_tool_orbit_from_free (void);
    IFB_INLINE void         camera_tool_orbit_to_free   (const camera_orbit& orbit);
    IFB_INLINE void         camera_tool_mouse           (camera_orbit& orbit);
    IFB_INLINE void         camera_tool_pan             (camera_orbit& orbit, const f32 dx, const f32 dy);
    IFB_INLINE void         camera_tool_controls        (camera_orbit& orbit);
    IFB_INLINE void         camera_tool_vectors         (const vec3& origin, const vec3& target, const mat4& view);
    IFB_INLINE void         camera_tool_view_matrix     (const mat4& view);
    IFB_INLINE void         camera_tool_axis_header     (const char* label, const ImU32 color);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_renderer_camera(
        bool& is_open) {

        auto& state = _camera_tool;

        // closing the window goes back to the game's camera
        if (!is_open) {
            if (eng_camera_free_is_enabled()) {
                eng_camera_free_set_enabled(false);
            }
            state.drag_button = -1;
            return;
        }

        // the mouse moves the camera even when the window is collapsed
        bool         is_free = eng_camera_free_is_enabled();
        camera_orbit orbit   = camera_tool_orbit_from_free();
        if (is_free) {
            camera_tool_mouse(orbit);
        }

        if (ImGui::Begin(_gui_str->window_renderer_camera, &is_open)) {

            if (ImGui::Checkbox("Free Camera", &is_free)) {
                eng_camera_free_set_enabled(is_free);
                orbit = camera_tool_orbit_from_free();
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(!is_free);
            if (ImGui::Button("Reset to Game View")) {
                eng_camera_free_reset();
                orbit = camera_tool_orbit_from_free();
            }
            ImGui::EndDisabled();

            if (is_free) {
                ImGui::TextWrapped("Drawing the free camera. The game's camera keeps following the player underneath.");
                ImGui::TextDisabled("Over the game: right-drag to orbit, middle-drag or shift + right-drag to pan, wheel to zoom.");
                ImGui::SeparatorText("Free Camera");
                camera_tool_controls(orbit);
            }
            else {
                ImGui::TextWrapped("Drawing the game's camera. Turn on the free camera to look around without moving it.");
                ImGui::Text("FOV Y: %.1f deg", eng_camera_get_fov());
            }

            // what's on screen, the free camera while it's on
            mat4 view;
            vec3 origin;
            vec3 target;
            if (is_free) {
                eng_camera_free_get_xform(view);
                origin = eng_camera_free_get_origin();
                target = eng_camera_free_get_target();
            }
            else {
                eng_camera_get_xform(view);
                origin = eng_camera_get_origin();
                target = eng_camera_get_target();
            }

            if (ImGui::CollapsingHeader("Camera Vectors")) {
                camera_tool_vectors(origin, target, view);
            }
            if (ImGui::CollapsingHeader("View Matrix")) {
                camera_tool_view_matrix(view);
            }
        }
        ImGui::End();

        if (!is_open) {
            eng_camera_free_set_enabled(false);
            state.drag_button = -1;
            return;
        }

        if (eng_camera_free_is_enabled()) {
            camera_tool_orbit_to_free(orbit);
        }
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INLINE camera_orbit
    camera_tool_orbit_from_free(
        void) {

        const vec3& origin = eng_camera_free_get_origin();
        const vec3& target = eng_camera_free_get_target();
        const vec3  offset = {
            origin.x - target.x,
            origin.y - target.y,
            origin.z - target.z
        };

        camera_orbit orbit;
        orbit.target   = target;
        orbit.distance = sqrtf((offset.x * offset.x) + (offset.y * offset.y) + (offset.z * offset.z));
        if (orbit.distance < CAMERA_DISTANCE_MIN) {
            orbit.distance = CAMERA_DISTANCE_MIN;
            orbit.yaw      = 0.0f;
            orbit.pitch    = 0.0f;
            return(orbit);
        }

        orbit.yaw   = atan2f(offset.x, offset.z);
        orbit.pitch = asinf(math_clamp(offset.y / orbit.distance, -1.0f, 1.0f));
        return(orbit);
    }

    IFB_INLINE void
    camera_tool_orbit_to_free(
        const camera_orbit& orbit) {

        const f32  pitch    = math_clamp(orbit.pitch,    -CAMERA_PITCH_LIMIT,  CAMERA_PITCH_LIMIT);
        const f32  distance = math_clamp(orbit.distance,  CAMERA_DISTANCE_MIN, CAMERA_DISTANCE_MAX);
        const vec3 offset   = vec3_from_yaw_pitch(orbit.yaw, pitch, distance);
        const vec3 origin   = {
            orbit.target.x + offset.x,
            orbit.target.y + offset.y,
            orbit.target.z + offset.z
        };

        eng_camera_free_set_target(orbit.target);
        eng_camera_free_set_origin(origin);
    }

    // drags only start over the game, not over a window, but they keep going if the mouse moves onto one
    IFB_INLINE void
    camera_tool_mouse(
        camera_orbit& orbit) {

        auto&       state = _camera_tool;
        const auto& io    = ImGui::GetIO();

        if (state.drag_button == -1 && !io.WantCaptureMouse) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                state.drag_button = ImGuiMouseButton_Right;
                state.is_panning  = io.KeyShift;
            }
            else if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                state.drag_button = ImGuiMouseButton_Middle;
                state.is_panning  = true;
            }
        }

        if (state.drag_button != -1 && !ImGui::IsMouseDown(state.drag_button)) {
            state.drag_button = -1;
        }

        // zoom towards the target
        if (!io.WantCaptureMouse && io.MouseWheel != 0.0f) {
            orbit.distance = math_clamp(orbit.distance * powf(CAMERA_ZOOM_STEP, io.MouseWheel), CAMERA_DISTANCE_MIN, CAMERA_DISTANCE_MAX);
        }

        if (state.drag_button == -1) return;

        const f32 dx = io.MouseDelta.x;
        const f32 dy = io.MouseDelta.y;
        if (dx == 0.0f && dy == 0.0f) return;

        if (state.is_panning) {
            camera_tool_pan(orbit, dx, dy);
        }
        else {
            orbit.yaw   -= dx * CAMERA_ORBIT_SPEED;
            orbit.pitch  = math_clamp(orbit.pitch + (dy * CAMERA_ORBIT_SPEED), -CAMERA_PITCH_LIMIT, CAMERA_PITCH_LIMIT);

            // keep the yaw in -pi to pi so the slider can show it
            if (orbit.yaw >  CAMERA_PI) orbit.yaw -= 2.0f * CAMERA_PI;
            if (orbit.yaw < -CAMERA_PI) orbit.yaw += 2.0f * CAMERA_PI;
        }
    }

    // moves the target across the screen, so whatever is at the target stays under the mouse
    IFB_INLINE void
    camera_tool_pan(
        camera_orbit& orbit,
        const f32     dx,
        const f32     dy) {

        const f32 screen_height = ImGui::GetIO().DisplaySize.y;
        if (screen_height <= 0.0f) return;

        const f32 half_fov       = eng_camera_free_get_fov() * (CAMERA_PI / 360.0f);
        const f32 units_per_px   = (2.0f * orbit.distance * tanf(half_fov)) / screen_height;
        const vec3 forward       = vec3_from_yaw_pitch(orbit.yaw, orbit.pitch, -1.0f);
        const vec3 right         = { cosf(orbit.yaw), 0.0f, -sinf(orbit.yaw) };
        const vec3 up            = vec3_normalize(vec3_cross(right, forward));

        // dragging right moves the scene right, so the camera goes left
        const f32 move_right = -dx * units_per_px;
        const f32 move_up    =  dy * units_per_px;
        orbit.target.x += (right.x * move_right) + (up.x * move_up);
        orbit.target.y += (right.y * move_right) + (up.y * move_up);
        orbit.target.z += (right.z * move_right) + (up.z * move_up);
    }

    IFB_INLINE void
    camera_tool_controls(
        camera_orbit& orbit) {

        auto& state = _camera_tool;

        ImGui::DragFloat3   ("Target",   &orbit.target.x, CAMERA_DRAG_SPEED, 0.0f, 0.0f, "%.3f");
        ImGui::SliderAngle  ("Yaw",      &orbit.yaw,   -180.0f, 180.0f);
        ImGui::SliderAngle  ("Pitch",    &orbit.pitch, -CAMERA_PITCH_LIMIT * (180.0f / CAMERA_PI), CAMERA_PITCH_LIMIT * (180.0f / CAMERA_PI));
        ImGui::DragFloat    ("Distance", &orbit.distance, CAMERA_DRAG_SPEED, CAMERA_DISTANCE_MIN, CAMERA_DISTANCE_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

        // with keep framing on, changing the fov moves the camera along the line to the target,
        // so whatever is at the target stays the same size on screen
        const f32 fov_prev = eng_camera_free_get_fov();
        f32       fov      = fov_prev;
        ImGui::SliderFloat ("FOV Y",        &fov, 10.0f, 90.0f, "%.1f deg");
        ImGui::Checkbox    ("Keep Framing", &state.keep_framing);

        if (fov != fov_prev) {
            if (state.keep_framing) {
                const f32 deg_to_half_rad = CAMERA_PI / 360.0f;
                orbit.distance *= tanf(fov_prev * deg_to_half_rad) / tanf(fov * deg_to_half_rad);
                orbit.distance  = math_clamp(orbit.distance, CAMERA_DISTANCE_MIN, CAMERA_DISTANCE_MAX);
            }
            eng_camera_free_set_fov(fov);
        }
    }

    IFB_INLINE void
    camera_tool_vectors(
        const vec3& origin,
        const vec3& target,
        const mat4& view) {

        const vec3 forward = xform_view_forward (view);
        const vec3 right   = xform_view_right   (view);
        const vec3 up      = xform_view_up      (view);

        if (!ImGui::BeginTable("tbl-camera-vectors", 4, ImGuiTableFlags_Borders)) return;

        ImGui::TableSetupColumn("Vector");
        ImGui::TableSetupColumn("X");
        ImGui::TableSetupColumn("Y");
        ImGui::TableSetupColumn("Z");
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

        ImGui::TableNextColumn();
        ImGui::TableHeader("Vector");
        camera_tool_axis_header("X", IM_COL32(0xCC,0x24,0x1D,0xFF));
        camera_tool_axis_header("Y", IM_COL32(0x98,0x97,0x1A,0xFF));
        camera_tool_axis_header("Z", IM_COL32(0x45,0x85,0x88,0xFF));

        const char* names   [] = {"Origin", "Target", "Forward", "Right", "Up"};
        const vec3* vectors [] = {&origin,  &target,  &forward,  &right,  &up};
        for (
            u32 index = 0;
                index < 5;
              ++index) {

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::TextUnformatted(names[index]);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", vectors[index]->x);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", vectors[index]->y);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", vectors[index]->z);
        }

        ImGui::EndTable();
    }

    IFB_INLINE void
    camera_tool_view_matrix(
        const mat4& view) {

        const ImGuiTableFlags flags = (
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg
        );

        if (!ImGui::BeginTable("tbl-cam-view-matrix", 4, flags)) return;

        const f32 rows[4][4] = {
            { view.r0c0, view.r0c1, view.r0c2, view.r0c3 },
            { view.r1c0, view.r1c1, view.r1c2, view.r1c3 },
            { view.r2c0, view.r2c1, view.r2c2, view.r2c3 },
            { view.r3c0, view.r3c1, view.r3c2, view.r3c3 },
        };

        for (
            u32 row = 0;
                row < 4;
              ++row) {

            ImGui::TableNextRow();
            for (
                u32 col = 0;
                    col < 4;
                  ++col) {

                ImGui::TableNextColumn();
                ImGui::Text("%.4f", rows[row][col]);
            }
        }

        ImGui::EndTable();
    }

    IFB_INLINE void
    camera_tool_axis_header(
        const char* label,
        const ImU32 color) {

        ImGui::TableNextColumn ();
        ImGui::TableSetBgColor (ImGuiTableBgTarget_CellBg, color);
        ImGui::PushStyleColor  (ImGuiCol_Header,           color);
        ImGui::PushStyleColor  (ImGuiCol_HeaderHovered,    color);
        ImGui::PushStyleColor  (ImGuiCol_Text,             IM_COL32(0x28,0x28,0x28,0xFF));
        ImGui::TableHeader     (label);
        ImGui::PopStyleColor   (3);
    }
};
