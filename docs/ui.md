# In-Game UI

The in-game UI (`IFB.Game/src/game-ui.cpp`, declared in `ifb-game.hpp`) is a set of controls drawn with [immediate drawing](renderer.md#immediate-drawing) on the screen layer: buttons, checkboxes, sliders, and drop downs. It's for UI that's part of the game, like menus. The ImGui windows are the developer tools, and are separate.

The controls are **immediate mode**: nothing is created or kept. Each pass, the game calls a function for every control it wants on screen, and the function draws it, reads the mouse and keys, and returns whether the player did something to it.

```cpp
game_ui_begin(ctx->fnt, !ctx->is_paused);

const rect row = { 40.0f, 40.0f, 300.0f, 32.0f };
if (game_ui_button(row, "Start")) {
    // pressed this pass
}

game_ui_end();
```

## Controls

Each control takes a `rect` in screen pixels (`x` and `y` are the top left) and a label.

| Function | Draws | Returns `true` |
| --- | --- | --- |
| `game_ui_button(dst, label)` | A box with the label centered in it | The pass it's clicked |
| `game_ui_checkbox(dst, label, value)` | A square box on the left of `dst`, ticked when `value` is set, with the label after it. Clicking anywhere in `dst` flips it. | The pass `value` flips |
| `game_ui_slider(dst, label, value, min, max)` | A bar filled up to a knob, with the label and value written over it. `value` is kept between `min` and `max`. | Each pass `value` changes |
| `game_ui_dropdown(dst, label, items, item_count, selected)` | A box with `label: item`, and a list of the items under it while it's open | The pass `selected` changes |
| `game_ui_label(dst, text)` | Text, vertically centered in `dst`. It isn't a control, so the focus skips it. | Nothing |

`game_ui_begin(fnt, is_input_enabled)` reads the mouse and keys once for the pass. With `is_input_enabled` false, the controls are drawn but nothing can use them, like while the game is paused or a scene is fading. `game_ui_end()` draws the open drop down's list and lets go of the mouse once its button is up. Call every control between the two.

### Labels Are IDs

A control is found again from one pass to the next by a hash of its label, so it knows if it's the one being held or the open drop down. The labels in a pass have to be unique. Two buttons both labeled `OK` would act as one.

## Mouse

The mouse comes from the [engine](input.md#mouse): `eng_input_mouse_get_screen_pos` already puts it in screen-layer units, so it lines up with the rects even when [`eng_draw_set_screen_size`](renderer.md#screen-size) stretches the screen. While the mouse is over an ImGui window, the controls don't see it.

- **Buttons and checkboxes** act when the mouse is let go over them, after being pressed on them. Pressing one and dragging off of it before letting go does nothing.
- **Sliders** move the knob to the mouse while they're held, even past the ends, so it's easy to pin them to `min` or `max`.
- **Drop downs** open when the box is pressed. Letting go over an item picks it, so pressing the box and dragging down to an item works too. Pressing the box again, or anywhere outside the box and list, closes it without picking.

## Keyboard

| Key | What it does |
| --- | --- |
| Up / Down | Move the focus to the last or next control, in the order they're called. It wraps from the last to the first. |
| Left / Right | Move the focused slider a twentieth of its range (`GAME_UI_SLIDER_KEY_STEPS`), or pick the last or next item of the focused drop down |
| Enter / Space | Press the focused button, flip the focused checkbox, or open the focused drop down |

While a drop down is open, Up and Down move its highlight and Enter or Space picks the highlighted item.

The focused control has an orange outline. It shows once a key is used, and hides on a click. Clicking a control also gives it the focus.

Keys act on the press, not while they're held, and they come from `eng_input_is_key_down`, so they don't reach the controls while an ImGui window has the keyboard. The arrows are used instead of W, A, S, and D so the UI doesn't fight the player rig.

## Drop Down Lists

An open list goes over the controls after it, so it's drawn last, by `game_ui_end`. The items array has to stay around until then, which a `static const` array does.

Only one drop down is open at a time. While it is:

- Controls under the list don't get the mouse. Only the list does.
- The keys go to the list, so the focus doesn't move.
- If the drop down stops being called, it closes.

A list shows up to `GAME_UI_DROPDOWN_ITEMS_MAX` (16) items.

## Look

The sizes and colors are the `GAME_UI_*` constants in `ifb-game.hpp`. The defaults match the dialog box (`game-dialog.cpp`): a dark border around a light fill, with dark text in the game's font. A control is lighter blue under the mouse, darker blue while it's held, and outlined in orange while it has the focus.

### Text Color

`game_ui_set_text_color(rgba)` changes the text color (`0xRRGGBBAA`) for the controls called after it, until it's set again. `game_ui_begin` puts it back to `GAME_UI_COLOR_TEXT` every pass, so a color can't stay set by accident.

```cpp
game_ui_set_text_color(0xFFFFFFFF);         // white, for a label on a dark background
game_ui_label(title_row, "Paused");
game_ui_set_text_color(GAME_UI_COLOR_TEXT); // back to the default for the buttons
```

Only the text changes. The border and fill stay the same, so keep text in a control readable on the light fill. An open drop down's list is drawn later by `game_ui_end`, but it keeps the text color its drop down was called with.

## The UI Test Scene

**Game > Testing > UI** in the GUI opens the UI test scene (`IFB.Game/src/game-scene-ui-test.cpp`, `game_scene_id_e_ui_test`). It has one of every control, wired to a preview so you can see them work:

| Control | Changes |
| --- | --- |
| **Press Me** | Counts presses |
| **Reset** | Puts everything back |
| **Show Preview**, **Outline** | Show the swatch, and its white outline |
| **Red**, **Green**, **Blue** | The swatch's color |
| **Size** | The swatch's size |
| **Shape** | Its proportions. The list opens over **Backdrop**, which tests that a list blocks the controls under it. |
| **Backdrop** | The color behind the panel |

The title and the hint at the bottom use `game_ui_set_text_color`. The last line reports the last thing a control did. The scene has no map or entities, only the screen layer. Its controls are drawn but can't be used while the game is paused or the scene is fading.
