#!/usr/bin/env python3
"""Extract bitmap font sheets from DOS font loader binaries."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from PIL import Image, ImageDraw


def read_font(data: bytes, offset: int, glyph_height: int, count: int = 256) -> list[bytes]:
    size = count * glyph_height
    if offset < 0:
        offset = len(data) - size
    if offset < 0 or offset + size > len(data):
        raise ValueError(
            f"font range {offset}:{offset + size} is outside file length {len(data)}"
        )
    return [
        data[offset + index * glyph_height : offset + (index + 1) * glyph_height]
        for index in range(count)
    ]


def draw_sheet(
    glyphs: list[bytes],
    output: Path,
    glyph_height: int,
    *,
    scale: int = 3,
    margin: int = 2,
) -> None:
    glyph_width = 8
    columns = 16
    rows = (len(glyphs) + columns - 1) // columns
    label_height = 8
    cell_width = glyph_width + margin * 2
    cell_height = glyph_height + label_height + margin * 2
    image = Image.new("RGB", (columns * cell_width * scale, rows * cell_height * scale), "white")
    draw = ImageDraw.Draw(image)

    for code, glyph in enumerate(glyphs):
        column = code % columns
        row = code // columns
        base_x = column * cell_width * scale
        base_y = row * cell_height * scale
        draw.text((base_x + margin * scale, base_y), f"{code:02X}", fill="gray")
        glyph_x = base_x + margin * scale
        glyph_y = base_y + (label_height + margin) * scale
        for y, bits in enumerate(glyph):
            for x in range(glyph_width):
                if bits & (0x80 >> x):
                    draw.rectangle(
                        (
                            glyph_x + x * scale,
                            glyph_y + y * scale,
                            glyph_x + (x + 1) * scale - 1,
                            glyph_y + (y + 1) * scale - 1,
                        ),
                        fill="black",
                    )

    output.parent.mkdir(parents=True, exist_ok=True)
    image.save(output)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--offset", type=lambda value: int(value, 0), default=-1)
    parser.add_argument("--height", type=int, default=8)
    parser.add_argument("--count", type=int, default=256)
    parser.add_argument("--scale", type=int, default=3)
    args = parser.parse_args(argv)

    glyphs = read_font(args.input.read_bytes(), args.offset, args.height, args.count)
    draw_sheet(glyphs, args.output, args.height, scale=args.scale)
    print(f"wrote {args.output} ({len(glyphs)} glyphs, height {args.height})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
