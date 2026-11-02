# Audio

The audio system sends samples to the default output device. The device is opened through the platform layer (`pfm_audio_*`, WASAPI on Windows), and the engine keeps one buffer that the game and GUI write into each frame.

| File | Contents |
| --- | --- |
| `IFB.Core/include/ifb.hpp` | `audio_buffer` and `audio_info` |
| `IFB.Engine/include/eng-audio.hpp`, `src/audio/audio-mngr.cpp` | The audio manager |
| `IFB.Engine/src/engine/eng-audio.cpp` | The public `eng_audio_*` functions |
| `IFB.Engine/src/win32/win32-audio.cpp` | The WASAPI implementation |
| `IFB.GUI/src/gui-engine-audio-test.cpp` | The audio test window |

## Format

Samples are interleaved 32 bit floats from `-1` to `1`, at 48 kHz in stereo (`IFB_CONFIG_AUDIO_*` in `ifb-config.hpp`). The device runs in shared mode, and Windows converts the samples to its mix format.

## Each Frame

Everything runs on the main thread, in the [frame loop](architecture.md#frame-loop).

| Call | When | What it does |
| --- | --- | --- |
| `audio_mngr_frame_start` | Start of the frame | Reads how many frames are queued on the device, updates the stats, and clears the buffer to silence |
| `eng_audio_get_buffer` | Game or GUI callback | Gets the buffer, with enough frames to fill the queue back up to the target latency |
| `audio_mngr_frame_end` | Before the swap | Sends the buffer to the device |

The buffer always goes to the device, so if nothing writes to it, silence plays. The frame count changes every frame, so anything generating sound has to keep its own position (like the phase of a wave) between frames.

The game callback can run several times in one frame, and every call gets the same buffer. Write to it once per frame.

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

The wave fades in and out over 20 ms when it starts and stops, so those aren't heard as clicks. Switching waves or changing the frequency keeps the phase, but a switch between waves can still click once because the value jumps. That isn't a skip.
