#!/usr/bin/env python3
"""Interactive prototype for the Socher Hayam port runtime."""

from __future__ import annotations

import tkinter as tk
from dataclasses import dataclass
from pathlib import Path
import argparse

from PIL import Image, ImageDraw, ImageTk

from render_scene import FrameBuffer, read_asset
from text_layer import TextLayer, right_aligned


@dataclass(frozen=True)
class Option:
    name: str
    rect: tuple[int, int, int, int]
    window: str | None = None
    window_x: int = 0
    window_bottom_y: int = 142
    sub_options: tuple[tuple[int, int, int, int], ...] = ()


@dataclass
class GameState:
    money: int = 3000
    bank_balance: int = 0
    cargo_copper: int = 0
    cargo_olives: int = 0
    cargo_wheat: int = 0
    copper_price: int = 3000
    olives_price: int = 500
    wheat_price: int = 50
    country: str = "לארשי"
    day: str = "ןושאר"
    hour: int = 8
    message: str = ""
    show_message_box: bool = False
    trade_amount: int = 1
    ship_capacity: int = 100

    def prices(self) -> list[int]:
        return [self.copper_price, self.olives_price, self.wheat_price]

    def cargo(self) -> list[int]:
        return [self.cargo_copper, self.cargo_olives, self.cargo_wheat]

    def total_cargo(self) -> int:
        return self.cargo_copper + self.cargo_olives + self.cargo_wheat

    def add_cargo(self, index: int, amount: int) -> None:
        if index == 0:
            self.cargo_copper += amount
        elif index == 1:
            self.cargo_olives += amount
        elif index == 2:
            self.cargo_wheat += amount

    def remove_cargo(self, index: int, amount: int) -> None:
        if index == 0:
            self.cargo_copper -= amount
        elif index == 1:
            self.cargo_olives -= amount
        elif index == 2:
            self.cargo_wheat -= amount


OPTIONS = [
    Option(
        "buy",
        (131, 36, 177, 45),
        "BUYISR.WIN",
        1,
        142,
        ((177, 2, 236, 18), (8, 2, 67, 18), (179, 86, 236, 101)),
    ),
    Option(
        "sell",
        (126, 51, 174, 60),
        "SELLISR.WIN",
        0,
        142,
        ((19, 26, 59, 35), (19, 42, 59, 51), (19, 58, 59, 67)),
    ),
    Option(
        "travel",
        (125, 67, 171, 76),
        "MAP.WIN",
        0,
        142,
        ((144, 2, 182, 9), (228, 106, 237, 137), (75, 134, 111, 141)),
    ),
    Option(
        "bank",
        (81, 83, 167, 92),
        "BANK.WIN",
        0,
        142,
        ((97, 7, 149, 16), (16, 7, 64, 16)),
    ),
    Option("stay", (42, 99, 162, 108), "STAY.WIN", 0, 142),
    Option("repair", (33, 114, 160, 123), "REPAIR.WIN", 0, 142),
]

COUNTRIES = ["היכרות", "לארשי", "םירצמ"]


def image_to_cga_pixels(image: Image.Image) -> list[int]:
    raw = image.convert("RGB").tobytes()
    return [
        RuntimeDemo.rgb_to_index((raw[index], raw[index + 1], raw[index + 2]))
        for index in range(0, len(raw), 3)
    ]


class RuntimeDemo:
    def __init__(self, root: tk.Tk, asset_dir: Path, scale: int) -> None:
        self.root = root
        self.asset_dir = asset_dir
        self.scale = scale
        self.mode = "main"
        self.selected = 0
        self.sub_selected = 0
        self.input_buffer = ""
        self.state = GameState()
        self.canvas = tk.Canvas(root, bg="black", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)
        self.photo: ImageTk.PhotoImage | None = None

        root.bind("<Up>", lambda _event: self.move(-1))
        root.bind("<Down>", lambda _event: self.move(1))
        root.bind("<Left>", lambda _event: self.move(-1))
        root.bind("<Right>", lambda _event: self.move(1))
        root.bind("<Escape>", lambda _event: self.back())
        root.bind("<BackSpace>", lambda _event: self.backspace())
        root.bind("<Return>", lambda _event: self.activate())
        root.bind("<Key>", self.keypress)
        root.bind("+", lambda _event: self.change_trade_amount(1))
        root.bind("-", lambda _event: self.change_trade_amount(-1))
        root.bind("b", lambda _event: self.open_named("buy"))
        root.bind("s", lambda _event: self.open_named("sell"))
        root.bind("t", lambda _event: self.open_named("travel"))
        root.bind("k", lambda _event: self.open_named("bank"))

        self.render()

    def move(self, delta: int) -> None:
        if self.state.show_message_box or self.mode == "input":
            return
        if self.mode == "main":
            self.selected = (self.selected + delta) % len(OPTIONS)
        else:
            sub_options = OPTIONS[self.selected].sub_options
            if sub_options:
                self.sub_selected = (self.sub_selected + delta) % len(sub_options)
        self.render()

    def open_named(self, name: str) -> None:
        if self.state.show_message_box or self.mode == "input":
            return
        for index, option in enumerate(OPTIONS):
            if option.name == name:
                self.selected = index
                self.sub_selected = 0
                self.mode = "window"
                self.render()
                return

    def change_trade_amount(self, delta: int) -> None:
        if self.mode == "input":
            return
        self.state.trade_amount = max(1, min(99, self.state.trade_amount + delta))
        self.state.message = f"{self.state.trade_amount} :תומכ"
        self.state.show_message_box = False
        self.render()

    def keypress(self, event: tk.Event) -> None:
        if self.mode != "input":
            return
        if event.char.isdigit() and len(self.input_buffer) < 5:
            self.input_buffer += event.char
            self.render()

    def activate(self) -> None:
        if self.state.show_message_box:
            self.state.show_message_box = False
            self.state.message = ""
        elif self.mode == "main":
            self.mode = "window"
            self.sub_selected = 0
        elif self.mode == "input":
            amount = int(self.input_buffer or "0")
            self.input_buffer = ""
            self.apply_window_action(amount)
        else:
            option = OPTIONS[self.selected].name
            if option in {"buy", "sell", "bank"}:
                self.input_buffer = str(self.state.trade_amount)
                self.mode = "input"
            else:
                self.apply_window_action(self.state.trade_amount)
        self.render()

    def apply_window_action(self, amount: int) -> None:
        option = OPTIONS[self.selected].name
        amount = max(0, amount)
        if option == "buy":
            price = self.state.prices()[self.sub_selected]
            total = price * amount
            if amount <= 0:
                self.state.message = "הלועפה הלטוב"
            elif self.state.total_cargo() + amount > self.state.ship_capacity:
                self.state.message = "ידמ לקשמ רתוי"
            elif self.state.money >= total:
                self.state.money -= total
                self.state.add_cargo(self.sub_selected, amount)
                self.state.message = f"{amount} ונקנ"
            else:
                self.state.message = "קיפסמ ףסכ ןיא"
            self.state.show_message_box = True
        elif option == "sell":
            cargo = self.state.cargo()[self.sub_selected]
            if amount <= 0:
                self.state.message = "הלועפה הלטוב"
            elif cargo >= amount:
                self.state.remove_cargo(self.sub_selected, amount)
                self.state.money += self.state.prices()[self.sub_selected] * amount
                self.state.message = f"{amount} ורכמנ"
            else:
                self.state.message = "רוכמל המ ןיא"
            self.state.show_message_box = True
        elif option == "bank":
            bank_amount = 100 * amount
            if amount <= 0:
                self.state.message = "הלועפה הלטוב"
            elif self.sub_selected == 0 and self.state.money >= bank_amount:
                self.state.money -= bank_amount
                self.state.bank_balance += bank_amount
                self.state.message = f"{bank_amount} ודקפוה"
            elif self.sub_selected == 1 and self.state.bank_balance >= bank_amount:
                self.state.bank_balance -= bank_amount
                self.state.money += bank_amount
                self.state.message = f"{bank_amount} וכשמנ"
            else:
                self.state.message = "הלועפה הרשפאתה אל"
            self.state.show_message_box = True
        elif option == "stay":
            self.state.hour += 1
            self.state.message = "העש הרבע"
            self.state.show_message_box = True
        elif option == "repair":
            self.state.message = "ןוקית ךשמהב"
            self.state.show_message_box = True
        elif option == "travel":
            destination = COUNTRIES[self.sub_selected]
            if destination == self.state.country:
                self.state.message = "רבכ ןאכ התא"
            else:
                duration = 4 if (destination == "לארשי" or self.state.country == "לארשי") else 8
                if self.state.hour + duration > 20:
                    self.state.message = "תרחואמ העשה"
                else:
                    self.state.country = destination
                    self.state.hour += duration
                    self.state.message = "העגה העצוב"
            self.state.show_message_box = True

        if option in {"stay", "repair", "travel", "buy", "sell", "bank"}:
            self.mode = "main"

    def back(self) -> None:
        if self.state.show_message_box:
            self.state.show_message_box = False
            self.state.message = ""
            self.render()
        elif self.mode == "input":
            self.input_buffer = ""
            self.mode = "window"
            self.render()
        elif self.mode == "window":
            self.mode = "main"
            self.render()
        else:
            self.root.destroy()

    def backspace(self) -> None:
        if self.mode == "input":
            self.input_buffer = self.input_buffer[:-1]
            self.render()
        else:
            self.back()

    def render(self) -> None:
        fb = FrameBuffer(color=1)
        fb.put_pic(read_asset(self.asset_dir, "MAINSCRN.SCR"), 0, 199)
        self.draw_hud(fb, self.state)
        if self.mode == "main":
            self.highlight(fb, OPTIONS[self.selected].rect)
        else:
            option = OPTIONS[self.selected]
            if option.window is not None:
                fb.put_pic(
                    read_asset(self.asset_dir, option.window),
                    option.window_x,
                    option.window_bottom_y,
                )
            if option.sub_options:
                self.highlight(fb, option.sub_options[self.sub_selected])
        if self.state.show_message_box and self.state.message:
            self.draw_message_box(fb, self.state.message)
        if self.mode == "input":
            self.draw_input_box(fb)
        image = fb.to_image()
        if self.scale > 1:
            image = image.resize(
                (image.width * self.scale, image.height * self.scale),
                Image.Resampling.NEAREST,
            )
        self.photo = ImageTk.PhotoImage(image)
        self.canvas.config(width=self.photo.width(), height=self.photo.height())
        self.canvas.delete("all")
        self.canvas.create_image(0, 0, anchor=tk.NW, image=self.photo)
        self.root.title(
            f"Socher port demo - {self.mode}:{OPTIONS[self.selected].name} "
            f"[{self.selected + 1}/{len(OPTIONS)}]"
        )

    def draw_hud(self, fb: FrameBuffer, state: GameState) -> None:
        self.draw_hud_static(fb, state)

    @staticmethod
    def draw_hud_static(fb: FrameBuffer, state: GameState | None = None) -> None:
        state = state or GameState()
        image = fb.to_image()
        text = TextLayer(ImageDraw.Draw(image))
        text.text_color(3)
        text.write_at(73, 1, right_aligned(state.country, 6))
        text.write_at(32, 3, state.day)
        text.write_at(32, 5, right_aligned(str(state.hour), 2))
        text.write_at(34, 5, ":00")
        text.write_at(33, 17, right_aligned(f"{state.money:,}", 5))
        text.write_at(32, 9, right_aligned(str(state.cargo_copper), 3))
        text.write_at(32, 11, right_aligned(str(state.cargo_olives), 3))
        text.write_at(32, 13, right_aligned(str(state.cargo_wheat), 3))
        for cell_x in (30, 16, 2):
            text.write_at(cell_x, 21, right_aligned(str(state.copper_price), 4))
            text.write_at(cell_x, 23, right_aligned(str(state.olives_price), 4))
            text.write_at(cell_x, 25, right_aligned(str(state.wheat_price), 4))
        if state.message:
            text.write_at(3, 18, state.message[:28])
        text.write_at(2, 19, f"x{state.trade_amount}")
        fb.pixels = image_to_cga_pixels(image)

    def draw_message_box(self, fb: FrameBuffer, message: str) -> None:
        fb.put_pic(read_asset(self.asset_dir, "MESSAGE.SGN"), 10, 138)
        image = fb.to_image()
        text = TextLayer(ImageDraw.Draw(image))
        text.text_color(3)
        text.write_at(max(1, 29 - len(message)), 16, message[:35])
        fb.pixels = image_to_cga_pixels(image)

    def draw_input_box(self, fb: FrameBuffer) -> None:
        fb.put_pic(read_asset(self.asset_dir, "MESSAGE.SGN"), 10, 138)
        image = fb.to_image()
        text = TextLayer(ImageDraw.Draw(image))
        text.text_color(3)
        text.write_at(12, 15, "תומכ")
        text.write_at(12, 16, self.input_buffer or "0")
        fb.pixels = image_to_cga_pixels(image)

    def highlight(self, fb: FrameBuffer, rect: tuple[int, int, int, int]) -> None:
        self.highlight_static(fb, rect)

    @staticmethod
    def highlight_static(fb: FrameBuffer, rect: tuple[int, int, int, int]) -> None:
        x1, y1, x2, y2 = rect
        table = [3, 2, 1, 0]
        for y in range(y1, y2 + 1):
            for x in range(x1, x2 + 1):
                index = y * fb.width + x
                fb.pixels[index] = table[fb.pixels[index] & 0x03]

    @staticmethod
    def rgb_to_index(pixel: tuple[int, int, int]) -> int:
        palette = [(0, 0, 0), (0, 170, 170), (170, 0, 170), (170, 170, 170)]
        return min(
            range(4),
            key=lambda index: sum(
                abs(pixel[channel] - palette[index][channel]) for channel in range(3)
            ),
        )


def render_demo_image(
    asset_dir: Path,
    mode: str,
    selected: int,
    sub_selected: int,
    state: GameState | None = None,
):
    demo_state = DemoRenderer(asset_dir, mode, selected, sub_selected, state or GameState())
    return demo_state.render_image()


class DemoRenderer:
    def __init__(
        self,
        asset_dir: Path,
        mode: str,
        selected: int,
        sub_selected: int,
        state: GameState,
    ) -> None:
        self.asset_dir = asset_dir
        self.mode = mode
        self.selected = selected
        self.sub_selected = sub_selected
        self.state = state

    def render_image(self):
        fb = FrameBuffer(color=1)
        fb.put_pic(read_asset(self.asset_dir, "MAINSCRN.SCR"), 0, 199)
        RuntimeDemo.draw_hud_static(fb, self.state)
        if self.mode == "main":
            RuntimeDemo.highlight_static(fb, OPTIONS[self.selected].rect)
        else:
            option = OPTIONS[self.selected]
            if option.window is not None:
                fb.put_pic(read_asset(self.asset_dir, option.window), option.window_x, option.window_bottom_y)
            if option.sub_options:
                RuntimeDemo.highlight_static(fb, option.sub_options[self.sub_selected])
        if self.mode == "input":
            fb.put_pic(read_asset(self.asset_dir, "MESSAGE.SGN"), 10, 138)
            image = fb.to_image()
            text = TextLayer(ImageDraw.Draw(image))
            text.text_color(3)
            text.write_at(12, 15, "תומכ")
            text.write_at(12, 16, str(self.state.trade_amount))
            fb.pixels = image_to_cga_pixels(image)
        if self.state.show_message_box and self.state.message:
            fb.put_pic(read_asset(self.asset_dir, "MESSAGE.SGN"), 10, 138)
            image = fb.to_image()
            text = TextLayer(ImageDraw.Draw(image))
            text.text_color(3)
            text.write_at(max(1, 29 - len(self.state.message)), 16, self.state.message[:35])
            fb.pixels = image_to_cga_pixels(image)
        return fb.to_image()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--snapshot", type=Path)
    parser.add_argument("--mode", choices=["main", "window", "input"], default="main")
    parser.add_argument("--selected", type=int, default=0)
    parser.add_argument("--sub-selected", type=int, default=0)
    parser.add_argument("--action", choices=["none", "buy", "sell", "deposit", "withdraw"], default="none")
    parser.add_argument("--amount", type=int, default=1)
    args = parser.parse_args()

    if args.snapshot is not None:
        state = GameState(trade_amount=args.amount)
        if args.action == "buy":
            price = state.prices()[args.sub_selected]
            if state.money >= price * args.amount:
                state.money -= price * args.amount
                state.add_cargo(args.sub_selected, args.amount)
                state.message = f"{args.amount} ונקנ"
                state.show_message_box = True
        elif args.action == "sell":
            state.add_cargo(args.sub_selected, args.amount)
            state.remove_cargo(args.sub_selected, args.amount)
            state.money += state.prices()[args.sub_selected] * args.amount
            state.message = f"{args.amount} ורכמנ"
            state.show_message_box = True
        elif args.action == "deposit":
            bank_amount = 100 * args.amount
            state.money -= bank_amount
            state.bank_balance += bank_amount
            state.message = f"{bank_amount} ודקפוה"
            state.show_message_box = True
        elif args.action == "withdraw":
            bank_amount = 100 * args.amount
            state.bank_balance += bank_amount
            state.bank_balance -= bank_amount
            state.money += bank_amount
            state.message = f"{bank_amount} וכשמנ"
            state.show_message_box = True

        image = render_demo_image(
            Path("socher1"), args.mode, args.selected, args.sub_selected, state
        )
        args.snapshot.parent.mkdir(parents=True, exist_ok=True)
        image.save(args.snapshot)
        print(f"wrote {args.snapshot}")
        return

    root = tk.Tk()
    RuntimeDemo(root, Path("socher1"), scale=3)
    root.mainloop()


if __name__ == "__main__":
    main()
