#!/usr/bin/env python3
"""Run local checks for the port prototype."""

from __future__ import annotations

import py_compile
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "port" / "tools"


PYTHON_FILES = [
    TOOLS / "decode_pic.py",
    TOOLS / "extract_font.py",
    TOOLS / "text_layer.py",
    TOOLS / "render_scene.py",
    TOOLS / "asset_viewer.py",
    TOOLS / "runtime_demo.py",
    ROOT / "build" / "validate.py",
]


COMMANDS = [
    [sys.executable, "port/tools/decode_pic.py", "socher1/MAINSCRN.SCR", "port/mainscrn.png"],
    [sys.executable, "port/tools/decode_pic.py", "socher1/MAP.WIN", "port/map.png"],
    [
        sys.executable,
        "port/tools/extract_font.py",
        "socher1/FONTHE8.COM",
        "port/fonthe8-sheet.png",
        "--height",
        "8",
        "--offset",
        "604",
    ],
    [sys.executable, "port/tools/render_scene.py", "main-map", "port/scene-main-map.png"],
    [sys.executable, "port/tools/render_scene.py", "message", "port/scene-message.png"],
    [sys.executable, "port/tools/render_scene.py", "hud-mock", "port/scene-hud-bitmap.png"],
    [
        sys.executable,
        "port/tools/runtime_demo.py",
        "--snapshot",
        "port/demo-main.png",
        "--mode",
        "main",
        "--selected",
        "0",
    ],
    [
        sys.executable,
        "port/tools/runtime_demo.py",
        "--snapshot",
        "port/demo-buy.png",
        "--mode",
        "window",
        "--selected",
        "0",
        "--sub-selected",
        "1",
    ],
    [
        sys.executable,
        "port/tools/runtime_demo.py",
        "--snapshot",
        "port/demo-bank.png",
        "--mode",
        "window",
        "--selected",
        "3",
        "--sub-selected",
        "0",
    ],
    [
        sys.executable,
        "port/tools/runtime_demo.py",
        "--snapshot",
        "port/demo-buy-action.png",
        "--mode",
        "window",
        "--selected",
        "0",
        "--sub-selected",
        "2",
        "--action",
        "buy",
        "--amount",
        "3",
    ],
    [
        sys.executable,
        "port/tools/runtime_demo.py",
        "--snapshot",
        "port/demo-input.png",
        "--mode",
        "input",
        "--selected",
        "0",
        "--sub-selected",
        "2",
        "--amount",
        "12",
    ],
    [
        sys.executable,
        "port/tools/runtime_demo.py",
        "--snapshot",
        "port/demo-travel.png",
        "--mode",
        "window",
        "--selected",
        "2",
        "--sub-selected",
        "0",
    ],
]


def main() -> int:
    for path in PYTHON_FILES:
        py_compile.compile(str(path), doraise=True)
        print(f"compiled {path.relative_to(ROOT)}")

    for command in COMMANDS:
        print("running " + " ".join(command))
        subprocess.run(command, cwd=ROOT, check=True)

    print("port checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
