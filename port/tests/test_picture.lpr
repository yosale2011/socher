program TestPicture;

{$mode objfpc}{$H+}

{ Pixel-layer regression test.

  1. Loads socher1/MAINSCRN.SCR with Picture.pas, blits it at (0,199) and
     saves port/bin/test-mainscrn.ppm. That image must match the trusted
     Python decoder output (port/tools/decode_pic.py) pixel for pixel.
  2. Asserts a handful of known pixel values (probed from the Python
     reference decoder) and exits with a non-zero code on any mismatch.
  3. Round-trips the picture through EncodePictureBuffer/DecodePictureBuffer.
  4. Renders text (ASCII + raw CP862 Hebrew bytes) through TextGrid onto a
     copy of the framebuffer and saves port/bin/test-text.ppm for eyeballing. }

uses
  SysUtils, Picture, TextGrid;

var
  Failures: Integer = 0;

procedure CheckPixel(const FrameBuffer: TFrameBuffer; X, Y: Integer;
  Expected: Byte);
var
  Actual: Byte;
begin
  Actual := FrameBuffer.Pixels[Y * FrameBuffer.Width + X];
  if Actual <> Expected then
  begin
    Writeln('FAIL pixel (', X, ',', Y, '): expected ', Expected,
      ' got ', Actual);
    Inc(Failures);
  end
  else
    Writeln('ok   pixel (', X, ',', Y, ') = ', Actual);
end;

procedure CheckTrue(Condition: Boolean; const What: string);
begin
  if not Condition then
  begin
    Writeln('FAIL ', What);
    Inc(Failures);
  end
  else
    Writeln('ok   ', What);
end;

var
  FrameBuffer: TFrameBuffer;
  TextFrame: TFrameBuffer;
  MainScreen: TPicture;
  RoundTrip: TPicture;
  Buffer: array of Byte;
  Grid: TTextGrid;
  I: Integer;
  RoundTripOk: Boolean;
  BaseDir: string;
begin
  BaseDir := ExtractFilePath(ParamStr(0)) + '..\..\';

  MainScreen := LoadPicture(BaseDir + 'socher1\MAINSCRN.SCR');
  CheckTrue(MainScreen.Width = 320, 'MAINSCRN width = 320');
  CheckTrue(MainScreen.Height = 200, 'MAINSCRN height = 200');

  InitFrameBuffer(FrameBuffer, 320, 200, 0);
  PutPicture(FrameBuffer, MainScreen, 0, 199);

  { Expected values probed from port/tools/decode_pic.py on MAINSCRN.SCR. }
  CheckPixel(FrameBuffer, 0, 0, 1);
  CheckPixel(FrameBuffer, 319, 0, 1);
  CheckPixel(FrameBuffer, 0, 199, 1);
  CheckPixel(FrameBuffer, 319, 199, 1);
  CheckPixel(FrameBuffer, 160, 100, 0);
  CheckPixel(FrameBuffer, 10, 5, 1);
  CheckPixel(FrameBuffer, 50, 150, 2);
  CheckPixel(FrameBuffer, 200, 20, 1);
  CheckPixel(FrameBuffer, 100, 180, 2);

  SaveFrameBufferAsPpm(FrameBuffer, BaseDir + 'port\bin\test-mainscrn.ppm');
  Writeln('wrote port\bin\test-mainscrn.ppm');

  { Encode/decode round trip through the TP buffer format. }
  SetLength(Buffer, 6 + ((MainScreen.Width + 3) div 4) * MainScreen.Height);
  EncodePictureBuffer(MainScreen, Buffer[0]);
  RoundTrip := DecodePictureBuffer(Buffer[0]);
  RoundTripOk := (RoundTrip.Width = MainScreen.Width) and
    (RoundTrip.Height = MainScreen.Height);
  if RoundTripOk then
    for I := 0 to High(MainScreen.Pixels) do
      if RoundTrip.Pixels[I] <> MainScreen.Pixels[I] then
      begin
        RoundTripOk := False;
        Break;
      end;
  CheckTrue(RoundTripOk, 'Encode/DecodePictureBuffer round trip');

  { GetPicture must read back exactly what PutPicture wrote. }
  RoundTrip := GetPicture(FrameBuffer, 0, 0, 319, 199);
  RoundTripOk := True;
  for I := 0 to High(MainScreen.Pixels) do
    if RoundTrip.Pixels[I] <> MainScreen.Pixels[I] then
    begin
      RoundTripOk := False;
      Break;
    end;
  CheckTrue(RoundTripOk, 'GetPicture matches blitted picture');

  { Text layer render for visual inspection: ASCII, digits, and a raw
    CP862 Hebrew byte string (stored order, must not be reordered).
    #$99#$8C#$85#$8D = shin, lamed, vav, mem-sofit drawn left-to-right
    exactly as stored. }
  TextFrame := FrameBuffer;
  SetLength(TextFrame.Pixels, Length(FrameBuffer.Pixels));
  for I := 0 to High(FrameBuffer.Pixels) do
    TextFrame.Pixels[I] := FrameBuffer.Pixels[I];

  InitTextGrid(Grid, BaseDir + 'socher1\FONTHE8.COM');
  TextColor(Grid, 3);
  WriteTextAt(Grid, TextFrame, 32, 5, '8:00');
  TextColor(Grid, 2);
  WriteTextAt(Grid, TextFrame, 2, 2, 'ABC abc 0123456789');
  TextColor(Grid, 1);
  WriteTextAt(Grid, TextFrame, 10, 12, #$99#$8C#$85#$8D);
  CheckTrue((WhereX(Grid) = 14) and (WhereY(Grid) = 12),
    'cursor advanced 4 cells for 4 CP862 bytes');

  SaveFrameBufferAsPpm(TextFrame, BaseDir + 'port\bin\test-text.ppm');
  Writeln('wrote port\bin\test-text.ppm');

  if Failures > 0 then
  begin
    Writeln(Failures, ' check(s) failed.');
    Halt(1);
  end;
  Writeln('All checks passed.');
end.
