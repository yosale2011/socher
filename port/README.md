# Socher Hayam 32-bit port

This directory is the modern port workspace. It is intentionally separate from
the matching Turbo Pascal decompilation in the repository root.

Current goals:

- Keep the original decompilation as the behavioral baseline.
- Move platform-dependent work into a small replacement layer.
- Decode the original TP3/CGA picture buffers before changing gameplay code.

## First milestone

Run the picture decoder against an asset:

```powershell
python port\tools\decode_pic.py socher1\MAINSCRN.SCR port\mainscrn.png
```

The decoder supports `.SCR`, `.WIN`, `.SGN`, and `.LIN` TP picture buffers.
TP picture data is stored bottom-up, so the decoder flips rows vertically by
default.

To open a small visual smoke viewer:

```powershell
python port\tools\asset_viewer.py
```

Use the arrow keys to cycle assets and `Esc` to close.

To run the current interactive runtime prototype:

```powershell
python port\tools\runtime_demo.py
```

Use the arrow keys to move the highlighted menu option and `Esc` to close.
Press `Enter` to open the selected original game window; inside a window, arrow
keys move between sub-options where available, `Enter` starts a quantity prompt
for buy/sell/bank actions, and `Esc` steps back.
In the quantity prompt, type digits and press `Enter` to confirm.
The demo also supports quick keys: `b` buy, `s` sell, `t` travel, `k` bank,
and `+`/`-` to change the transaction amount.

To run all current prototype checks:

```powershell
python port\tools\check_port.py
```

To render scenes using original `PutPic(Buffer, X, BottomY)` coordinates:

```powershell
python port\tools\render_scene.py main-map port\scene-main-map.png
python port\tools\render_scene.py message port\scene-message.png
python port\tools\render_scene.py hud-mock port\scene-hud-mock.png
```

To inspect the extracted DOS bitmap font:

```powershell
python port\tools\extract_font.py socher1\FONTHE8.COM port\fonthe8-sheet.png --height 8 --offset 604
```

When Free Pascal is installed, compile the first Pascal smoke test with:

```powershell
fpc port\src\smoke_picture.lpr
fpc port\src\smoke_platform.lpr
```

`smoke_platform` writes `port\smoke-platform.ppm` through the same platform
functions that will replace the original TP calls.

## Known 32-bit port risks

- Turbo Pascal `Integer` is 16-bit; Free Pascal defaults vary by mode/target.
- Turbo Pascal `Real` and short string record layout affect `WINNERS.WIN`.
- `Random` needs a TP-compatible wrapper for exact gameplay parity.
- Hebrew strings are stored in visual reverse order and should be rendered as-is.
- `Read(KBD, ch)` and special key codes need a DOS-compatible input adapter.
