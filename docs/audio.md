# Audio

The audio system sends samples to the default output device. The device is opened through the platform layer (`pfm_audio_*`, WASAPI on Windows), and the engine keeps one buffer that the game and GUI write into each frame.

| File | Contents |
| --- | --- |
| `IFB.Core/include/ifb.hpp` | `audio_buffer` and `audio_info` |
| `IFB.Engine/include/eng-audio.hpp`, `src/audio/audio-mngr.cpp` | The audio manager |
| `IFB.Engine/src/audio/audio-source.cpp` | Where a voice gets its samples from |
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
| `audio_mngr_frame_end` | Before the swap | Mixes the [voices](#voices) into the buffer, scales it by the [master volume](#master-volume), clamps it, and sends it to the device |

The buffer always goes to the device, so if nothing writes to it, silence plays. The frame count changes every frame, so anything generating sound has to keep its own position (like the phase of a wave) between frames.

The game callback can run several times in one frame, and every call gets the same buffer. Write to it once per frame.

Most sound should go through voices instead of the buffer. Writing to the buffer is for generated sound, like the audio test.

## Voices

A voice plays a `wav` from memory. The audio manager has a fixed pool of them (`IFB_CONFIG_AUDIO_VOICE_CAPACITY`, 32) and mixes every active one at the end of the frame, after the game and GUI callbacks, so they're added to anything written to the buffer directly.

```cpp
hnd_audio_voice h_voice = eng_audio_voice_play(w, audio_group_e_sfx, 0.5f, false);

eng_audio_voice_set_paused (h_voice, true);
eng_audio_voice_seek       (h_voice, 0.0);
eng_audio_voice_stop       (h_voice);
```

| Function | Description |
| --- | --- |
| `eng_audio_voice_play` | Start a voice from the beginning of a wav, in a [group](#volume-groups). Returns an invalid handle if there's no device or every voice is in use. |
| `eng_audio_sound_play` | Start a voice on a [sound asset](assets.md#sounds) by name. Also returns an invalid handle if there's no sound by that name, or no room to load it. |
| `eng_audio_sound_preload` / `_unload` | Load a sound ahead of time and keep it in memory, see [Preloading](assets.md#preloading) |
| `eng_audio_sound_stream` | Start a voice on a sound asset by name, read from the bank a chunk at a time instead of loading it, see [Streaming](#streaming) |
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

### Sources

A voice reads its samples through an `audio_source` (`audio-source.cpp`), so the mixer doesn't care where they come from. A memory source reads a wav that's already in memory, and a stream source reads a sound from the bank a chunk at a time.

| Function | Description |
| --- | --- |
| `audio_source_from_memory` | A source for a wav in memory. With a sound asset's handle, the source holds its pin. |
| `audio_source_from_stream` | A source for an open stream, which the source gives back when it's released |
| `audio_source_read` | Read a range of frames as floats, at most 2 channels (mono stays mono, extra channels are dropped). The frames have to be inside the source. Returns the frames it read, and a stream can come up short. |
| `audio_source_release` | Let go of the source, and unpin its sound asset if it has one |

The mixer works on a voice a block of up to 256 device frames at a time. It reads the source frames that block needs into a scratch buffer first, handling the loop and the end itself: a looping voice wraps around, and one that isn't holds its last frame. Then it blends from the scratch buffer. A source at a high sample rate needs more of its frames for each block, so its blocks are shorter, and anything the source comes up short on is played as silence.

Sound assets are always 16 bit, so a memory source reads them directly. Other wavs, like the ones the WAV Player opens, go through `wav_read_sample`.

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

## Streaming

`eng_audio_sound_stream` plays a sound asset without loading it. It's read from the [sound bank](assets.md#sound-bank) a chunk at a time while it plays, so a long sound like music doesn't take its whole size in memory, or hitch the frame it starts in. It returns an ordinary voice, so pausing, seeking, looping, volume, groups, and stopping all work the same.

```cpp
hnd_audio_voice h_music = eng_audio_sound_stream("PIANO", audio_group_e_music, 0.5f, true);
```

Whether a sound streams is up to the call, not the sound, so the same sound can be streamed in one place and loaded in another. Stream long sounds, and load the short ones that play often.

| Setting | Value | |
| --- | --- | --- |
| `IFB_CONFIG_AUDIO_STREAM_CAPACITY` | 4 | Streams that can play at once. Another one returns an invalid handle. |
| `IFB_CONFIG_AUDIO_STREAM_CHUNK_FRAMES` | 32768 | Frames in a chunk, about 0.7 seconds |
| `IFB_CONFIG_AUDIO_STREAM_CHUNK_COUNT` | 3 | Chunks each stream keeps |

Each stream has a ring of chunks, set aside when the engine starts (1.5 MB for all of them, in the audio reservation), so starting one never allocates. Before a streaming voice is mixed, its ring is topped up to hold the chunk the position is in and the 2 after it, reading the ones it's missing from the bank. A looping stream's chunks after the last one are the first ones again, so the loop doesn't wait on the disk. A seek reads the chunks it lands on before the next mix. Starting a stream reads its first chunks right away, so it starts on time.

Things to know:

- **The reads are on the main thread for now.** A chunk is read when the playhead reaches the chunk before it, which is about 128 KB of stereo every 0.7 seconds, and seeking reads up to 3 at once. They're small reads from the bank's own file, but a slow disk can still hitch a frame. Reading on a worker thread is next.
- **A chunk that can't be read plays as silence**, and the voice keeps going. It's read again the next frame.
- **Nothing is pinned or loaded.** Preloading doesn't apply, and a stream doesn't count against asset memory.
- **Copy to Runtime stops every stream**, the same as the loaded sounds, since the bank they read from is replaced.

The **Audio Test** window shows the streams in use and the chunks read since the stats were reset, and the database window has **Stream From Runtime** next to **Play From Runtime**.

## Volume Groups

Every voice is in a group, given when it's played, and plays at its own volume times its group's. A settings menu can turn the music down without touching the sound effects, or the game can duck the music under a cutscene.

| Group | For |
| --- | --- |
| `audio_group_e_sfx` | Sound effects, and the engine's tools (the WAV Player and the asset previews) |
| `audio_group_e_music` | Music |

```cpp
eng_audio_group_set_volume (audio_group_e_music, 0.5f);
f32 volume = eng_audio_group_get_volume(audio_group_e_music);
```

| Function | Description |
| --- | --- |
| `eng_audio_group_set_volume` | Set a group's volume, from `0` to `1`, values outside that are clamped |
| `eng_audio_group_get_volume` | A group's volume |

Things to know:

- **Every group starts at `1`**, and the volumes aren't saved. A settings menu has to save and restore them itself.
- **A change fades over 20 ms** on the voices that are playing, the same as a voice's own volume, so it doesn't click.
- **A voice in a group at `0` still plays.** It's silent, but its position moves and it frees itself at the end, so turning the group back up brings it in where it would be.
- **The voice's own volume doesn't change.** `eng_audio_voice_get_info` still returns the volume it was given, and its group.

The **Audio Test** window has a slider for each group.

## Master Volume

The master volume scales everything that goes to the device, after the voices are mixed. Since it's applied to the whole buffer, it also covers sound written to the buffer directly, like the audio test's wave, which the groups don't.

```cpp
eng_audio_set_master_volume(0.8f);
f32 volume = eng_audio_get_master_volume();
```

A voice plays at its own volume, times its group's, times the master. The master runs from `0` to `1`, values outside that are clamped, it starts at `1`, and it isn't saved. A change fades over 20 ms like the others. It's applied before the mix is clamped, so turning it down brings a loud sum of voices back under `1` instead of clipping.

The **Audio Test** window has a slider for it above the groups.

## WAV Player

**Engine > WAV Player** opens a `.wav` file and plays it on a voice, with a waveform, a position slider, volume, and looping. Closing the window pauses it.

Files have to be uncompressed PCM at 8, 16, 24, or 32 bit. Files that use the extensible format header, 32 bit float, or compression won't open. The samples have to fit in one large arena (64 MB). Loading takes a second large arena for as long as it takes to read the file.

Mono files play on both channels, and past the first 2 channels of a file are ignored.

The file is loaded on the main thread, and the file dialog blocks it too, so the sound pauses and can underrun while choosing a file.
