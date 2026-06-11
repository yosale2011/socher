"""40x25 text layer helpers for Socher Hayam render prototypes."""

from __future__ import annotations

from pathlib import Path

from PIL import ImageDraw, ImageFont

from decode_pic import CGA_PALETTE


class DosBitmapFont:
    def __init__(self, path: Path, glyph_height: int = 8, offset: int = -1) -> None:
        data = path.read_bytes()
        size = 256 * glyph_height
        if offset < 0:
            if path.name.upper() == "FONTHE8.COM" and glyph_height == 8:
                offset = 604
            else:
                offset = len(data) - size
        self.glyph_height = glyph_height
        self.glyphs = [
            data[offset + code * glyph_height : offset + (code + 1) * glyph_height]
            for code in range(256)
        ]

    def draw_char(
        self,
        draw: ImageDraw.ImageDraw,
        x: int,
        y: int,
        char: str,
        fill: tuple[int, int, int],
    ) -> None:
        try:
            code = char.encode("cp862")[0]
        except UnicodeEncodeError:
            code = ord("?")
        glyph = self.glyphs[code]
        for row, bits in enumerate(glyph):
            for bit in range(8):
                if bits & (0x80 >> bit):
                    draw.point((x + bit, y + row), fill=fill)


def find_font(size: int = 8) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    candidates = [
        Path("C:/Windows/Fonts/arialbd.ttf"),
        Path("C:/Windows/Fonts/arial.ttf"),
        Path("C:/Windows/Fonts/consolab.ttf"),
        Path("C:/Windows/Fonts/consola.ttf"),
    ]
    for path in candidates:
        if path.exists():
            return ImageFont.truetype(str(path), size)
    return ImageFont.load_default()


class TextLayer:
    def __init__(
        self,
        draw: ImageDraw.ImageDraw,
        font: ImageFont.FreeTypeFont | ImageFont.ImageFont | DosBitmapFont | None = None,
        cell_width: int = 8,
        cell_height: int = 8,
    ) -> None:
        self.draw = draw
        self.font = font or default_font()
        self.cell_width = cell_width
        self.cell_height = cell_height
        self.color = CGA_PALETTE[3]

    def text_color(self, color: int) -> None:
        self.color = CGA_PALETTE[color & 0x03]

    def write_at(
        self,
        cell_x: int,
        cell_y: int,
        text: str,
        *,
        color: int | None = None,
    ) -> None:
        fill = CGA_PALETTE[color & 0x03] if color is not None else self.color
        x = (cell_x - 1) * self.cell_width
        y = (cell_y - 1) * self.cell_height - 1

        # Draw each character independently to avoid modern bidi reordering.
        for index, char in enumerate(text):
            char_x = x + index * self.cell_width
            if isinstance(self.font, DosBitmapFont):
                self.font.draw_char(self.draw, char_x, y + 1, char, fill)
            else:
                self.draw.text((char_x, y), char, font=self.font, fill=fill)


def right_aligned(value: str, width: int) -> str:
    return value.rjust(width)


def default_font() -> ImageFont.FreeTypeFont | ImageFont.ImageFont | DosBitmapFont:
    font_path = Path("socher1/FONTHE8.COM")
    if font_path.exists():
        return DosBitmapFont(font_path)
    return find_font()
