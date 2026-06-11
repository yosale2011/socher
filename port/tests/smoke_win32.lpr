program smoke_win32;

{ Smoke test for the Win32 GDI backend (PlatformWin32).

  Run headless-scripted:
    $env:SOCHER_KEYS = 'port\tests\empty_keys.txt'   (empty file)
    $env:SOCHER_DUMP_DIR = 'port\tests\out'
  The final PortReadKey dumps frame-0000.ppm with the current palette
  and Halts(0) because the key script is exhausted. }

{$mode objfpc}{$H+}

uses
  SysUtils, PlatformWin32;

const
  ScrSize = 16128;

var
  Buf: array[0..ScrSize - 1] of Byte;
  F: file;
  ScrPath: string;
  BytesRead: Integer;

function FindScrPath: string;
const
  Relative = 'socher1' + DirectorySeparator + 'MAINSCRN.SCR';
var
  Base, Up: string;
  I: Integer;
begin
  if FileExists(Relative) then
    Exit(Relative);
  Base := ExtractFilePath(ParamStr(0));
  Up := '';
  for I := 0 to 4 do
  begin
    if FileExists(Base + Up + Relative) then
      Exit(Base + Up + Relative);
    Up := Up + '..' + DirectorySeparator;
  end;
  raise Exception.Create('cannot find socher1\MAINSCRN.SCR');
end;

begin
  ScrPath := FindScrPath;
  Assign(F, ScrPath);
  Reset(F, 1);
  BlockRead(F, Buf, ScrSize, BytesRead);
  Close(F);
  if BytesRead <> ScrSize then
    raise Exception.CreateFmt('expected %d bytes in %s, got %d',
      [ScrSize, ScrPath, BytesRead]);

  PortGraphColorMode;
  PortPalette(0);
  PortGraphBackground(1);
  PortPutPic(Buf, 0, 199);

  PortGotoXY(10, 5);
  Write('SHALOM 123');

  PortSound(700);
  PortDelay(100);
  PortNoSound;

  PortDelay(300);

  { Scripted mode: dumps a PPM, then Halt(0) when the (empty) key
    script is exhausted. Interactive mode: waits for a key. }
  PortReadKey;
end.
