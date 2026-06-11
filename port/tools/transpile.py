#!/usr/bin/env python3
"""Mechanical transpiler for the Socher Hayam Win32 port.

Reads the annotated original sources (UTF-8):
    GLOBALS.PAS, CODE.PAS
and emits CP862-encoded include files for the Free Pascal port:
    port/gen/globals.inc, port/gen/code.inc

It is a deterministic, idempotent line/regex rewriter:
  * Strips the leading 'program Socher;' line from GLOBALS.PAS.
  * Renames TP3 platform identifiers to their PlatformWin32 equivalents
    (GraphColorMode -> PortGraphColorMode, Random -> Tp3Random, ...).
    Lookarounds guarantee a name is never rewritten inside an
    already-prefixed identifier, so applying the transform twice is a
    no-op (verified by a built-in self-test on every run).
  * Rewrites 'Read(KBD, X)' to 'X := PortReadKey' (TP3 KBD device).
  * Applies two literal port patches (documented in emitted comments):
      - 'GetDir(0, CurrentDir);'              -> "CurrentDir := '';"
      - "ExtraFilesPath := '\\kika\\socher_b\\';" -> "ExtraFilesPath := '';"
    All assets live in one directory; the exe runs with CWD=socher1.
  * String literals come out as raw CP862 bytes (Hebrew text is stored
    in visual order and must stay byte-identical).

Caveat: the renames are plain text substitutions, applied inside string
literals and comments too.  This is safe for these sources (audited: no
game literal contains a renamed identifier) but would need a tokenizer
if the originals ever change in that direction.

Usage:
    python transpile.py          # generate port/gen/globals.inc, code.inc
    python transpile.py --check  # verify generated files contain no
                                 # un-replaced platform identifiers
"""

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent.parent
GEN = REPO / "port" / "gen"

SOURCES = {
    "GLOBALS.PAS": "globals.inc",
    "CODE.PAS": "code.inc",
}

# Identifier renames. Lookarounds ((?<![A-Za-z0-9_]) ... (?![A-Za-z0-9_]))
# ensure whole-identifier matching only, which also makes the rewrite
# idempotent: 'PortPutPic' / 'Tp3Randomize' never match again.
RENAMES = [
    ("GraphColorMode", "PortGraphColorMode"),
    ("GraphBackground", "PortGraphBackground"),
    ("Palette", "PortPalette"),
    ("ColorTable", "PortColorTable"),
    ("PutPic", "PortPutPic"),
    ("GetPic", "PortGetPic"),
    ("GotoXY", "PortGotoXY"),
    ("WhereX", "PortWhereX"),
    ("WhereY", "PortWhereY"),
    ("TextColor", "PortTextColor"),
    ("KeyPressed", "PortKeyPressed"),
    ("NoSound", "PortNoSound"),
    ("Sound", "PortSound"),
    ("Delay", "PortDelay"),
    ("Randomize", "Tp3Randomize"),
    ("Random", "Tp3Random"),
]


def word_re(name):
    return re.compile(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"(?![A-Za-z0-9_])")


# 'Read(KBD, X)' -> 'X := PortReadKey'  (TP3 reads the keyboard device
# one char at a time; PortReadKey emulates it, indentation is kept).
KBD_READ_RE = re.compile(
    r"Read\(KBD,\s*([A-Za-z_][A-Za-z0-9_]*)\)"
)

# Literal port patches: original line content -> replacement line content.
# Comments emitted into the output document each patch.
PORT_PATCHES = [
    (
        "GetDir(0, CurrentDir);",
        "CurrentDir := ''; "
        "{ PORT PATCH: was GetDir(0, CurrentDir); "
        "exe runs with CWD=socher1, no drive check needed }",
    ),
    (
        "ExtraFilesPath := '\\kika\\socher_b\\';",
        "ExtraFilesPath := ''; "
        "{ PORT PATCH: was ExtraFilesPath := '\\kika\\socher_b\\'; "
        "all assets live in the working directory }",
    ),
    # THighScoreEntry is stored on disk (winners.win, 'file of').  In TP3
    # 'real' is the 6-byte Real48; in FPC it is an 8-byte double, which
    # would change the record size (27 -> 29+) and misread the original
    # file.  Tp3Compat exposes Tp3GameReal = Real48 (6 bytes, binary
    # compatible).  FPC can read Real48 in expressions but cannot assign
    # a double TO one, so the two assignment sites go through
    # DoubleToTp3GameReal (also in Tp3Compat).
    (
        "Score: real;",
        "Score: Tp3GameReal; "
        "{ PORT PATCH: was Score: real; TP3 real = 6-byte Real48, "
        "keeps THighScoreEntry at 27 bytes for winners.win }",
    ),
    (
        "HighScores[NewHighScoreIndex].Score := FinalScore;",
        "HighScores[NewHighScoreIndex].Score := DoubleToTp3GameReal(FinalScore); "
        "{ PORT PATCH: FPC cannot assign a double directly to Real48 }",
    ),
    # FPC reads Real48 only via assignment; comparison operators and
    # Write formatting on a Real48 do not compile, so the two read
    # sites go through Tp3GameRealToDouble (Tp3Compat).
    (
        "if (NewHighScoreIndex = 0) and "
        "(HighScores[HighScoreCount].Score < FinalScore) then",
        "if (NewHighScoreIndex = 0) and "
        "(Tp3GameRealToDouble(HighScores[HighScoreCount].Score) < FinalScore) then "
        "{ PORT PATCH: FPC cannot compare Real48 directly }",
    ),
    (
        "Write(HighScores[I].Score:6:0);",
        "Write(Tp3GameRealToDouble(HighScores[I].Score):6:0); "
        "{ PORT PATCH: FPC cannot Write a Real48 directly }",
    ),
]

# Context patches: like PORT_PATCHES but anchored on the previous
# source line, for target lines whose text is not unique in the file.
# (prev_line_stripped, line_stripped, replacement_line_content)
CONTEXT_PATCHES = [
    # ZeroPad3 (nested inside ToStrWithCommas) runs a for loop over the
    # ENCLOSING function's counter I - legal in TP3, rejected by FPC
    # ("Illegal counter variable").  The conditional double pad is
    # exactly equivalent to padding while shorter than 3 chars.
    (
        "Str(NumberSegment:1:0, NumberString);",
        "for I := 1 to 2 do",
        "while Length(NumberString) < 3 do "
        "{ PORT PATCH: was for I := 1 to 2 do; FPC rejects a for-counter "
        "owned by the enclosing function; pads the same zeroes }",
    ),
]


def transpile(name, text):
    """Apply all mechanical rewrites to one source file's text."""
    lines = text.split("\n")

    if name == "GLOBALS.PAS":
        if lines and lines[0].strip() == "program Socher;":
            lines = lines[1:]

    out_lines = []
    prev_stripped = ""
    for line in lines:
        cur_stripped = line.strip()
        for original, patched in PORT_PATCHES:
            if cur_stripped == original:
                indent = line[: len(line) - len(line.lstrip())]
                line = indent + patched
                break
        for anchor, original, patched in CONTEXT_PATCHES:
            if cur_stripped == original and prev_stripped == anchor:
                indent = line[: len(line) - len(line.lstrip())]
                line = indent + patched
                break
        prev_stripped = cur_stripped
        line = KBD_READ_RE.sub(r"\1 := PortReadKey", line)
        for old, new in RENAMES:
            line = word_re(old).sub(new, line)
        out_lines.append(line)

    return "\n".join(out_lines)


# Patterns that must NOT appear in the generated output (whole words /
# literal fragments left over from an incomplete rewrite).
def forbidden_patterns():
    pats = [(old, word_re(old)) for old, _new in RENAMES]
    pats.append(("Read(KBD", re.compile(r"Read\(KBD")))
    pats.append(("GetDir", word_re("GetDir")))
    pats.append(("\\kika\\socher_b", re.compile(re.escape("\\kika\\socher_b"))))
    pats.append(("program Socher", re.compile(r"^\s*program\s+Socher\s*;", re.M | re.I)))
    pats.append(("Score: real", re.compile(r"(?<![A-Za-z0-9_])Score:\s*real\b")))
    pats.append((".Score := FinalScore", re.compile(re.escape(".Score := FinalScore"))))
    pats.append((".Score < FinalScore", re.compile(re.escape(".Score < FinalScore"))))
    pats.append(("Write(HighScores[I].Score", re.compile(re.escape("Write(HighScores[I].Score"))))
    return pats


def check():
    failed = False
    for src_name, gen_name in SOURCES.items():
        gen_path = GEN / gen_name
        if not gen_path.exists():
            print(f"CHECK FAIL: {gen_path} does not exist")
            failed = True
            continue
        text = gen_path.read_bytes().decode("cp862")
        # Strip single-line { ... } comments: the emitted PORT PATCH
        # comments quote the original (forbidden) text on purpose.
        comment_re = re.compile(r"\{[^}]*\}")
        for label, pat in forbidden_patterns():
            hits = [
                i + 1
                for i, line in enumerate(text.split("\n"))
                if pat.search(comment_re.sub("", line))
            ]
            if hits:
                print(
                    f"CHECK FAIL: {gen_name}: un-replaced '{label}' "
                    f"on line(s) {hits[:10]}"
                )
                failed = True
    if failed:
        return 1
    print("check OK: no un-replaced platform identifiers in generated files")
    return 0


def generate():
    GEN.mkdir(parents=True, exist_ok=True)
    for src_name, gen_name in SOURCES.items():
        src_path = REPO / src_name
        text = src_path.read_text(encoding="utf-8")
        once = transpile(src_name, text)
        twice = transpile(src_name, once)
        if once != twice:
            print(f"ERROR: transpile of {src_name} is not idempotent")
            return 1

        header = (
            "{ GENERATED FILE - do not edit. "
            f"Produced by port/tools/transpile.py from {src_name}. }}\n"
        )
        out = header + once
        data = out.encode("cp862")
        gen_path = GEN / gen_name

        # Sanity checks on this run's transform.
        if src_name == "CODE.PAS":
            n_readkey = out.count(":= PortReadKey")
            if n_readkey != 3:
                print(f"ERROR: expected 3 Read(KBD,..) rewrites, got {n_readkey}")
                return 1
            for original, _patched in PORT_PATCHES:
                if original not in text:
                    print(f"ERROR: expected patch target not found: {original}")
                    return 1

        gen_path.write_bytes(data)
        print(f"wrote {gen_path} ({len(data)} bytes)")
    return 0


def main(argv):
    if "--check" in argv:
        return check()
    rc = generate()
    if rc:
        return rc
    return check()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
