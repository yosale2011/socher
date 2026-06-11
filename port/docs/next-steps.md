# Next implementation steps

The asset pipeline now has three pieces:

- `decode_pic.py` decodes `.SCR`, `.WIN`, `.SGN`, and `.LIN` buffers.
- `asset_viewer.py` opens decoded assets in a small Tk window.
- `render_scene.py` implements the original `PutPic(Buffer, X, BottomY)`
  coordinate convention on a 320x200 framebuffer.
- `text_layer.py` prototypes a 40x25 text grid using the bitmap font from
  `socher1/FONTHE8.COM` at offset `604`, and draws characters one by one so
  visual-order Hebrew strings are not reordered by the host OS.

Next work should move this logic into the real port runtime:

1. Create an FPC `Picture` unit with the same header parser and packed-CGA row
   decoder. Done in `src/Picture.pas`.
2. Replace `PortPutPic` and `PortGetPic` in `src/Platform.pas` with framebuffer
   implementations. Done for the current smoke runtime.
3. Move the 8x8 bitmap font and text grid into the Pascal runtime. Done in
   `src/TextGrid.pas`.
4. Add a real window backend (SDL2 or another FPC-friendly graphics layer) that
   presents the 320x200 framebuffer.
5. Route original `GotoXY`, `WhereX`, `WhereY`, `TextColor`, and graphics-mode
   `Write` calls through `Platform`.
6. Wire keyboard input and replace the temporary blocking console read.
7. Only after graphics and text are stable, wire the full gameplay flow.

This order keeps the port honest: each gameplay screen can be compared visually
against the original assets before game logic changes.
