# Audio

The audio system sends samples to the default output device. The device is opened through the platform layer (`pfm_audio_*`, WASAPI on Windows), and the engine keeps one buffer that the game and GUI write into each frame.

| File | Contents |
| --- | --- |
| `IFB.Core/include/ifb.hpp` | `audio_buffer` and `audio_info` |
| `IFB.Engine/include/eng-audio.hpp`, `src/audio/audio-mngr.cpp` | The audio manager |
| `IFB.Engine/src/engine/eng-audio.cpp` | The public `eng_audio_*` functions |
| `IFB.Engine/src/win32/win32-audio.cpp` | The WASAPI implementation |
| `IFB.GUI/src/gui-engine-audio-test.cpp` | The audio test window |
| `IFB.Engine/include/eng-wav.hpp`, `src/wav/wav.cpp` | Parsing WAV files and decoding their samples |
| `IFB.Engine/src/engine/eng-wav.cpp` | The public `eng_wav_*` functions |
| `IFB.GUI/src/gui-engine-wav-player.cpp` | The WAV player window |

## Format

Samples are interleaved 32 bit floats from `-1` to `1`, at 48 kHz in stereo (`IFB_CONFIG_AUDIO_*` in `ifb-config.hpp`). The device runs in shared mode, and Windows converts the samples to its mix format.

## Each Frame

Everything runs on the main thread, in the [frame loop](architecture.md#frame-loop).

| Call | When | What it does |
| --- | --- | --- |
| `audio_mngr_frame_start` | Start of the frame | Reads how many frames are queued on the device, updates the stats, and clears the buffer to silence |
| `eng_audio_get_buffer` | Game or GUI callback | Gets the buffer, with enough frames to fill the queue back up to the target latency |
| `audio_mngr_frame_end` | Before the swap | Mixes the [voices](#voices) into the buffer, clamps it, and sends it to the device |

The buffer always goes to the device, so if nothing writes to it, silence plays. The frame count changes every frame, so anything generating sound has to keep its own position (like the phase of a wave) between frames.

The game callback can run several times in one frame, and every call gets the same buffer. Write to it once per frame.

Most sound should go through voices instead of the buffer. Writing to the buffer is for generated sound, like the audio test.

## Voices

A voice plays a `wav` from memory. The audio manager has a fixed pool of them (`IFB_CONFIG_AUDIO_VOICE_CAPACITY`, 32) and mixes every active one at the end of the frame, after the game and GUI callbacks, so they're added to anything written to the buffer directly.

```cpp
hnd_audio_voice h_voice = eng_audio_voice_play(w, 0.5f, false);

eng_audio_voice_set_paused (h_voice, true);
eng_audio_voice_seek       (h_voice, 0.0);
eng_audio_voice_stop       (h_voice);
```

| Function | Description |
| --- | --- |
| `eng_audio_voice_play` | Start a voice from the beginning of a wav. Returns an invalid handle if there's no device or every voice is in use. |
| `eng_audio_sound_play` | Start a voice on a [sound asset](assets.md#sounds) by name. Also returns an invalid handle if there's no sound by that name, or no room to load it. |
| `eng_audio_voice_stop` | Fade out, then free the voice |
| `eng_audio_voice_stop_now` | Free the voice without a fade |
| `eng_audio_voice_stop_all_using` | Free every voice playing a wav, without a fade |
| `eng_audio_voice_set_paused` | Fade out and hold the position, or fade back in |
| `eng_audio_voice_set_volume` | Change the volume, which fades to the new value |
| `eng_audio_voice_set_looping` | Loop back to the start at the end, instead of freeing the voice |
| `eng_audio_voice_seek` | Jump to a position, in frames of the wav |
| `eng_audio_voice_get_info` | The position, volume, and state. Returns `false` if the voice was freed. |

Things to know:

- **A voice frees itself at the end of a wav** unless it's looping, so a sound can be played and forgotten. Its handle goes stale, and every call with a stale handle does nothing.
- **The wav has to stay in memory until every voice playing it is freed.** A stopped voice keeps reading it while it fades out, so call `eng_audio_voice_stop_all_using` before freeing it. Sound assets take care of this themselves, a voice keeps its sound pinned until it's freed.
- **Every change fades over 20 ms** (`IFB_CONFIG_AUDIO_FADE_MS`): starting, stopping, pausing, resuming, and volume. Seeking doesn't fade, so it can click once.
- **The mix clips.** Voices are added together and clamped to `-1` to `1` once at the end, so many loud voices at once will distort.
- **Voices are main thread only**, like the rest of audio.
- **Without a device, voices don't advance.** If the device goes away, the voices that were playing stay where they were.

Each voice resamples on its own, so a wav at any sample rate plays at the right speed. It blends the 2 closest frames, which is fine for a preview but loses some of the high end.

## Latency

The target is 50 ms of queued audio (`IFB_CONFIG_AUDIO_LATENCY_MS`) in a 100 ms device buffer. A frame can take up to the target latency before the device runs out of samples, and raising it trades slower response for more room.

The main thread fills the buffer, so anything that blocks the main thread for longer than the target causes an underrun. Dragging or resizing the window blocks it for as long as the drag lasts, because Windows runs its own message loop until the drag ends.

## Device Failure

If there's no device at startup, or the device goes away while running (like unplugging headphones), the system stops and `eng_audio_get_buffer` returns `false`. The game keeps running, silent. It doesn't reopen a device yet.

## Audio Test

**Engine > Audio Test** writes a square, sawtooth, or sine wave, or white noise, to the buffer, to check that it plays without skipping between frames. A clean run sounds steady, the scope shows an unbroken wave, and the underrun count stays at `0`. Noise ignores the frequency slider, and it hides skips, so use a sine wave to listen for them.

| Stat | Meaning |
| --- | --- |
| Queued | The audio left on the device at the start of this frame |
| Queued Min | The lowest that's been since the last reset. Close to `0` means underruns are close. |
| Frame Gap | The time since the last frame started |
| Frame Gap Max | The longest gap since the last reset. Longer than the target latency means the device ran out. |
| Underruns | The number of times the device ran out of samples |
| Voices | The voices in use, out of the pool |

The wave fades in and out over 20 ms when it starts and stops, so those aren't heard as clicks. Switching waves or changing the frequency keeps the phase, but a switch between waves can still click once because the value jumps. That isn't a skip.

## WAV Player

**Engine > WAV Player** opens a `.wav` file and plays it on a voice, with a waveform, a position slider, volume, and looping. Closing the window pauses it.

Files have to be uncompressed PCM at 8, 16, 24, or 32 bit. Files that use the extensible format header, 32 bit float, or compression won't open. The samples have to fit in one large arena (64 MB). Loading takes a second large arena for as long as it takes to read the file.

Mono files play on both channels, and past the first 2 channels of a file are ignored.

The file is loaded on the main thread, and the file dialog blocks it too, so the sound pauses and can underrun while choosing a file.
