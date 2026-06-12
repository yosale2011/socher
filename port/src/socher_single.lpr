program SocherHayam;

{ Single-file distribution build: identical to socher.lpr except that the
  Bootstrap unit embeds all game assets and extracts them at startup to
  %LOCALAPPDATA%\SocherHayam (where winners.win and port.cfg also live). }

{$MODE TP}{$R-}{$Q-}{$V-}{$B-}

uses
  Bootstrap, PlatformWin32, Tp3Compat;

{$I ..\gen\globals.inc}
{$I ..\gen\code.inc}
