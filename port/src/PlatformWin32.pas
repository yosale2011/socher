unit PlatformWin32;

{ Win32 GDI backend for the Socher Hayam 32-bit port.

  Implements the platform contract used by the transpiled game code:
  a 320x200 4-color indexed framebuffer presented in a window scaled by
  an integer factor (SOCHER_SCALE, default 3), a TP3-style KBD key queue,
  a custom Output text-device driver that renders bytes as 8x8 CP862
  glyphs from FONTHE8.COM, and PC-speaker-ish sound via Windows Beep.

  Scripted-test hooks:
    SOCHER_KEYS     = path to a key-script file (one token per line:
                      ENTER, ESC, UP, DOWN, LEFT, RIGHT, F10, SPACE,
                      WAIT (dump a frame), or a single literal char).
                      When set, PortReadKey pops from the script and the
                      program Halts(0) when the script is exhausted.
    SOCHER_DUMP_DIR = directory; every PortReadKey call (before consuming)
                      saves the framebuffer as frame-NNNN.ppm using the
                      CURRENT palette.
    SOCHER_SCALE    = integer window scale factor (default 3). }

{$mode objfpc}{$H+}

interface

procedure PortGraphColorMode;
procedure PortGraphBackground(Color: Integer);
procedure PortPalette(N: Integer);
procedure PortColorTable(C1, C2, C3, C4: Integer);
procedure PortPutPic(var Buffer; X, Y: Integer);
procedure PortGetPic(var Buffer; X1, Y1, X2, Y2: Integer);

procedure PortTextColor(Color: Integer);
procedure PortGotoXY(X, Y: Integer);
function PortWhereX: Integer;
function PortWhereY: Integer;

function PortKeyPressed: Boolean;
function PortReadKey: Char;

procedure PortSound(Frequency: Integer);
procedure PortNoSound;
procedure PortDelay(Ms: Integer);

implementation

uses
  Windows, Classes, SysUtils, Picture, TextGrid;

const
  ScreenW = 320;
  ScreenH = 200;
  TextCols = 40;
  TextRows = 25;
  KeyQueueSize = 256;
  WindowClassName = 'SocherHayamWindow';
  WindowTitle = 'Socher Hayam - 32-bit port';

  { Full 16-color CGA table as $RRGGBB. }
  CgaColors: array[0..15] of DWORD = (
    $000000, $0000AA, $00AA00, $00AAAA,
    $AA0000, $AA00AA, $AA5500, $AAAAAA,
    $555555, $5555FF, $55FF55, $55FFFF,
    $FF5555, $FF55FF, $FFFF55, $FFFFFF);

  { CGA palette selects for entries 1..3 (entry 0 is the background). }
  CgaPalettes: array[0..3, 1..3] of DWORD = (
    ($00AA00, $AA0000, $AA5500),   { green, red, brown }
    ($00AAAA, $AA00AA, $AAAAAA),   { cyan, magenta, lightgray }
    ($55FF55, $FF5555, $FFFF55),   { lightgreen, lightred, yellow }
    ($55FFFF, $FF55FF, $FFFFFF));  { lightcyan, lightmagenta, white }

var
  FrameBuffer: TFrameBuffer;
  Grid: TTextGrid;
  PaletteRgb: array[0..3] of DWORD = ($000000, $00AAAA, $AA00AA, $AAAAAA);
  ColorTable: array[0..3] of TPixel = (0, 1, 2, 3);
  Initialized: Boolean = False;
  FontLoaded: Boolean = False;
  WindowHandle: HWND = 0;
  WindowClassRegistered: Boolean = False;
  Scale: Integer = 3;
  Pixels32: array[0..ScreenW * ScreenH - 1] of DWORD;

  KeyQueue: array[0..KeyQueueSize - 1] of Char;
  KeyHead: Integer = 0;
  KeyCount: Integer = 0;

  ActiveFrequency: Integer = 0;
  OutputInstalled: Boolean = False;

  ScriptMode: Boolean = False;
  ScriptTokens: TStringList = nil;
  ScriptIndex: Integer = 0;
  DumpDir: string = '';
  DumpCounter: Integer = 0;

procedure EnsureInitialized;
begin
  if not Initialized then
    PortGraphColorMode;
end;

{ ------------------------------------------------------------------ }
{ Key queue                                                          }
{ ------------------------------------------------------------------ }

procedure EnqueueKey(Ch: Char);
begin
  if KeyCount >= KeyQueueSize then
    Exit;
  KeyQueue[(KeyHead + KeyCount) mod KeyQueueSize] := Ch;
  Inc(KeyCount);
end;

function DequeueKey: Char;
begin
  if KeyCount = 0 then
    Exit(#0);
  Result := KeyQueue[KeyHead];
  KeyHead := (KeyHead + 1) mod KeyQueueSize;
  Dec(KeyCount);
end;

{ ------------------------------------------------------------------ }
{ Presentation                                                       }
{ ------------------------------------------------------------------ }

procedure PresentToDc(Dc: HDC);
var
  Bmi: BITMAPINFO;
  I: Integer;
begin
  for I := 0 to High(FrameBuffer.Pixels) do
    Pixels32[I] := PaletteRgb[FrameBuffer.Pixels[I] and $03];

  FillChar(Bmi, SizeOf(Bmi), 0);
  Bmi.bmiHeader.biSize := SizeOf(BITMAPINFOHEADER);
  Bmi.bmiHeader.biWidth := ScreenW;
  Bmi.bmiHeader.biHeight := -ScreenH; { top-down }
  Bmi.bmiHeader.biPlanes := 1;
  Bmi.bmiHeader.biBitCount := 32;
  Bmi.bmiHeader.biCompression := BI_RGB;

  SetStretchBltMode(Dc, COLORONCOLOR);
  StretchDIBits(Dc, 0, 0, ScreenW * Scale, ScreenH * Scale,
    0, 0, ScreenW, ScreenH, @Pixels32, Bmi, DIB_RGB_COLORS, SRCCOPY);
end;

procedure Present;
var
  Dc: HDC;
begin
  if WindowHandle = 0 then
    Exit;
  Dc := GetDC(WindowHandle);
  if Dc <> 0 then
  begin
    PresentToDc(Dc);
    ReleaseDC(WindowHandle, Dc);
  end;
end;

{ ------------------------------------------------------------------ }
{ Frame dumps (palette-aware PPM)                                    }
{ ------------------------------------------------------------------ }

procedure SaveFrameWithPalette(const Path: string);
var
  Stream: TFileStream;
  Header: AnsiString;
  I: Integer;
  C: DWORD;
  Rgb: array[0..2] of Byte;
begin
  Stream := TFileStream.Create(Path, fmCreate);
  try
    Header := 'P6' + #10 + IntToStr(ScreenW) + ' ' + IntToStr(ScreenH) +
      #10 + '255' + #10;
    Stream.WriteBuffer(Header[1], Length(Header));
    for I := 0 to High(FrameBuffer.Pixels) do
    begin
      C := PaletteRgb[FrameBuffer.Pixels[I] and $03];
      Rgb[0] := (C shr 16) and $FF;
      Rgb[1] := (C shr 8) and $FF;
      Rgb[2] := C and $FF;
      Stream.WriteBuffer(Rgb[0], SizeOf(Rgb));
    end;
  finally
    Stream.Free;
  end;
end;

procedure DumpFrame;
begin
  if DumpDir = '' then
    Exit;
  ForceDirectories(DumpDir);
  SaveFrameWithPalette(IncludeTrailingPathDelimiter(DumpDir) +
    Format('frame-%.4d.ppm', [DumpCounter]));
  Inc(DumpCounter);
end;

{ ------------------------------------------------------------------ }
{ Window + message pump                                              }
{ ------------------------------------------------------------------ }

function WndProc(Wnd: HWND; Msg: UINT; WParam: WPARAM;
  LParam: LPARAM): LRESULT; stdcall;
var
  Ps: PAINTSTRUCT;
  Dc: HDC;
  Ch: Char;
begin
  case Msg of
    WM_PAINT:
      begin
        Dc := BeginPaint(Wnd, @Ps);
        PresentToDc(Dc);
        EndPaint(Wnd, @Ps);
        Exit(0);
      end;
    WM_ERASEBKGND:
      Exit(1);
    WM_KEYDOWN:
      case WParam of
        VK_UP:
          begin
            EnqueueKey(#27); EnqueueKey(#72); Exit(0);
          end;
        VK_DOWN:
          begin
            EnqueueKey(#27); EnqueueKey(#80); Exit(0);
          end;
        VK_LEFT:
          begin
            EnqueueKey(#27); EnqueueKey(#75); Exit(0);
          end;
        VK_RIGHT:
          begin
            EnqueueKey(#27); EnqueueKey(#77); Exit(0);
          end;
        VK_F10:
          begin
            EnqueueKey(#27); EnqueueKey(#68); Exit(0);
          end;
        VK_ESCAPE:
          begin
            EnqueueKey(#27); Exit(0);
          end;
      end;
    WM_SYSKEYDOWN:
      { F10 arrives as a system key (menu activation); swallow it. }
      if WParam = VK_F10 then
      begin
        EnqueueKey(#27); EnqueueKey(#68); Exit(0);
      end;
    WM_CHAR:
      begin
        { Hebrew keyboard layout: the window class is ANSI, so on a
          Hebrew-codepage system WM_CHAR delivers Windows-1255 bytes.
          Alef..tav are contiguous at $E0..$FA in the same alphabet
          order (finals interleaved) as CP862 $80..$9A, so a constant
          offset maps them.  The game needs this for InputYesNo
          (kaf/lamed) and high-score name entry.  Unicode Hebrew
          ($05D0..$05EA) is mapped too in case the window class is
          ever registered wide. }
        if (WParam >= $E0) and (WParam <= $FA) then
          EnqueueKey(Char($80 + WParam - $E0))
        else if (WParam >= $05D0) and (WParam <= $05EA) then
          EnqueueKey(Char($80 + WParam - $05D0))
        else
        begin
          Ch := Char(WParam and $FF);
          { Esc already enqueued from WM_KEYDOWN -- skip its WM_CHAR. }
          if (Ch <> #27) and
            ((Ch = #8) or (Ch = #13) or ((Ch >= #32) and (Ch <= #126))) then
            EnqueueKey(Ch);
        end;
        Exit(0);
      end;
    WM_CLOSE, WM_DESTROY:
      Halt(0);
  end;
  Result := DefWindowProc(Wnd, Msg, WParam, LParam);
end;

procedure CreateGameWindow;
var
  Wc: WNDCLASS;
  R: TRect;
  Style: DWORD;
  ShowCmd: Integer;
begin
  if WindowHandle <> 0 then
    Exit;

  if not WindowClassRegistered then
  begin
    FillChar(Wc, SizeOf(Wc), 0);
    Wc.style := CS_HREDRAW or CS_VREDRAW;
    Wc.lpfnWndProc := @WndProc;
    Wc.hInstance := HInstance;
    Wc.hCursor := LoadCursor(0, IDC_ARROW);
    Wc.hbrBackground := 0;
    Wc.lpszClassName := WindowClassName;
    Windows.RegisterClass(@Wc);
    WindowClassRegistered := True;
  end;

  Style := WS_CAPTION or WS_SYSMENU or WS_MINIMIZEBOX;
  R := Classes.Rect(0, 0, ScreenW * Scale, ScreenH * Scale);
  AdjustWindowRect(@R, Style, False);
  WindowHandle := CreateWindowEx(0, WindowClassName, WindowTitle, Style,
    CW_USEDEFAULT, CW_USEDEFAULT, R.Right - R.Left, R.Bottom - R.Top,
    0, 0, HInstance, nil);

  if ScriptMode then
    ShowCmd := SW_SHOWNOACTIVATE
  else
    ShowCmd := SW_SHOW;
  ShowWindow(WindowHandle, ShowCmd);
  UpdateWindow(WindowHandle);
end;

procedure PumpMessages;
var
  M: Windows.MSG;
begin
  while PeekMessage(@M, 0, 0, 0, PM_REMOVE) do
  begin
    TranslateMessage(M);
    DispatchMessage(M);
  end;
end;

{ ------------------------------------------------------------------ }
{ Output text-device driver (TP3 KBD/CRT style)                      }
{ ------------------------------------------------------------------ }

procedure DrawGlyphAtCursor(Code: Byte);
var
  PixelX, PixelY: Integer;
  Row, Bit: Integer;
  Bits: Byte;
  DestX, DestY: Integer;
begin
  PixelX := (Grid.CursorX - 1) * 8;
  PixelY := (Grid.CursorY - 1) * 8;
  for Row := 0 to 7 do
  begin
    Bits := Grid.Font[Code, Row];
    for Bit := 0 to 7 do
    begin
      DestX := PixelX + Bit;
      DestY := PixelY + Row;
      if (DestX < 0) or (DestX >= ScreenW) or (DestY < 0) or
        (DestY >= ScreenH) then
        Continue;
      if (Bits and ($80 shr Bit)) <> 0 then
        FrameBuffer.Pixels[DestY * ScreenW + DestX] := Grid.Color
      else
        FrameBuffer.Pixels[DestY * ScreenW + DestX] := Grid.Background;
    end;
  end;
end;

procedure WriteOutputByte(B: Byte);
begin
  EnsureInitialized;
  case B of
    7: { bell -- TP3 CON output beeps and prints nothing }
      if not ScriptMode then
        Windows.Beep(800, 100);
    8: { backspace }
      if Grid.CursorX > 1 then
        Dec(Grid.CursorX);
    10: { line feed }
      if Grid.CursorY < TextRows then
        Inc(Grid.CursorY);
    13: { carriage return }
      Grid.CursorX := 1;
  else
    { Render the raw byte as a CP862 glyph index -- no reordering. }
    DrawGlyphAtCursor(B);
    Inc(Grid.CursorX);
    if Grid.CursorX > TextCols then
    begin
      Grid.CursorX := 1;
      if Grid.CursorY < TextRows then
        Inc(Grid.CursorY);
    end;
  end;
end;

procedure OutputDevWrite(var F: TextRec);
var
  I: Integer;
begin
  for I := 0 to F.BufPos - 1 do
    WriteOutputByte(Byte(F.BufPtr^[I]));
  F.BufPos := 0;
end;

procedure OutputDevOpen(var F: TextRec);
begin
  F.Mode := fmOutput;
  F.BufPos := 0;
  F.BufEnd := 0;
end;

procedure OutputDevClose(var F: TextRec);
begin
end;

procedure InstallOutputDriver;
begin
  if OutputInstalled then
    Exit;
  with TextRec(Output) do
  begin
    Mode := fmOutput;
    BufPtr := @Buffer;
    BufSize := SizeOf(Buffer);
    BufPos := 0;
    BufEnd := 0;
    OpenFunc := @OutputDevOpen;
    InOutFunc := @OutputDevWrite;
    FlushFunc := @OutputDevWrite;
    CloseFunc := @OutputDevClose;
  end;
  OutputInstalled := True;
end;

{ ------------------------------------------------------------------ }
{ Configuration / assets                                             }
{ ------------------------------------------------------------------ }

function FindFontPath: string;
const
  Relative = 'socher1' + DirectorySeparator + 'FONTHE8.COM';
var
  Base: string;
  Up: string;
  I: Integer;
begin
  Result := SysUtils.GetEnvironmentVariable('SOCHER_FONT');
  if (Result <> '') and FileExists(Result) then
    Exit;
  { The single-exe build extracts all assets (font included) into the cwd }
  if FileExists('FONTHE8.COM') then
    Exit('FONTHE8.COM');
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
  raise Exception.Create('PlatformWin32: cannot find socher1\FONTHE8.COM ' +
    '(set SOCHER_FONT or run from the repository root)');
end;

procedure ReadEnvConfig;
var
  S: string;
  V: Integer;
begin
  S := SysUtils.GetEnvironmentVariable('SOCHER_SCALE');
  if (S <> '') and TryStrToInt(S, V) and (V >= 1) and (V <= 10) then
    Scale := V;

  DumpDir := SysUtils.GetEnvironmentVariable('SOCHER_DUMP_DIR');

  S := SysUtils.GetEnvironmentVariable('SOCHER_KEYS');
  if S <> '' then
  begin
    if not FileExists(S) then
    begin
      { Fail loudly: a typo'd script path would otherwise run an empty
        script (boot, dump one frame, exit 0) and look like a pass. }
      WriteLn(ErrOutput, 'PlatformWin32: SOCHER_KEYS file not found: ', S);
      Halt(2);
    end;
    ScriptMode := True;
    ScriptTokens := TStringList.Create;
    ScriptTokens.LoadFromFile(S);
  end;
end;

{ ------------------------------------------------------------------ }
{ Platform contract: graphics                                        }
{ ------------------------------------------------------------------ }

procedure PortGraphColorMode;
begin
  InitFrameBuffer(FrameBuffer, ScreenW, ScreenH, 0);
  if not FontLoaded then
  begin
    InitTextGrid(Grid, FindFontPath);
    FontLoaded := True;
  end
  else
  begin
    Grid.CursorX := 1;
    Grid.CursorY := 1;
    Grid.Color := 3;
    Grid.Background := 0;
  end;
  ColorTable[0] := 0;
  ColorTable[1] := 1;
  ColorTable[2] := 2;
  ColorTable[3] := 3;
  CreateGameWindow;
  InstallOutputDriver;
  Initialized := True;
  Present;
end;

{ TP3's GraphBackground only loads the background palette register
  (index 0); it does not clear video memory.  This game calls it solely
  right after GraphColorMode (which has just cleared the framebuffer to
  index 0), so also clearing here is equivalent -- and keeps the screen
  consistent if a future caller breaks that pattern. }
procedure PortGraphBackground(Color: Integer);
var
  I: Integer;
begin
  EnsureInitialized;
  PaletteRgb[0] := CgaColors[Color and $0F];
  for I := 0 to High(FrameBuffer.Pixels) do
    FrameBuffer.Pixels[I] := 0;
end;

procedure PortPalette(N: Integer);
var
  I: Integer;
begin
  EnsureInitialized;
  for I := 1 to 3 do
    PaletteRgb[I] := CgaPalettes[N and $03, I];
end;

procedure PortColorTable(C1, C2, C3, C4: Integer);
begin
  ColorTable[0] := C1 and $03;
  ColorTable[1] := C2 and $03;
  ColorTable[2] := C3 and $03;
  ColorTable[3] := C4 and $03;
end;

procedure PortPutPic(var Buffer; X, Y: Integer);
var
  Pic: TPicture;
  I: Integer;
begin
  EnsureInitialized;
  Pic := DecodePictureBuffer(Buffer);
  for I := 0 to High(Pic.Pixels) do
    Pic.Pixels[I] := ColorTable[Pic.Pixels[I] and $03];
  PutPicture(FrameBuffer, Pic, X, Y);
end;

procedure PortGetPic(var Buffer; X1, Y1, X2, Y2: Integer);
var
  Pic: TPicture;
begin
  EnsureInitialized;
  Pic := GetPicture(FrameBuffer, X1, Y1, X2, Y2);
  EncodePictureBuffer(Pic, Buffer);
end;

{ ------------------------------------------------------------------ }
{ Platform contract: text grid                                       }
{ ------------------------------------------------------------------ }

procedure PortTextColor(Color: Integer);
begin
  EnsureInitialized;
  Grid.Color := Color and $03;
end;

procedure PortGotoXY(X, Y: Integer);
begin
  EnsureInitialized;
  { CGA BIOS graphics-mode addressing: a column past the 40-column grid
    wraps around within the SAME row - the game calls GotoXY(73,1) /
    GotoXY(53,16) / GotoXY(45,16) expecting columns 33 / 13 / 5
    (TextGrid.GotoXY performs the wrap). }
  if X < 1 then X := 1;
  if Y < 1 then Y := 1;
  if Y > TextRows then Y := TextRows;
  TextGrid.GotoXY(Grid, X, Y);
end;

function PortWhereX: Integer;
begin
  EnsureInitialized;
  Result := Grid.CursorX;
end;

function PortWhereY: Integer;
begin
  EnsureInitialized;
  Result := Grid.CursorY;
end;

{ ------------------------------------------------------------------ }
{ Platform contract: keyboard                                        }
{ ------------------------------------------------------------------ }

procedure FeedScriptUntilKeyAvailable;
var
  Token, U: string;
begin
  while KeyCount = 0 do
  begin
    PumpMessages;
    if (ScriptTokens = nil) or (ScriptIndex >= ScriptTokens.Count) then
    begin
      Present;
      Halt(0);
    end;
    Token := Trim(ScriptTokens[ScriptIndex]);
    Inc(ScriptIndex);
    Sleep(30); { let frames render between scripted keys }
    Present;
    if Token = '' then
      Continue;
    U := UpperCase(Token);
    if U = 'WAIT' then
      DumpFrame
    else if U = 'ENTER' then
      EnqueueKey(#13)
    else if U = 'ESC' then
      EnqueueKey(#27)
    else if U = 'UP' then
    begin
      EnqueueKey(#27); EnqueueKey(#72);
    end
    else if U = 'DOWN' then
    begin
      EnqueueKey(#27); EnqueueKey(#80);
    end
    else if U = 'LEFT' then
    begin
      EnqueueKey(#27); EnqueueKey(#75);
    end
    else if U = 'RIGHT' then
    begin
      EnqueueKey(#27); EnqueueKey(#77);
    end
    else if U = 'F10' then
    begin
      EnqueueKey(#27); EnqueueKey(#68);
    end
    else if U = 'SPACE' then
      EnqueueKey(' ')
    else if U = 'BACKSPACE' then
      EnqueueKey(#8)
    else
      EnqueueKey(Token[1]);
  end;
end;

function PortKeyPressed: Boolean;
begin
  EnsureInitialized;
  PumpMessages;
  Result := KeyCount > 0;
end;

function PortReadKey: Char;
var
  M: Windows.MSG;
begin
  EnsureInitialized;
  Present;
  DumpFrame; { no-op unless SOCHER_DUMP_DIR is set }

  if ScriptMode then
  begin
    FeedScriptUntilKeyAvailable;
    Exit(DequeueKey);
  end;

  while KeyCount = 0 do
  begin
    { GetMessage returns -1 on error; as a LongBool that is True, which
      would dispatch a garbage MSG forever.  Treat <= 0 (quit or error)
      as end-of-program. }
    if Integer(GetMessage(@M, 0, 0, 0)) <= 0 then
      Halt(0);
    TranslateMessage(M);
    DispatchMessage(M);
  end;
  Result := DequeueKey;
end;

{ ------------------------------------------------------------------ }
{ Platform contract: sound + delay                                   }
{ ------------------------------------------------------------------ }

procedure PortSound(Frequency: Integer);
begin
  ActiveFrequency := Frequency;
end;

procedure PortNoSound;
begin
  ActiveFrequency := 0;
end;

procedure PortDelay(Ms: Integer);
var
  Start: QWord;
  Elapsed: Integer;
  Remaining: Integer;
  Freq: Integer;
begin
  if Initialized then
    Present;
  PumpMessages;
  if Ms <= 0 then
    Exit;

  if ActiveFrequency > 0 then
  begin
    Freq := ActiveFrequency;
    if Freq < 37 then
      Freq := 37;
    if Freq > 32767 then
      Freq := 32767;
    { Beep blocks the message pump for Ms, but the game's longest sounded
      delay is 50 ms (the InputYesNo error beep), so the window never
      stalls noticeably. }
    Windows.Beep(Freq, Ms);
    PumpMessages;
    Exit;
  end;

  Start := GetTickCount64;
  repeat
    PumpMessages;
    Elapsed := Integer(GetTickCount64 - Start);
    Remaining := Ms - Elapsed;
    if Remaining <= 0 then
      Break;
    if Remaining > 10 then
      Sleep(10)
    else
      Sleep(Remaining);
  until False;
end;

initialization
  ReadEnvConfig;

finalization
  ScriptTokens.Free;

end.
