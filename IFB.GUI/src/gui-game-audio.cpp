#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-game.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"
#include <math.h>
#include <stdio.h>

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 GAME_AUDIO_SCOPE_FRAMES     = 2048;   // about 43 ms at 48 kHz
    static constexpr f32 GAME_AUDIO_SCOPE_HEIGHT     = 72.0f;
    static constexpr f32 GAME_AUDIO_PEAK_HOLD_DECAY  = 0.5f;   // the peak marker falls this much a second
    static constexpr f32 GAME_AUDIO_METER_FLOOR_DB   = -60.0f;
    static constexpr f32 GAME_AUDIO_FIT_MIN          = 0.01f;  // fitting the scope to the peak never zooms in past this
    static constexpr f32 GAME_AUDIO_WINDOW_WIDTH     = 560.0f;
    static constexpr f32 GAME_AUDIO_WINDOW_HEIGHT    = 720.0f;

    static const char* _game_audio_group_names[audio_group_e_count] = { "SFX", "Music" };

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the scope's copy of the output, and the peak markers, which fall slowly so a short peak can be seen
    struct game_audio_window_state {
        f32 scope_left  [GAME_AUDIO_SCOPE_FRAMES];
        f32 scope_right [GAME_AUDIO_SCOPE_FRAMES];
        f32  peak_hold  [2];
        bool is_fit = true;   // the scope's scale follows the peak, so quiet sound can be seen
    } static _game_audio_window;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void game_audio_window_volumes (void);
    IFB_INLINE void game_audio_window_music   (game_audio* audio);
    IFB_INLINE void game_audio_window_effects (void);
    IFB_INLINE void game_audio_window_output  (void);
    IFB_INLINE void game_audio_window_meter   (const char* label, const f32 peak, f32& peak_hold);
    IFB_INLINE void game_audio_window_voices  (void);
    IFB_INLINE void game_audio_window_stats   (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_game_audio(
        bool&               is_open,
        const game_context* game_ctx) {

        if (!is_open) return;
        assert(game_ctx && game_ctx->audio);

        // wide enough for the voices table the first time it opens, then it's wherever it was left
        ImGui::SetNextWindowSize(ImVec2(GAME_AUDIO_WINDOW_WIDTH, GAME_AUDIO_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(_gui_str->window_game_audio, &is_open)) {

            audio_info info;
            eng_audio_get_info(info);
            if (!info.is_device_ready) {
                ImGui::TextUnformatted("No audio device, nothing will play");
            }

            game_audio_window_volumes ();
            game_audio_window_music   (game_ctx->audio);
            game_audio_window_effects ();
            game_audio_window_output  ();
            game_audio_window_voices  ();
            game_audio_window_stats   ();
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // the engine has the volumes, so they're read back every frame
    IFB_INLINE void
    game_audio_window_volumes(
        void) {

        ImGui::SeparatorText("Volume");

        f32 master = eng_audio_get_master_volume();
        if (ImGui::SliderFloat("Master", &master, 0.0f, 1.0f, "%.2f")) {
            eng_audio_set_master_volume(master);
        }

        for (
            u32 group = 0;
                group < audio_group_e_count;
              ++group) {

            f32 volume = eng_audio_group_get_volume((audio_group_e)group);
            if (ImGui::SliderFloat(_game_audio_group_names[group], &volume, 0.0f, 1.0f, "%.2f")) {
                eng_audio_group_set_volume((audio_group_e)group, volume);
            }
        }
    }

    // the game's music voice, a stopped one can be started again from here
    IFB_INLINE void
    game_audio_window_music(
        game_audio* audio) {

        ImGui::SeparatorText("Music");

        audio_voice_info info;
        const bool is_playing = eng_audio_voice_get_info(audio->h_music, info);

        if (!is_playing) {
            ImGui::TextUnformatted("Not playing");
            if (ImGui::Button("Play##music")) {
                audio->h_music = eng_audio_sound_stream("PIANO", audio_group_e_music, GAME_MUSIC_VOLUME, true);
            }
            if (audio->h_music == INVALID_HANDLE) {
                ImGui::SameLine();
                ImGui::TextDisabled("(it can't start without the sound bank, or with every stream in use)");
            }
            return;
        }

        const f32 duration = (f32)info.frame_count / (f32)info.sample_rate;
        ImGui::Text(
            "%s, %s, %s",
            info.name[0] != 0 ? info.name : "(a wav)",
            info.is_streaming ? "streamed" : "loaded",
            info.is_stopping  ? "stopping" : (info.is_paused ? "paused" : "playing")
        );

        if (ImGui::Button(info.is_paused ? "Resume##music" : "Pause##music")) {
            eng_audio_voice_set_paused(audio->h_music, !info.is_paused);
        }

        ImGui::SameLine();
        if (ImGui::Button("Restart##music")) {
            eng_audio_voice_seek       (audio->h_music, 0.0);
            eng_audio_voice_set_paused (audio->h_music, false);
        }

        // it fades out and the handle goes stale, then it can be started again
        ImGui::SameLine();
        if (ImGui::Button("Stop##music")) {
            eng_audio_voice_stop(audio->h_music);
        }

        ImGui::SameLine();
        bool is_looping = info.is_looping;
        if (ImGui::Checkbox("Loop##music", &is_looping)) {
            eng_audio_voice_set_looping(audio->h_music, is_looping);
        }

        f32 seconds = (f32)(info.position / (f64)info.sample_rate);
        if (ImGui::SliderFloat("Position##music", &seconds, 0.0f, duration, "%.1f s")) {
            eng_audio_voice_seek(audio->h_music, (f64)seconds * (f64)info.sample_rate);
        }

        f32 volume = info.volume;
        if (ImGui::SliderFloat("Volume##music", &volume, 0.0f, 1.0f, "%.2f")) {
            eng_audio_voice_set_volume(audio->h_music, volume);
        }
    }

    // plays the sound the game plays, without the bump's debounce
    IFB_INLINE void
    game_audio_window_effects(
        void) {

        ImGui::SeparatorText("Sound Effects");

        if (ImGui::Button("Play Boop")) {
            (void)eng_audio_sound_play("BOOP", audio_group_e_sfx, GAME_BOOP_VOLUME, false);
        }
    }

    // what was sent to the device, after the mix and the master volume
    IFB_INLINE void
    game_audio_window_output(
        void) {

        auto& state = _game_audio_window;

        ImGui::SeparatorText("Output");

        const u32 frames = eng_audio_get_output(state.scope_left, state.scope_right, GAME_AUDIO_SCOPE_FRAMES);

        f32 peak[2] = { 0.0f, 0.0f };
        for (
            u32 i_frame = 0;
                i_frame < frames;
              ++i_frame) {

            peak[0] = fmaxf(peak[0], fabsf(state.scope_left [i_frame]));
            peak[1] = fmaxf(peak[1], fabsf(state.scope_right[i_frame]));
        }

        game_audio_window_meter("L", peak[0], state.peak_hold[0]);
        game_audio_window_meter("R", peak[1], state.peak_hold[1]);

        // fit scales both channels the same, so they can still be compared
        ImGui::Checkbox("Fit to the peak", &state.is_fit);
        const f32 range = state.is_fit ? fmaxf(fmaxf(peak[0], peak[1]), GAME_AUDIO_FIT_MIN) : 1.0f;
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", state.is_fit ? "scaled" : "-1 to 1");

        ImGui::PlotLines("##scope-left",  state.scope_left,  (int)frames, 0, "Left",  -range, range, ImVec2(-1.0f, GAME_AUDIO_SCOPE_HEIGHT));
        ImGui::PlotLines("##scope-right", state.scope_right, (int)frames, 0, "Right", -range, range, ImVec2(-1.0f, GAME_AUDIO_SCOPE_HEIGHT));
    }

    // a bar from GAME_AUDIO_METER_FLOOR_DB to 0 dB, with a marker that holds the peak and falls slowly
    IFB_INLINE void
    game_audio_window_meter(
        const char* label,
        const f32   peak,
        f32&        peak_hold) {

        const f32 dt = ImGui::GetIO().DeltaTime;
        peak_hold = fmaxf(peak, peak_hold - (GAME_AUDIO_PEAK_HOLD_DECAY * dt));

        // the bar is in decibels, so quiet sound still shows
        auto to_fraction = [](const f32 level) {
            if (level <= 0.0f) return(0.0f);
            const f32 db = 20.0f * log10f(level);
            const f32 t  = 1.0f - (db / GAME_AUDIO_METER_FLOOR_DB);
            return((t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t);
        };

        char overlay[32];
        if (peak > 0.0f) snprintf(overlay, sizeof(overlay), "%.1f dB%s", 20.0f * log10f(peak), (peak >= 1.0f) ? "  CLIP" : "");
        else             snprintf(overlay, sizeof(overlay), "silent");

        ImGui::TextUnformatted(label);
        ImGui::SameLine();

        // a full scale peak turns the bar red
        const bool is_clipping = (peak >= 1.0f);
        if (is_clipping) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(220, 60, 60, 255));
        ImGui::ProgressBar(to_fraction(peak), ImVec2(-1.0f, 0.0f), overlay);
        if (is_clipping) ImGui::PopStyleColor();

        const ImVec2 bar_min = ImGui::GetItemRectMin();
        const ImVec2 bar_max = ImGui::GetItemRectMax();
        const f32    x       = bar_min.x + ((bar_max.x - bar_min.x) * to_fraction(peak_hold));
        ImGui::GetWindowDrawList()->AddLine(ImVec2(x, bar_min.y), ImVec2(x, bar_max.y), IM_COL32(255, 255, 255, 255), 2.0f);
    }

    // every voice playing, sound effects, music, and the engine's tools
    IFB_INLINE void
    game_audio_window_voices(
        void) {

        hnd_audio_voice handles[IFB_CONFIG_AUDIO_VOICE_CAPACITY];
        const u32 count = eng_audio_get_voices(handles, IFB_CONFIG_AUDIO_VOICE_CAPACITY);

        ImGui::SeparatorText("Voices");

        if (count == 0) {
            ImGui::TextUnformatted("Nothing is playing");
            return;
        }

        if (ImGui::BeginTable("tbl-game-audio-voices", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {

            ImGui::TableSetupColumn ("Sound");
            ImGui::TableSetupColumn ("Source", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn ("Group",  ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn ("State",  ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn ("Position");
            ImGui::TableSetupColumn ("Volume", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn ("Gain",   ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow  ();

            for (
                u32 index = 0;
                    index < count;
                  ++index) {

                audio_voice_info info;
                if (!eng_audio_voice_get_info(handles[index], info)) continue;

                const f32 seconds  = (f32)(info.position / (f64)info.sample_rate);
                const f32 duration = (f32)info.frame_count / (f32)info.sample_rate;
                const char* state  = info.is_stopping ? "stopping" : (info.is_paused ? "paused" : (info.is_looping ? "looping" : "playing"));

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(info.name[0] != 0 ? info.name : "(a wav)");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(info.is_streaming ? "stream" : "memory");
                ImGui::TableNextColumn(); ImGui::TextUnformatted((info.group.val < audio_group_e_count) ? _game_audio_group_names[info.group.val] : "?");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(state);
                ImGui::TableNextColumn();
                {
                    char overlay[32];
                    snprintf(overlay, sizeof(overlay), "%.1f / %.1f s", seconds, duration);
                    ImGui::ProgressBar((duration > 0.0f) ? (seconds / duration) : 0.0f, ImVec2(-1.0f, 0.0f), overlay);
                }
                ImGui::TableNextColumn(); ImGui::Text("%.2f", info.volume);
                ImGui::TableNextColumn(); ImGui::Text("%.2f", info.gain);
            }

            ImGui::EndTable();
        }
    }

    IFB_INLINE void
    game_audio_window_stats(
        void) {

        audio_info info;
        eng_audio_get_info(info);

        if (!ImGui::CollapsingHeader("Stats")) return;

        const f32 ms_per_frame = 1000.0f / (f32)config::audio_sample_rate;
        ImGui::Text("Voices:          %u / %u", info.voice_count,   config::audio_voice_capacity);
        ImGui::Text("Streams:         %u / %u", info.stream_count,  config::audio_stream_capacity);
        ImGui::Text("Preloads:        %u / %u", info.preload_count, config::audio_preload_capacity);
        ImGui::Text("Queued:          %.1f ms (lowest %.1f ms)", info.frames_queued * ms_per_frame, info.frames_queued_min * ms_per_frame);
        ImGui::Text("Device Underruns %u", info.underrun_count);
        ImGui::Text("Chunk Reads:     %u", info.stream_chunk_reads);
        ImGui::Text("Stream Waits:    %u", info.stream_underruns);
        ImGui::Text("Clipped Samples: %u", info.output_clips);

        if (ImGui::Button("Reset Stats")) {
            eng_audio_reset_stats();
        }
    }
};
