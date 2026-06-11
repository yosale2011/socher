#!/usr/bin/env python3
"""Small Tk viewer for decoded Socher Hayam picture buffers."""

from __future__ import annotations

import argparse
import sys
import tkinter as tk
from pathlib import Path

from decode_pic import CGA_PALETTE, decode_pic


DEFAULT_ASSETS = [
    "MAINSCRN.SCR",
    "MAP.WIN",
    "BUY.SGN",
    "SELL.SGN",
    "MESSAGE.SGN",
    "LINE_6.LIN",
    "INTRO.SCR",
    "HELP.SCR",
]


class AssetViewer:
    def __init__(self, root: tk.Tk, assets: list[Path], scale: int) -> None:
        self.root = root
        self.assets = assets
        self.scale = scale
        self.index = 0
        self.canvas = tk.Canvas(root, bg="black", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)
        self.photo: tk.PhotoImage | None = None

        root.bind("<Right>", lambda _event: self.next_asset())
        root.bind("<Down>", lambda _event: self.next_asset())
        root.bind("<Left>", lambda _event: self.previous_asset())
        root.bind("<Up>", lambda _event: self.previous_asset())
        root.bind("<Escape>", lambda _event: root.destroy())

        self.show_asset()

    def previous_asset(self) -> None:
        self.index = (self.index - 1) % len(self.assets)
        self.show_asset()

    def next_asset(self) -> None:
        self.index = (self.index + 1) % len(self.assets)
        self.show_asset()

    def show_asset(self) -> None:
        path = self.assets[self.index]
        width, height, pixels = decode_pic(path.read_bytes())
        image = tk.PhotoImage(width=width, height=height)

        for y in range(height):
            row = []
            for x in range(width):
                r, g, b = CGA_PALETTE[pixels[y * width + x]]
                row.append(f"#{r:02x}{g:02x}{b:02x}")
            image.put("{" + " ".join(row) + "}", to=(0, y))

        if self.scale > 1:
            image = image.zoom(self.scale, self.scale)

        self.photo = image
        self.canvas.delete("all")
        self.canvas.config(width=image.width(), height=image.height())
        self.canvas.create_image(0, 0, anchor=tk.NW, image=image)
        self.root.title(
            f"Socher asset viewer - {path.name} ({width}x{height}) "
            f"[{self.index + 1}/{len(self.assets)}]"
        )


def default_asset_paths(asset_dir: Path) -> list[Path]:
    return [asset_dir / name for name in DEFAULT_ASSETS if (asset_dir / name).exists()]


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("assets", nargs="*", type=Path)
    parser.add_argument("--asset-dir", type=Path, default=Path("socher1"))
    parser.add_argument("--scale", type=int, default=3)
    args = parser.parse_args(argv)

    assets = args.assets or default_asset_paths(args.asset_dir)
    if not assets:
        parser.error("no assets supplied and no default assets found")

    root = tk.Tk()
    AssetViewer(root, assets, args.scale)
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
