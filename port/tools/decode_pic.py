#!/usr/bin/env python3
"""Decode Turbo Pascal 3 CGA picture buffers to PNG.

Socher Hayam assets are TP3 `GetPic` buffers saved in 128-byte block multiples.
The useful picture data starts with a 6-byte header:

    word marker, word width, word height

The remaining bytes are packed CGA 320x200x4 data. The file may contain padding
after the useful data because the original loader reads whole 128-byte blocks.
"""

from __future__ import annotations

import argparse
import struct
import sys
import zlib
from pathlib import Path


CGA_PALETTE = [
    (0x00, 0x00, 0x00),
    (0x00, 0xAA, 0xAA),
    (0xAA, 0x00, 0xAA),
    (0xAA, 0xAA, 0xAA),
]


def read_le_word(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


def parse_header(data: bytes) -> tuple[int, int, int]:
    if len(data) < 128:
        raise ValueError("file is too small to contain a TP picture header")
    marker = read_le_word(data, 0)
    width = read_le_word(data, 2)
    height = read_le_word(data, 4)
    if marker != 2:
        raise ValueError(f"unexpected TP picture marker {marker}; expected 2")
    return marker, width, height


def decode_pic(
    data: bytes,
    *,
    reverse_pixels_in_byte: bool = False,
    mirror_x: bool = False,
    mirror_y: bool = True,
) -> tuple[int, int, list[int]]:
    _, width, height = parse_header(data)

    pixels = [0] * (width * height)
    bytes_per_scanline = (width + 3) // 4
    useful_size = 6 + bytes_per_scanline * height
    if len(data) < useful_size:
        raise ValueError(
            f"file is too small for {width}x{height}: "
            f"need at least {useful_size} bytes, got {len(data)}"
        )

    payload = data[6:useful_size]

    for y in range(height):
        row = y * bytes_per_scanline
        for x_byte in range(bytes_per_scanline):
            packed = payload[row + x_byte]
            x = x_byte * 4
            for bit_pair in range(4):
                pixel_index = 3 - bit_pair if reverse_pixels_in_byte else bit_pair
                target_x = x + pixel_index
                if target_x < width:
                    shift = 6 - bit_pair * 2
                    dst_x = width - 1 - target_x if mirror_x else target_x
                    dst_y = height - 1 - y if mirror_y else y
                    pixels[dst_y * width + dst_x] = (packed >> shift) & 0x03

    return width, height, pixels


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + kind
        + payload
        + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
    )


def write_png(path: Path, width: int, height: int, pixels: list[int]) -> None:
    rows = []
    for y in range(height):
        row = bytearray([0])
        for x in range(width):
            row.extend(CGA_PALETTE[pixels[y * width + x]])
        rows.append(bytes(row))

    raw = b"".join(rows)
    png = (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + png_chunk(b"IDAT", zlib.compress(raw, 9))
        + png_chunk(b"IEND", b"")
    )
    path.write_bytes(png)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--reverse-pixels-in-byte", action="store_true")
    parser.add_argument("--mirror-x", action="store_true")
    parser.add_argument(
        "--no-mirror-y",
        action="store_false",
        dest="mirror_y",
        help="disable the default vertical flip used by TP picture buffers",
    )
    parser.set_defaults(mirror_y=True)
    args = parser.parse_args(argv)

    data = args.input.read_bytes()
    width, height, pixels = decode_pic(
        data,
        reverse_pixels_in_byte=args.reverse_pixels_in_byte,
        mirror_x=args.mirror_x,
        mirror_y=args.mirror_y,
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_png(args.output, width, height, pixels)
    print(f"wrote {args.output} ({width}x{height})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
