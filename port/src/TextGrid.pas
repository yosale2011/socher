unit TextGrid;

{$mode objfpc}{$H+}

interface

uses
  Picture;

type
  TFont8x8 = array[0..255, 0..7] of Byte;

  TTextGrid = record
    CursorX: Integer;
    CursorY: Integer;
    Color: TPixel;
    Background: TPixel;
    Font: TFont8x8;
  end;

procedure InitTextGrid(var Grid: TTextGrid; const FontPath: string);
procedure TextColor(var Grid: TTextGrid; Color: TPixel);
procedure GotoXY(var Grid: TTextGrid; X, Y: Integer);
function WhereX(const Grid: TTextGrid): Integer;
function WhereY(const Grid: TTextGrid): Integer;
procedure WriteText(var Grid: TTextGrid; var FrameBuffer: TFrameBuffer;
  const Text: string);
procedure WriteTextAt(var Grid: TTextGrid; var FrameBuffer: TFrameBuffer;
  X, Y: Integer; const Text: string);

implementation

uses
  Classes, SysUtils;

const
  { FONTHE8.COM is the original DOS font TSR.  Its 8x8 glyph table (256
    glyphs x 8 bytes, MSB = leftmost pixel) sits at this fixed byte
    offset inside the .COM image; bytes 0..603 are loader code. }
  Fonthe8Offset = 604;
  CellWidth = 8;
  CellHeight = 8;
  GridColumns = 40;
  GridRows = 25;

procedure LoadFont8x8(var Font: TFont8x8; const FontPath: string);
var
  Stream: TFileStream;
  Code: Integer;
begin
  Stream := TFileStream.Create(FontPath, fmOpenRead or fmShareDenyWrite);
  try
    if Stream.Size < Fonthe8Offset + SizeOf(Font) then
      raise EReadError.CreateFmt('%s is too small for FONTHE8 data', [FontPath]);
    Stream.Position := Fonthe8Offset;
    for Code := 0 to 255 do
      Stream.ReadBuffer(Font[Code, 0], 8);
  finally
    Stream.Free;
  end;
end;

procedure InitTextGrid(var Grid: TTextGrid; const FontPath: string);
begin
  Grid.CursorX := 1;
  Grid.CursorY := 1;
  Grid.Color := 3;
  Grid.Background := 0;
  LoadFont8x8(Grid.Font, FontPath);
end;

procedure TextColor(var Grid: TTextGrid; Color: TPixel);
begin
  Grid.Color := Color and $03;
end;

procedure GotoXY(var Grid: TTextGrid; X, Y: Integer);
begin
  { CGA BIOS graphics-mode cursor addressing: a column past the 40-column
    grid wraps around within the SAME row.  The original game relies on
    this - it calls GotoXY(73,1), GotoXY(53,16) and GotoXY(45,16)
    expecting columns 33, 13 and 5 (verified against the DOS reference
    screenshot: the HUD country name sits at row 1, columns 33-38). }
  if X > GridColumns then
    X := ((X - 1) mod GridColumns) + 1;
  Grid.CursorX := X;
  Grid.CursorY := Y;
end;

function WhereX(const Grid: TTextGrid): Integer;
begin
  Result := Grid.CursorX;
end;

function WhereY(const Grid: TTextGrid): Integer;
begin
  Result := Grid.CursorY;
end;

function HebrewCodePointToCp862(CodePoint: Integer; var OutCode: Byte): Boolean;
begin
  Result := True;
  case CodePoint of
    $05D0: OutCode := $80; { Alef }
    $05D1: OutCode := $81;
    $05D2: OutCode := $82;
    $05D3: OutCode := $83;
    $05D4: OutCode := $84;
    $05D5: OutCode := $85;
    $05D6: OutCode := $86;
    $05D7: OutCode := $87;
    $05D8: OutCode := $88;
    $05D9: OutCode := $89;
    $05DA: OutCode := $8A; { final kaf }
    $05DB: OutCode := $8B;
    $05DC: OutCode := $8C;
    $05DD: OutCode := $8D; { final mem }
    $05DE: OutCode := $8E;
    $05DF: OutCode := $8F; { final nun }
    $05E0: OutCode := $90;
    $05E1: OutCode := $91;
    $05E2: OutCode := $92;
    $05E3: OutCode := $93; { final pe }
    $05E4: OutCode := $94;
    $05E5: OutCode := $95; { final tsadi }
    $05E6: OutCode := $96;
    $05E7: OutCode := $97;
    $05E8: OutCode := $98;
    $05E9: OutCode := $99;
    $05EA: OutCode := $9A;
  else
    Result := False;
  end;
end;

function NextCp862Code(const Text: string; var Index: Integer): Byte;
var
  B1: Byte;
  B2: Byte;
  CodePoint: Integer;
begin
  if Index > Length(Text) then
    Exit(0);

  B1 := Ord(Text[Index]);
  Inc(Index);
  if B1 < $80 then
    Exit(B1);

  { Convenience: accept UTF-8 encoded Hebrew letters (lead byte $D7) from
    repo-source string literals and map them to CP862. }
  if (B1 = $D7) and (Index <= Length(Text)) then
  begin
    B2 := Ord(Text[Index]);
    if (B2 and $C0) = $80 then
    begin
      CodePoint := ((B1 and $1F) shl 6) or (B2 and $3F);
      if HebrewCodePointToCp862(CodePoint, Result) then
      begin
        Inc(Index);
        Exit;
      end;
    end;
  end;

  { Otherwise the byte is already CP862: render it directly as a glyph
    index, in stored order, without any bidi reordering. }
  Result := B1;
end;

procedure DrawGlyph(var FrameBuffer: TFrameBuffer; const Grid: TTextGrid;
  CellX, CellY: Integer; Code: Byte);
var
  PixelX: Integer;
  PixelY: Integer;
  Row: Integer;
  Bit: Integer;
  Bits: Byte;
  DestX: Integer;
  DestY: Integer;
begin
  PixelX := (CellX - 1) * CellWidth;
  PixelY := (CellY - 1) * CellHeight;

  for Row := 0 to 7 do
  begin
    Bits := Grid.Font[Code, Row];
    for Bit := 0 to 7 do
    begin
      DestX := PixelX + Bit;
      DestY := PixelY + Row;
      if (DestX < 0) or (DestX >= FrameBuffer.Width) or
        (DestY < 0) or (DestY >= FrameBuffer.Height) then
        Continue;

      if (Bits and ($80 shr Bit)) <> 0 then
        FrameBuffer.Pixels[DestY * FrameBuffer.Width + DestX] := Grid.Color
      else
        FrameBuffer.Pixels[DestY * FrameBuffer.Width + DestX] := Grid.Background;
    end;
  end;
end;

procedure WriteText(var Grid: TTextGrid; var FrameBuffer: TFrameBuffer;
  const Text: string);
var
  Index: Integer;
  Code: Byte;
begin
  Index := 1;
  while Index <= Length(Text) do
  begin
    Code := NextCp862Code(Text, Index);
    DrawGlyph(FrameBuffer, Grid, Grid.CursorX, Grid.CursorY, Code);
    Inc(Grid.CursorX);
    if Grid.CursorX > GridColumns then
    begin
      Grid.CursorX := 1;
      Inc(Grid.CursorY);
      if Grid.CursorY > GridRows then
        Grid.CursorY := GridRows;
    end;
  end;
end;

procedure WriteTextAt(var Grid: TTextGrid; var FrameBuffer: TFrameBuffer;
  X, Y: Integer; const Text: string);
begin
  GotoXY(Grid, X, Y);
  WriteText(Grid, FrameBuffer, Text);
end;

end.
