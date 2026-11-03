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

    // the position and gain carry over between frames, a jump in either is an audible click
    struct wav_player_state {
        const wav*  w;
        hnd_arena   h_arena; // only valid when there's a wav
        const char* status;
        char        path [WAV_PLAYER_PATH_SIZE];
        u32         frame_count;
        u32         peak_count;
        f64         position; // in frames of the file, which may not be at the device's sample rate
        f32         gain;
        f32         volume = 0.5f;
        bool        is_playing;
        bool        is_looping;
        bool        is_rewinding; // stopped, and goes back to the start when the fade out is done
        f32         peaks[WAV_PLAYER_PEAK_COUNT];
    } static _wav_player;

    //--------------------------------------------------------------------
    // INLINE METHOD DECLARATIONS
    //--------------------------------------------------------------------

    IFB_INLINE void wav_player_mix          (const bool is_open);
    IFB_INLINE void wav_player_open         (void);
    IFB_INLINE void wav_player_close        (void);
    IFB_INLINE void wav_player_build_peaks  (void);
    IFB_INLINE void wav_player_controls     (void);

    //--------------------------------------------------------------------
    // INTERNAL METHOD DEFINITIONS
    //--------------------------------------------------------------------

    IFB_INTERNAL void
    gui_engine_wav_player(
        bool& is_open) {

        auto& state = _wav_player;

        // keep writing while the window closes, so the sound fades out instead of cutting off
        if (!is_open) state.is_playing = false;
        wav_player_mix(is_open);

        if (!is_open) return;

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

    // adds to the buffer, so this can play on top of the audio test
    IFB_INLINE void
    wav_player_mix(
        const bool is_open) {

        auto& state = _wav_player;
        if (!state.w) return;

        const bool is_audible = (state.is_playing && is_open);

        audio_buffer buffer;
        if (eng_audio_get_buffer(buffer) && (is_audible || state.gain > 0.0f)) {

            const wav_format& format      = state.w->format;
            const f64         step        = (f64)format.sample_rate / (f64)buffer.sample_rate;
            const f32         gain_target = is_audible ? state.volume : 0.0f;
            const f32         gain_step   = 1.0f / ((f32)buffer.sample_rate * GUI_AUDIO_FADE_SECONDS);

            for (
                u32 i_frame = 0;
                    i_frame < buffer.frame_count;
                  ++i_frame) {

                if      (state.gain < gain_target) { state.gain += gain_step; if (state.gain > gain_target) state.gain = gain_target; }
                else if (state.gain > gain_target) { state.gain -= gain_step; if (state.gain < gain_target) state.gain = gain_target; }

                // the fade out is done
                if (!is_audible && state.gain <= 0.0f) break;

                // the end of the file
                if (state.position >= (f64)state.frame_count) {
                    if (state.is_looping) {
                        state.position -= (f64)state.frame_count;
                    }
                    else {
                        state.position   = 0.0;
                        state.gain       = 0.0f;
                        state.is_playing = false;
                        break;
                    }
                }

                // the file's sample rate may not match the device, so this blends the 2 closest frames
                const u32 frame_a = (u32)state.position;
                const f32 blend   = (f32)(state.position - (f64)frame_a);
                u32       frame_b = frame_a + 1;
                if (frame_b >= state.frame_count) {
                    frame_b = state.is_looping ? 0 : frame_a;
                }

                for (
                    u32 i_channel = 0;
                        i_channel < buffer.channel_count;
                      ++i_channel) {

                    // mono goes to every channel, extra channels in the file are ignored
                    const u32 file_channel = (i_channel < format.num_channels) ? i_channel : (u32)(format.num_channels - 1);

                    const f32 sample_a = eng_wav_get_sample(state.w, frame_a, file_channel);
                    const f32 sample_b = eng_wav_get_sample(state.w, frame_b, file_channel);
                    const f32 sample   = (sample_a + ((sample_b - sample_a) * blend)) * state.gain;

                    f32& out = buffer.samples[(i_frame * buffer.channel_count) + i_channel];
                    out += sample;
                    if      (out >  1.0f) out =  1.0f;
                    else if (out < -1.0f) out = -1.0f;
                }

                state.position += step;
            }
        }

        if (state.is_rewinding && state.gain <= 0.0f) {
            state.position     = 0.0;
            state.is_rewinding = false;
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
            eng_arena_free(state.h_arena);
        }

        strncpy_s(state.path, path, WAV_PLAYER_PATH_SIZE - 1);
        state.w            = w;
        state.h_arena      = h_arena;
        state.frame_count  = eng_wav_get_frame_count(w);
        state.position     = 0.0;
        state.gain         = 0.0f;
        state.is_playing   = false;
        state.is_rewinding = false;
        state.status       = "Opened the file";

        wav_player_build_peaks();
    }

    IFB_INLINE void
    wav_player_close(
        void) {

        auto& state = _wav_player;

        eng_arena_free(state.h_arena);

        state.w            = NULL;
        state.path[0]      = 0;
        state.position     = 0.0;
        state.gain         = 0.0f;
        state.is_playing   = false;
        state.is_rewinding = false;
        state.status       = NULL;
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
            state.is_playing = !state.is_playing;
            if (state.is_playing && state.is_rewinding) {
                state.position     = 0.0;
                state.is_rewinding = false;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            state.is_playing   = false;
            state.is_rewinding = true;
        }

        ImGui::SameLine();
        ImGui::Checkbox("Loop", &state.is_looping);

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
            state.position     = (f64)seconds * (f64)format.sample_rate;
            state.is_rewinding = false;
        }

        ImGui::SliderFloat("Volume", &state.volume, 0.0f, 1.0f, "%.2f");
    }
};
