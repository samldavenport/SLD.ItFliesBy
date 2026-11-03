#pragma once

#include "ifb-config.hpp"
#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "ifb.hpp"
#include "imgui.h"

namespace ifb {

    //--------------------------------------------------------------------
    // CONSTANTS
    //--------------------------------------------------------------------

    static constexpr u32 WAV_PLAYER_PATH_SIZE        = IFB_CONFIG_FILE_PATH_SIZE;
    static constexpr u32 WAV_PLAYER_PEAK_COUNT       = 512;
    static constexpr u32 WAV_PLAYER_PEAK_READ_FRAMES = 64;
    static constexpr f32 WAV_PLAYER_PLOT_HEIGHT      = 96.0f;

    static const char* _wav_player_filter_type = "*.wav";
    static const char* _wav_player_filter_desc = "WAV Audio";

    //--------------------------------------------------------------------
    // DEFINITIONS
    //--------------------------------------------------------------------

    // the audio manager plays the file, this keeps the settings for the next voice
    // and the position while there's no voice, like after stopping
    struct wav_player_state {
        const wav*      w;
        hnd_arena       h_arena; // only valid when there's a wav
        hnd_audio_voice h_voice = {INVALID_HANDLE};
        const char*     status;
        char            path [WAV_PLAYER_PATH_SIZE];
        u32             frame_count;
        u32             peak_count;
        f64             position;
        f32             volume = 0.5f;
        bool            is_playing;
        bool            is_looping;
        f32             peaks[WAV_PLAYER_PEAK_COUNT];
    } static _wav_player;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void wav_player_sync         (void);
    IFB_INLINE void wav_player_open         (void);
    IFB_INLINE void wav_player_close        (void);
    IFB_INLINE void wav_player_play         (void);
    IFB_INLINE void wav_player_build_peaks  (void);
    IFB_INLINE void wav_player_controls     (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_wav_player(
        bool& is_open) {

        auto& state = _wav_player;

        wav_player_sync();

        // closing the window pauses, the voice fades out and keeps its place
        if (!is_open) {
            if (state.is_playing) {
                eng_audio_voice_set_paused(state.h_voice, true);
                state.is_playing = false;
            }
            return;
        }

        if (ImGui::Begin(_gui_str->window_engine_wav_player, &is_open)) {

            audio_info info;
            eng_audio_get_info(info);
            if (!info.is_device_ready) {
                ImGui::TextUnformatted("No audio device");
            }

            if (ImGui::Button("Open...")) {
                wav_player_open();
            }

            ImGui::SameLine();
            ImGui::BeginDisabled(state.w == NULL);
            if (ImGui::Button("Close")) {
                wav_player_close();
            }
            ImGui::EndDisabled();

            if (state.status) {
                ImGui::TextUnformatted(state.status);
            }

            if (state.w) {
                wav_player_controls();
            }
        }
        ImGui::End();
    }

    //--------------------------------------------------------------------
    // INLINE METHOD DEFINITIONS
    //--------------------------------------------------------------------

    // reads the voice back from the audio manager, it's freed when it reaches the end of the file
    IFB_INLINE void
    wav_player_sync(
        void) {

        auto& state = _wav_player;
        if (state.h_voice == INVALID_HANDLE) return;

        audio_voice_info info;
        if (eng_audio_voice_get_info(state.h_voice, info)) {
            state.position   = info.position;
            state.is_playing = !info.is_paused;
        }
        else {
            state.h_voice    = {INVALID_HANDLE};
            state.position   = 0.0;
            state.is_playing = false;
        }
    }

    // a failed open leaves the open file playing
    IFB_INLINE void
    wav_player_open(
        void) {

        auto& state = _wav_player;

        const char* path = eng_file_dialog_open(_wav_player_filter_type, _wav_player_filter_desc);
        if (!path) return;

        const hnd_arena h_arena = eng_arena_large_alloc();
        if (h_arena == INVALID_HANDLE) {
            state.status = "Failed to open the file, there's no memory for it";
            return;
        }

        const wav* w = eng_wav_load_to_arena(path, h_arena);
        if (!w) {
            eng_arena_free(h_arena);
            state.status = "Failed to open the file, it may not be PCM (8, 16, 24, or 32 bit) or it's too big";
            return;
        }

        if (state.w) {
            wav_player_close();
        }

        strncpy_s(state.path, path, WAV_PLAYER_PATH_SIZE - 1);
        state.w           = w;
        state.h_arena     = h_arena;
        state.frame_count = eng_wav_get_frame_count(w);
        state.status      = "Opened the file";

        wav_player_build_peaks();
    }

    // the voices have to go before the memory they read, including one that's still fading out after a stop
    IFB_INLINE void
    wav_player_close(
        void) {

        auto& state = _wav_player;

        eng_audio_voice_stop_all_using(state.w);
        eng_arena_free(state.h_arena);

        state.w          = NULL;
        state.h_voice    = {INVALID_HANDLE};
        state.path[0]    = 0;
        state.position   = 0.0;
        state.is_playing = false;
        state.status     = NULL;
    }

    // resumes the paused voice, or starts a new one where the position is
    IFB_INLINE void
    wav_player_play(
        void) {

        auto& state = _wav_player;

        if (state.h_voice != INVALID_HANDLE) {
            eng_audio_voice_set_paused(state.h_voice, false);
            state.is_playing = true;
            return;
        }

        state.h_voice = eng_audio_voice_play(state.w, audio_group_e_sfx, state.volume, state.is_looping);
        if (state.h_voice == INVALID_HANDLE) {
            state.status = "Failed to play the file, there's no audio device or every voice is in use";
            return;
        }

        eng_audio_voice_seek(state.h_voice, state.position);
        state.is_playing = true;
    }

    // the loudest sample in each slice of the file, for the waveform
    // only part of each slice is read, it's enough to draw and a long file stays quick to open
    IFB_INLINE void
    wav_player_build_peaks(
        void) {

        auto& state = _wav_player;

        state.peak_count = (state.frame_count < WAV_PLAYER_PEAK_COUNT)
            ? state.frame_count
            : WAV_PLAYER_PEAK_COUNT;

        const u32 channel_count = state.w->format.num_channels;

        for (
            u32 i_peak = 0;
                i_peak < state.peak_count;
              ++i_peak) {

            const u32 frame_start = (u32)(((u64)i_peak       * state.frame_count) / state.peak_count);
            const u32 frame_end   = (u32)(((u64)(i_peak + 1) * state.frame_count) / state.peak_count);
            const u32 stride      = ((frame_end - frame_start) / WAV_PLAYER_PEAK_READ_FRAMES) + 1;

            f32 peak = 0.0f;
            for (
                u32 i_frame = frame_start;
                    i_frame < frame_end;
                    i_frame += stride) {

                for (
                    u32 i_channel = 0;
                        i_channel < channel_count;
                      ++i_channel) {

                    f32 sample = eng_wav_get_sample(state.w, i_frame, i_channel);
                    if (sample < 0.0f) sample = -sample;
                    if (sample > peak) peak   = sample;
                }
            }
            state.peaks[i_peak] = peak;
        }
    }

    IFB_INLINE void
    wav_player_controls(
        void) {

        auto& state = _wav_player;

        const wav_format& format   = state.w->format;
        const f32         duration = (f32)state.frame_count / (f32)format.sample_rate;

        ImGui::SeparatorText("File");

        ImGui::TextUnformatted(state.path);
        ImGui::Text("%u Hz, %u bit, %u channels", format.sample_rate, format.bits_per_sample, format.num_channels);
        ImGui::Text("%.2f seconds, %.1f MB", duration, (f32)state.w->data.size / (1024.0f * 1024.0f));

        ImGui::SeparatorText("Playback");

        if (ImGui::Button(state.is_playing ? "Pause" : "Play")) {
            if (state.is_playing) {
                eng_audio_voice_set_paused(state.h_voice, true);
                state.is_playing = false;
            }
            else {
                wav_player_play();
            }
        }

        // the voice fades out on its own, so the next play gets a new one from the start
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            eng_audio_voice_stop(state.h_voice);
            state.h_voice    = {INVALID_HANDLE};
            state.position   = 0.0;
            state.is_playing = false;
        }

        ImGui::SameLine();
        if (ImGui::Checkbox("Loop", &state.is_looping)) {
            eng_audio_voice_set_looping(state.h_voice, state.is_looping);
        }

        ImGui::PlotHistogram(
            "##wav-player-peaks",
            state.peaks,
            state.peak_count,
            0,
            NULL,
            0.0f,
            1.0f,
            ImVec2(-1.0f, WAV_PLAYER_PLOT_HEIGHT)
        );

        // the playhead goes on top of the waveform
        const ImVec2 plot_min = ImGui::GetItemRectMin();
        const ImVec2 plot_max = ImGui::GetItemRectMax();
        const f32    x        = plot_min.x + ((plot_max.x - plot_min.x) * (f32)(state.position / (f64)state.frame_count));
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(x, plot_min.y),
            ImVec2(x, plot_max.y),
            IM_COL32(255, 255, 255, 255)
        );

        f32 seconds = (f32)(state.position / (f64)format.sample_rate);
        if (ImGui::SliderFloat("Position", &seconds, 0.0f, duration, "%.2f s")) {
            state.position = (f64)seconds * (f64)format.sample_rate;
            eng_audio_voice_seek(state.h_voice, state.position);
        }

        if (ImGui::SliderFloat("Volume", &state.volume, 0.0f, 1.0f, "%.2f")) {
            eng_audio_voice_set_volume(state.h_voice, state.volume);
        }
    }
};
