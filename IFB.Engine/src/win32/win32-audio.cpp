#pragma once

#include "ifb-platform.hpp"
#include "ifb-win32.hpp"
#include <mmdeviceapi.h>
#include <audioclient.h>

namespace ifb {

    // the device takes interleaved 32 bit floats, and windows converts them to the mix format
    static constexpr u16  WIN32_AUDIO_BITS_PER_SAMPLE = 32;
    static constexpr u32  WIN32_AUDIO_HNS_PER_MS      = 10000; // REFERENCE_TIME is in 100 nanosecond units

    struct win32_audio {
        IMMDevice*           device;
        IAudioClient*        client;
        IAudioRenderClient*  render;
        u32                  channel_count;
        u32                  frame_capacity;
        bool                 is_started;
    } static _win32_audio;

    //--------------------------------------------------------------------
    // API METHODS
    //--------------------------------------------------------------------

    // false means there's no usable device, the game runs silent
    IFB_PLATFORM_API bool
    pfm_audio_init(
        const pfm_audio_config& cfg) {

        assert(cfg.sample_rate   != 0);
        assert(cfg.channel_count != 0);
        assert(cfg.buffer_ms     != 0);
        assert(_win32_audio.client == NULL);

        IMMDeviceEnumerator* enumerator = NULL;
        HRESULT hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            NULL,
            CLSCTX_ALL,
            IID_PPV_ARGS(&enumerator)
        );
        if (FAILED(hr)) return(false);

        // there's no default device if nothing is plugged in
        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_win32_audio.device);
        enumerator->Release();
        if (FAILED(hr)) {
            _win32_audio.device = NULL;
            return(false);
        }

        hr = _win32_audio.device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            NULL,
            (void**)&_win32_audio.client
        );

        WAVEFORMATEX format;
        format.wFormatTag      = WAVE_FORMAT_IEEE_FLOAT;
        format.nChannels       = (WORD)cfg.channel_count;
        format.nSamplesPerSec  = cfg.sample_rate;
        format.wBitsPerSample  = WIN32_AUDIO_BITS_PER_SAMPLE;
        format.nBlockAlign     = (WORD)(cfg.channel_count * (WIN32_AUDIO_BITS_PER_SAMPLE / 8));
        format.nAvgBytesPerSec = cfg.sample_rate * format.nBlockAlign;
        format.cbSize          = 0;

        // polled, the frame loop fills the buffer instead of waiting on a device event
        if (SUCCEEDED(hr)) {
            const REFERENCE_TIME buffer_duration = (REFERENCE_TIME)cfg.buffer_ms * WIN32_AUDIO_HNS_PER_MS;
            hr = _win32_audio.client->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
                buffer_duration,
                0,
                &format,
                NULL
            );
        }

        UINT32 frame_capacity = 0;
        if (SUCCEEDED(hr)) hr = _win32_audio.client->GetBufferSize(&frame_capacity);
        if (SUCCEEDED(hr)) hr = _win32_audio.client->GetService(IID_PPV_ARGS(&_win32_audio.render));

        if (FAILED(hr)) {
            pfm_audio_shutdown();
            return(false);
        }

        _win32_audio.channel_count  = cfg.channel_count;
        _win32_audio.frame_capacity = frame_capacity;
        _win32_audio.is_started     = false;
        return(true);
    }

    IFB_PLATFORM_API void
    pfm_audio_shutdown(
        void) {

        if (_win32_audio.is_started) _win32_audio.client->Stop();
        if (_win32_audio.render)     _win32_audio.render->Release();
        if (_win32_audio.client)     _win32_audio.client->Release();
        if (_win32_audio.device)     _win32_audio.device->Release();

        _win32_audio = {};
    }

    IFB_PLATFORM_API u32
    pfm_audio_get_frame_capacity(
        void) {

        return(_win32_audio.frame_capacity);
    }

    // false means the device is gone, like when headphones get unplugged
    IFB_PLATFORM_API bool
    pfm_audio_get_frames_queued(
        u32& frames_queued) {

        assert(_win32_audio.client);

        UINT32 padding = 0;
        const HRESULT hr = _win32_audio.client->GetCurrentPadding(&padding);
        if (FAILED(hr)) return(false);

        frames_queued = padding;
        return(true);
    }

    // playback starts on the first write, so the device doesn't begin with an empty buffer
    IFB_PLATFORM_API bool
    pfm_audio_write(
        const f32* samples,
        const u32  frame_count) {

        assert(_win32_audio.render);
        assert(samples);

        if (frame_count == 0) return(true);

        BYTE* device_buffer = NULL;
        HRESULT hr = _win32_audio.render->GetBuffer(frame_count, &device_buffer);
        if (FAILED(hr)) return(false);

        const u32 size = frame_count * _win32_audio.channel_count * sizeof(f32);
        memcpy(device_buffer, samples, size);

        hr = _win32_audio.render->ReleaseBuffer(frame_count, 0);
        if (FAILED(hr)) return(false);

        if (!_win32_audio.is_started) {
            hr = _win32_audio.client->Start();
            if (FAILED(hr)) return(false);
            _win32_audio.is_started = true;
        }

        return(true);
    }
};
