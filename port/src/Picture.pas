unit Picture;

{$mode objfpc}{$H+}

interface

type
  TPixel = Byte;
  TPixelArray = array of TPixel;

  TPicture = record
    Width: Integer;
    Height: Integer;
    Pixels: TPixelArray;
  end;

  TFrameBuffer = record
    Width: Integer;
    Height: Integer;
    Pixels: TPixelArray;
  end;

procedure InitFrameBuffer(var FrameBuffer: TFrameBuffer; Width, Height: Integer;
  Color: TPixel);
function LoadPicture(const Path: string): TPicture;
function DecodePictureBuffer(var Buffer): TPicture;
procedure EncodePictureBuffer(const Picture: TPicture; var Buffer);
procedure PutPicture(var FrameBuffer: TFrameBuffer; const Picture: TPicture;
  X, BottomY: Integer);
function GetPicture(const FrameBuffer: TFrameBuffer; X1, Y1, X2, Y2: Integer): TPicture;
procedure SaveFrameBufferAsPpm(const FrameBuffer: TFrameBuffer; const Path: string);

implementation

uses
  Classes, SysUtils;

type
  TByteArray = array[0..MaxInt div 2] of Byte;
  PByteArray = ^TByteArray;

function ReadLeWord(const Data: TBytes; Offset: Integer): Integer;
begin
  Result := Data[Offset] or (Data[Offset + 1] shl 8);
end;

function DecodePictureBytes(const Data: TBytes; const Name: string): TPicture; forward;

function LoadFileBytes(const Path: string): TBytes;
var
  Stream: TFileStream;
begin
  Stream := TFileStream.Create(Path, fmOpenRead or fmShareDenyWrite);
  try
    SetLength(Result, Stream.Size);
    if Stream.Size > 0 then
      Stream.ReadBuffer(Result[0], Stream.Size);
  finally
    Stream.Free;
  end;
end;

procedure InitFrameBuffer(var FrameBuffer: TFrameBuffer; Width, Height: Integer;
  Color: TPixel);
var
  I: Integer;
begin
  FrameBuffer.Width := Width;
  FrameBuffer.Height := Height;
  SetLength(FrameBuffer.Pixels, Width * Height);
  for I := 0 to High(FrameBuffer.Pixels) do
    FrameBuffer.Pixels[I] := Color;
end;

function LoadPicture(const Path: string): TPicture;
var
  Data: TBytes;
begin
  Data := LoadFileBytes(Path);
  Result := DecodePictureBytes(Data, Path);
end;

function DecodePictureBytes(const Data: TBytes; const Name: string): TPicture;
var
  Marker: Integer;
  BytesPerScanline: Integer;
  UsefulSize: Integer;
  SourceY: Integer;
  DestY: Integer;
  ByteX: Integer;
  BitPair: Integer;
  TargetX: Integer;
  Packed: Byte;
  Shift: Integer;
begin
  if Length(Data) < 6 then
    raise EReadError.CreateFmt('%s is too small to be a TP picture', [Name]);

  Marker := ReadLeWord(Data, 0);
  if Marker <> 2 then
    raise EReadError.CreateFmt('%s has unexpected TP picture marker %d',
      [Name, Marker]);

  Result.Width := ReadLeWord(Data, 2);
  Result.Height := ReadLeWord(Data, 4);
  BytesPerScanline := (Result.Width + 3) div 4;
  UsefulSize := 6 + BytesPerScanline * Result.Height;
  if Length(Data) < UsefulSize then
    raise EReadError.CreateFmt('%s is truncated for %dx%d picture data',
      [Name, Result.Width, Result.Height]);

  SetLength(Result.Pixels, Result.Width * Result.Height);

  for SourceY := 0 to Result.Height - 1 do
  begin
    DestY := Result.Height - 1 - SourceY;
    for ByteX := 0 to BytesPerScanline - 1 do
    begin
      Packed := Data[6 + SourceY * BytesPerScanline + ByteX];
      for BitPair := 0 to 3 do
      begin
        TargetX := ByteX * 4 + BitPair;
        if TargetX < Result.Width then
        begin
          Shift := 6 - BitPair * 2;
          Result.Pixels[DestY * Result.Width + TargetX] :=
            (Packed shr Shift) and $03;
        end;
      end;
    end;
  end;
end;

function DecodePictureBuffer(var Buffer): TPicture;
var
  Source: PByteArray;
  Data: TBytes;
  Width: Integer;
  Height: Integer;
  BytesPerScanline: Integer;
  UsefulSize: Integer;
begin
  Source := @Buffer;
  Width := Source^[2] or (Source^[3] shl 8);
  Height := Source^[4] or (Source^[5] shl 8);
  BytesPerScanline := (Width + 3) div 4;
  UsefulSize := 6 + BytesPerScanline * Height;
  SetLength(Data, UsefulSize);
  Move(Source^[0], Data[0], UsefulSize);
  Result := DecodePictureBytes(Data, 'memory buffer');
end;

procedure EncodePictureBuffer(const Picture: TPicture; var Buffer);
var
  Dest: PByteArray;
  BytesPerScanline: Integer;
  SourceY: Integer;
  StoredY: Integer;
  ByteX: Integer;
  BitPair: Integer;
  SourceX: Integer;
  Packed: Byte;
  Pixel: Byte;
begin
  Dest := @Buffer;
  BytesPerScanline := (Picture.Width + 3) div 4;
  Dest^[0] := 2;
  Dest^[1] := 0;
  Dest^[2] := Picture.Width and $FF;
  Dest^[3] := (Picture.Width shr 8) and $FF;
  Dest^[4] := Picture.Height and $FF;
  Dest^[5] := (Picture.Height shr 8) and $FF;

  for StoredY := 0 to Picture.Height - 1 do
  begin
    SourceY := Picture.Height - 1 - StoredY;
    for ByteX := 0 to BytesPerScanline - 1 do
    begin
      Packed := 0;
      for BitPair := 0 to 3 do
      begin
        SourceX := ByteX * 4 + BitPair;
        if SourceX < Picture.Width then
          Pixel := Picture.Pixels[SourceY * Picture.Width + SourceX] and $03
        else
          Pixel := 0;
        Packed := Packed or (Pixel shl (6 - BitPair * 2));
      end;
      Dest^[6 + StoredY * BytesPerScanline + ByteX] := Packed;
    end;
  end;
end;

procedure PutPicture(var FrameBuffer: TFrameBuffer; const Picture: TPicture;
  X, BottomY: Integer);
var
  TopY: Integer;
  SourceX: Integer;
  SourceY: Integer;
  DestX: Integer;
  DestY: Integer;
begin
  TopY := BottomY - Picture.Height + 1;
  for SourceY := 0 to Picture.Height - 1 do
  begin
    DestY := TopY + SourceY;
    if (DestY < 0) or (DestY >= FrameBuffer.Height) then
      Continue;

    for SourceX := 0 to Picture.Width - 1 do
    begin
      DestX := X + SourceX;
      if (DestX < 0) or (DestX >= FrameBuffer.Width) then
        Continue;

      FrameBuffer.Pixels[DestY * FrameBuffer.Width + DestX] :=
        Picture.Pixels[SourceY * Picture.Width + SourceX];
    end;
  end;
end;

function GetPicture(const FrameBuffer: TFrameBuffer; X1, Y1, X2, Y2: Integer): TPicture;
var
  X: Integer;
  Y: Integer;
begin
  Result.Width := X2 - X1 + 1;
  Result.Height := Y2 - Y1 + 1;
  SetLength(Result.Pixels, Result.Width * Result.Height);

  for Y := 0 to Result.Height - 1 do
    for X := 0 to Result.Width - 1 do
      if (X1 + X >= 0) and (X1 + X < FrameBuffer.Width) and
        (Y1 + Y >= 0) and (Y1 + Y < FrameBuffer.Height) then
        Result.Pixels[Y * Result.Width + X] :=
          FrameBuffer.Pixels[(Y1 + Y) * FrameBuffer.Width + X1 + X]
      else
        Result.Pixels[Y * Result.Width + X] := 0;
end;

procedure SaveFrameBufferAsPpm(const FrameBuffer: TFrameBuffer; const Path: string);
const
  Palette: array[0..3, 0..2] of Byte = (
    (0, 0, 0),
    (0, 170, 170),
    (170, 0, 170),
    (170, 170, 170)
  );
var
  Stream: TFileStream;
  Header: AnsiString;
  I: Integer;
  Pixel: Byte;
  Rgb: array[0..2] of Byte;
begin
  Stream := TFileStream.Create(Path, fmCreate);
  try
    Header := 'P6' + LineEnding + IntToStr(FrameBuffer.Width) + ' ' +
      IntToStr(FrameBuffer.Height) + LineEnding + '255' + LineEnding;
    Stream.WriteBuffer(Header[1], Length(Header));
    for I := 0 to High(FrameBuffer.Pixels) do
    begin
      Pixel := FrameBuffer.Pixels[I] and $03;
      Rgb[0] := Palette[Pixel, 0];
      Rgb[1] := Palette[Pixel, 1];
      Rgb[2] := Palette[Pixel, 2];
      Stream.WriteBuffer(Rgb[0], SizeOf(Rgb));
    end;
  finally
    Stream.Free;
  end;
end;

end.
