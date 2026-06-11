#!/usr/bin/env python3
"""Render small Socher Hayam scenes using original PutPic coordinates."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from PIL import Image, ImageDraw

from decode_pic import CGA_PALETTE, decode_pic, write_png
from text_layer import TextLayer, right_aligned


class FrameBuffer:
    def __init__(self, width: int = 320, height: int = 200, color: int = 0) -> None:
        self.width = width
        self.height = height
        self.pixels = [color] * (width * height)

    def put_pic(self, data: bytes, x: int, bottom_y: int) -> None:
        pic_width, pic_height, pic_pixels = decode_pic(data)
        top_y = bottom_y - pic_height + 1

        for src_y in range(pic_height):
            dst_y = top_y + src_y
            if dst_y < 0 or dst_y >= self.height:
                continue
            for src_x in range(pic_width):
                dst_x = x + src_x
                if dst_x < 0 or dst_x >= self.width:
                    continue
                self.pixels[dst_y * self.width + dst_x] = pic_pixels[
                    src_y * pic_width + src_x
                ]

    def save_png(self, path: Path) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        write_png(path, self.width, self.height, self.pixels)

    def to_image(self) -> Image.Image:
        image = Image.new("RGB", (self.width, self.height))
        image.putdata([CGA_PALETTE[pixel] for pixel in self.pixels])
        return image


def read_asset(asset_dir: Path, name: str) -> bytes:
    return (asset_dir / name).read_bytes()


def render_main_map(asset_dir: Path, output: Path) -> None:
    fb = FrameBuffer(color=1)
    fb.put_pic(read_asset(asset_dir, "MAINSCRN.SCR"), 0, 199)
    fb.put_pic(read_asset(asset_dir, "MAP.WIN"), 0, 142)
    fb.save_png(output)


def render_message(asset_dir: Path, output: Path) -> None:
    fb = FrameBuffer(color=1)
    fb.put_pic(read_asset(asset_dir, "MAINSCRN.SCR"), 0, 199)
    fb.put_pic(read_asset(asset_dir, "MESSAGE.SGN"), 10, 138)
    fb.save_png(output)


def render_hud_mock(asset_dir: Path, output: Path) -> None:
    fb = FrameBuffer(color=1)
    fb.put_pic(read_asset(asset_dir, "MAINSCRN.SCR"), 0, 199)
    image = fb.to_image()
    text = TextLayer(ImageDraw.Draw(image))
    text.text_color(3)

    text.write_at(73, 1, right_aligned("לארשי", 6))
    text.write_at(32, 3, "ןושאר")
    text.write_at(32, 5, right_aligned("8", 2))
    text.write_at(34, 5, ":00")
    text.write_at(33, 17, right_aligned("3,000", 5))

    text.write_at(32, 9, right_aligned("0", 3))
    text.write_at(32, 11, right_aligned("0", 3))
    text.write_at(32, 13, right_aligned("0", 3))

    text.write_at(30, 21, right_aligned("3000", 4))
    text.write_at(30, 23, right_aligned("500", 4))
    text.write_at(30, 25, right_aligned("50", 4))
    text.write_at(16, 21, right_aligned("3000", 4))
    text.write_at(16, 23, right_aligned("500", 4))
    text.write_at(16, 25, right_aligned("50", 4))
    text.write_at(2, 21, right_aligned("3000", 4))
    text.write_at(2, 23, right_aligned("500", 4))
    text.write_at(2, 25, right_aligned("50", 4))

    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("scene", choices=["main-map", "message", "hud-mock"])
    parser.add_argument("output", type=Path)
    parser.add_argument("--asset-dir", type=Path, default=Path("socher1"))
    args = parser.parse_args(argv)

    if args.scene == "main-map":
        render_main_map(args.asset_dir, args.output)
    elif args.scene == "message":
        render_message(args.asset_dir, args.output)
    elif args.scene == "hud-mock":
        render_hud_mock(args.asset_dir, args.output)
    print(f"wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
