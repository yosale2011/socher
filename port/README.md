# Socher Hayam 32-bit port

This directory is the modern port workspace. It is intentionally separate from
the matching Turbo Pascal decompilation in the repository root.

## Playing the 32-bit port

`port\bin\socher.exe` is a native 32-bit Windows executable (Win32 GDI window,
320x200 framebuffer scaled x3). No DOSBox, no runtime dependencies.

Build (from the repository root):

```powershell
python port\tools\transpile.py
& "C:\FPC\3.2.2\bin\i386-win32\fpc.exe" -B -Fuport\src -FEport\bin -FUport\bin port\src\socher.lpr
```

`transpile.py` regenerates `port\gen\globals.inc` / `code.inc` from the
original `GLOBALS.PAS` / `CODE.PAS` (mechanical renames + documented port
patches; never edit the `.inc` files by hand).

Run:

```powershell
port\run_port.bat
```

(The launcher just starts the exe with the working directory set to `socher1`
so the original `.scr`/`.win`/`.sgn`/`.lin` assets and `winners.win` are
found.)

Controls are the original ones: arrow keys / space move the menu highlight,
Enter selects, Esc backs out, F10 opens the help screen. Yes/No prompts accept
the Hebrew kaf/lamed keys or their Latin positions `f`/`k`.

Environment variables (all optional):

| Variable          | Effect |
|-------------------|--------|
| `SOCHER_SCALE`    | Window scale factor 1..10 (default 3). |
| `SOCHER_SEED`     | Decimal RNG seed for deterministic runs. |
| `SOCHER_KEYS`     | Path to a key-script file (one token per line: `ENTER`, `ESC`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `F10`, `SPACE`, `BACKSPACE`, `WAIT` = dump a frame, or a single literal character). Enables script mode: keys are fed from the file and the program exits 0 when the script ends (exit 2 if the file does not exist). |
| `SOCHER_DUMP_DIR` | Directory that receives `frame-NNNN.ppm` framebuffer dumps on every key read (and on `WAIT`). |
| `SOCHER_FONT`     | Explicit path to `FONTHE8.COM` if not running from the repo tree. |

Scripted regression runs: `port\tests\round1\run_round1.ps1` replays six
scenarios with a fixed seed and converts the dumped frames to PNG.

Known gaps:

- Sound is approximated with the Windows `Beep` API: tones only play during
  `Delay` calls and the square-wave timbre differs from a real PC speaker.
- `Random` is Free Pascal's generator, not TP3's LCG, so gameplay event
  sequences do not match the DOS original run-for-run (use `SOCHER_SEED` for
  reproducibility within the port).
- Typing Hebrew directly requires a Hebrew (CP1255) ANSI system codepage; on
  other systems use the Latin key positions, which the game itself maps to
  Hebrew letters.
- Window size is fixed at startup (`SOCHER_SCALE`); no fullscreen or resize.

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
