# Input

The input system (`IFB.Engine/src/engine/eng-input.cpp`, `eng-keycode.cpp`, `eng-input.hpp`) tracks which keys are held down. Keycodes are defined in `IFB.Core/include/ifb-input.hpp`.

## Checking Keys

```cpp
if (eng_input_is_key_down(input_keycode_e_a)) {
    // move left
}
```

`eng_input_is_key_down` tells you if a key is down **right now**. There are no "pressed this frame" or "released this frame" events, so to react once per press, keep the previous state yourself and compare.

The player rig moves with W, A, S, and D (`game_player_rig_update_and_render` in `game-player-rig.cpp`).

### ImGui Focus

`eng_input_is_key_down` returns `false` for every key while the GUI wants the keyboard: when any ImGui window is focused, or ImGui is capturing keyboard input (for example, while you're typing in a text field). This stops typing in the GUI from also moving the player.

Key state is still tracked while the GUI has focus, so a key that's released during that time isn't stuck down afterwards.

## How Keys Are Tracked

1. The Win32 window gets `WM_KEYDOWN` / `WM_SYSKEYDOWN` and `WM_KEYUP` / `WM_SYSKEYUP` messages while it processes events at the start of each frame (`win32-window.cpp`).
2. `win32_input_get_keycode` (`win32-input.cpp`) turns the virtual key code into an `input_keycode`. Windows reports Shift, Ctrl, and Alt as single keys, so the left and right versions are told apart using the scan code and the extended key flag.
3. The engine calls `eng_input_set_key_down` or `eng_input_set_key_up`, which sets or clears that key's bit.

Keys Windows sends that aren't in the keycode table map to `input_keycode_e_null` and are ignored.

Messages also go to ImGui's Win32 handler, so the GUI gets keyboard and mouse input on its own.

## Keycodes

`input_keycode` is a `u16`. Each keycode is the position of a bit in the keyboard state:

- **High byte:** the group, `0` to `3`
- **Low byte:** the bit within the group, `0` to `31`

The keyboard state is four 32-bit masks, which holds 128 keys. That's why the values in `input_keycode_e` skip from `0x001F` to `0x0100`: each group only uses the first 32 values.

| Group | Range | Keys |
| --- | --- | --- |
| 0 | `0x0000` – `0x001F` | Escape, Tab, Enter, Backspace, Space, Shift, Ctrl, Alt, Caps/Num/Scroll Lock, Super, Menu, F1–F12, Insert, Delete, Home |
| 1 | `0x0100` – `0x011F` | End, Page Up/Down, arrows, 0–9, minus, equals, A–M |
| 2 | `0x0200` – `0x021F` | N–Z, punctuation, keypad divide/multiply/subtract/add/enter/decimal, keypad 0–3 |
| 3 | `0x0300` – `0x030C` | Keypad 4–9, volume, media keys |

Use the `input_keycode_e_*` names instead of the numbers. When you add a key, keep its low byte under 32 so it maps to a valid bit.

## Mouse and Gamepad

The mouse functions in the public API (`eng_input_mouse_click_left_down`, `_up`, `eng_input_mouse_click_right_down`, `_up`, `eng_input_mouse_move`) exist, but their bodies are **empty**: the engine doesn't track the mouse yet. `WM_MOUSEMOVE` is passed to `eng_input_mouse_move` and ignored.

`eng-input.hpp` also defines mouse flags, gamepad button flags, and an `input_mouse` struct, but nothing uses them yet.

## Public API

| Function | Description |
| --- | --- |
| `eng_input_is_key_down` | Check if a key is held down. Always `false` while the GUI has keyboard focus. |
| `eng_input_set_key_down` / `_set_key_up` | Set a key's state. The platform layer calls these, but they can also be used to simulate input. |
| `eng_input_mouse_*` | Not implemented yet |
