{ Socher Hayam - 32-bit Windows port wrapper.

  The game itself is the transpiled original source (port\gen\*.inc,
  produced by port\tools\transpile.py - do not edit the .inc files).
  This wrapper only supplies the platform (PlatformWin32) and the TP3
  runtime helpers (Tp3Compat), then includes the game verbatim.

  Compiler state matches Turbo Pascal 3:
    $MODE TP - 16-bit Integer, shortstrings, TP semantics
    $R- $Q-  - no range/overflow checks (the game relies on wraparound)
    $V-      - relaxed var-string checking
    $B-      - short-circuit boolean evaluation }
program SocherPort;

{$MODE TP}
{$R-}{$Q-}{$V-}{$B-}

uses
  PlatformWin32, Tp3Compat;

{$I ..\gen\globals.inc}
{$I ..\gen\code.inc}
