#pragma once

#include "ifb-engine.hpp"
#include "ifb-gui.hpp"
#include "imgui.h"
#include <math.h>

namespace ifb {

    enum gui_audio_wave_e {
        gui_audio_wave_e_square   = 0,
        gui_audio_wave_e_sawtooth = 1,
        gui_audio_wave_e_sine     = 2,
        gui_audio_wave_e_noise    = 3
    };

    static constexpr u32 GUI_AUDIO_SCOPE_SAMPLES = 1024;

    // white noise from a xorshift generator, eng_random_f32 reads from a table
    // that's small enough to hear it repeat at this sample rate
    IFB_INTERNAL f32
    gui_audio_noise_sample(
        void) {

        static u32 state = 0x9E3779B9;

        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;

        return(((f32)state / 2147483648.0f) - 1.0f);
    }

    // phase is from 0 to 1 over one cycle
    IFB_INTERNAL f32
    gui_audio_wave_sample(
        const gui_audio_wave_e wave,
        const f32              phase) {

        switch (wave) {
            case (gui_audio_wave_e_square):   return((phase < 0.5f) ? 1.0f : -1.0f);
            case (gui_audio_wave_e_sawtooth): return((2.0f * phase) - 1.0f);
            case (gui_audio_wave_e_sine):     return(sinf(phase * 6.28318531f));
            case (gui_audio_wave_e_noise):    return(gui_audio_noise_sample());
        }
        return(0.0f);
    }

    IFB_INTERNAL void
    gui_engine_audio_test(
        bool& is_open) {

        // the phase and gain carry over between frames, a jump in either is an audible click
        static gui_audio_wave_e wave        = gui_audio_wave_e_sine;
        static bool             is_playing  = false;
        static f32              frequency   = 440.0f;
        static f32              volume      = 0.1f;
        static f32              phase       = 0.0f;
        static f32              gain        = 0.0f;
        static f32              scope[GUI_AUDIO_SCOPE_SAMPLES];
        static u32              scope_write = 0;

        // keep writing while the window closes, so the wave fades out instead of cutting off
        const bool is_audible = (is_playing && is_open);

        audio_buffer buffer;
        if (eng_audio_get_buffer(buffer) && (is_audible || gain > 0.0f)) {

            const f32 phase_step  = frequency / (f32)config::audio_sample_rate;
            const f32 gain_target = is_audible ? volume : 0.0f;
            const f32 gain_step   = 1.0f / ((f32)config::audio_sample_rate * GUI_AUDIO_FADE_SECONDS);

            for (
                u32 i_frame = 0;
                    i_frame < buffer.frame_count;
                  ++i_frame) {

                if      (gain < gain_target) { gain += gain_step; if (gain > gain_target) gain = gain_target; }
                else if (gain > gain_target) { gain -= gain_step; if (gain < gain_target) gain = gain_target; }

                const f32 sample = gui_audio_wave_sample(wave, phase) * gain;

                phase += phase_step;
                if (phase >= 1.0f) phase -= 1.0f;

                for (
                    u32 i_channel = 0;
                        i_channel < config::audio_channel_count;
                      ++i_channel) {

                    buffer.samples[(i_frame * config::audio_channel_count) + i_channel] = sample;
                }

                scope[scope_write] = sample;
                scope_write        = (scope_write + 1) % GUI_AUDIO_SCOPE_SAMPLES;
            }
        }

        if (!is_open) return;

        audio_info info;
        eng_audio_get_info(info);

        if (ImGui::Begin(_gui_str->window_engine_audio_test, &is_open)) {

            if (!info.is_device_ready) {
                ImGui::TextUnformatted("No audio device");
            }

            ImGui::SeparatorText("Wave");

            if (ImGui::Button(is_playing ? "Stop" : "Play")) {
                is_playing = !is_playing;
            }

            int wave_index = (int)wave;
            ImGui::Combo("Wave", &wave_index, "Square\0Sawtooth\0Sine\0Noise\0");
            wave = (gui_audio_wave_e)wave_index;

            ImGui::SliderFloat("Frequency", &frequency, 20.0f, 2000.0f, "%.0f Hz", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("Volume",    &volume,    0.0f,  0.5f,    "%.2f");

            // the plot reads from the oldest sample to the newest
            ImGui::PlotLines(
                "##audio-scope",
                scope,
                GUI_AUDIO_SCOPE_SAMPLES,
                scope_write,
                NULL,
                -1.0f,
                1.0f,
                ImVec2(-1.0f, 96.0f)
            );

            // the group volumes are the engine's, so they're read back every frame
            ImGui::SeparatorText("Groups");

            f32 master_volume = eng_audio_get_master_volume();
            if (ImGui::SliderFloat("Master", &master_volume, 0.0f, 1.0f, "%.2f")) {
                eng_audio_set_master_volume(master_volume);
            }

            static const char* group_names[audio_group_e_count] = { "SFX", "Music" };
            for (
                u32 group = 0;
                    group < audio_group_e_count;
                  ++group) {

                f32 group_volume = eng_audio_group_get_volume((audio_group_e)group);
                if (ImGui::SliderFloat(group_names[group], &group_volume, 0.0f, 1.0f, "%.2f")) {
                    eng_audio_group_set_volume((audio_group_e)group, group_volume);
                }
            }

            ImGui::SeparatorText("Buffer");

            const f32 ms_per_frame = 1000.0f / (f32)config::audio_sample_rate;
            ImGui::Text("Format:         %u Hz, %u channels", config::audio_sample_rate, config::audio_channel_count);
            ImGui::Text("Target Latency: %.1f ms", config::audio_latency_frames * ms_per_frame);
            ImGui::Text("Queued:         %.1f ms", info.frames_queued     * ms_per_frame);
            ImGui::Text("Queued Min:     %.1f ms", info.frames_queued_min * ms_per_frame);
            ImGui::Text("Frame Gap:      %.1f ms", info.frame_gap_ms);
            ImGui::Text("Frame Gap Max:  %.1f ms", info.frame_gap_ms_max);
            ImGui::Text("Underruns:      %u",      info.underrun_count);
            ImGui::Text("Voices:         %u / %u", info.voice_count, config::audio_voice_capacity);
            ImGui::Text("Preloads:       %u / %u", info.preload_count, config::audio_preload_capacity);
            ImGui::Text("Streams:        %u / %u", info.stream_count, config::audio_stream_capacity);
            ImGui::Text("Chunk Reads:    %u",      info.stream_chunk_reads);
            ImGui::Text("Stream Waits:   %u",      info.stream_underruns);

            if (ImGui::Button("Reset Stats")) {
                eng_audio_reset_stats();
            }
        }
        ImGui::End();
    }
};
