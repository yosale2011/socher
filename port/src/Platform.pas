unit Platform;

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

procedure PortDelay(Ms: Integer);
procedure PortSound(Frequency: Integer);
procedure PortNoSound;
procedure PortWriteText(const Text: string);
procedure PortSaveFrameBufferAsPpm(const Path: string);

implementation

uses
  SysUtils, Picture, TextGrid;

var
  GlobalFrameBuffer: TFrameBuffer;
  GlobalTextGrid: TTextGrid;
  IsInitialized: Boolean = False;
  CurrentColorTable: array[0..3] of TPixel = (0, 1, 2, 3);

procedure EnsureInitialized;
begin
  if not IsInitialized then
    PortGraphColorMode;
end;

procedure PortGraphColorMode;
begin
  InitFrameBuffer(GlobalFrameBuffer, 320, 200, 0);
  InitTextGrid(GlobalTextGrid, 'socher1/FONTHE8.COM');
  IsInitialized := True;
end;

procedure PortGraphBackground(Color: Integer);
var
  I: Integer;
begin
  EnsureInitialized;
  for I := 0 to High(GlobalFrameBuffer.Pixels) do
    GlobalFrameBuffer.Pixels[I] := Color and $03;
end;

procedure PortPalette(N: Integer);
begin
  EnsureInitialized;
end;

procedure PortColorTable(C1, C2, C3, C4: Integer);
begin
  CurrentColorTable[0] := C1 and $03;
  CurrentColorTable[1] := C2 and $03;
  CurrentColorTable[2] := C3 and $03;
  CurrentColorTable[3] := C4 and $03;
end;

procedure PortPutPic(var Buffer; X, Y: Integer);
var
  Pic: TPicture;
  I: Integer;
begin
  EnsureInitialized;
  Pic := DecodePictureBuffer(Buffer);
  for I := 0 to High(Pic.Pixels) do
    Pic.Pixels[I] := CurrentColorTable[Pic.Pixels[I] and $03];
  PutPicture(GlobalFrameBuffer, Pic, X, Y);
end;

procedure PortGetPic(var Buffer; X1, Y1, X2, Y2: Integer);
var
  Pic: TPicture;
begin
  EnsureInitialized;
  Pic := GetPicture(GlobalFrameBuffer, X1, Y1, X2, Y2);
  EncodePictureBuffer(Pic, Buffer);
end;

procedure PortTextColor(Color: Integer);
begin
  EnsureInitialized;
  TextGrid.TextColor(GlobalTextGrid, Color and $03);
end;

procedure PortGotoXY(X, Y: Integer);
begin
  EnsureInitialized;
  TextGrid.GotoXY(GlobalTextGrid, X, Y);
end;

function PortWhereX: Integer;
begin
  EnsureInitialized;
  PortWhereX := TextGrid.WhereX(GlobalTextGrid);
end;

function PortWhereY: Integer;
begin
  EnsureInitialized;
  PortWhereY := TextGrid.WhereY(GlobalTextGrid);
end;

function PortKeyPressed: Boolean;
begin
  PortKeyPressed := False;
end;

function PortReadKey: Char;
var
  Ch: Char;
begin
  Read(Ch);
  PortReadKey := Ch;
end;

procedure PortDelay(Ms: Integer);
begin
  Sleep(Ms);
end;

procedure PortSound(Frequency: Integer);
begin
end;

procedure PortNoSound;
begin
end;

procedure PortWriteText(const Text: string);
begin
  EnsureInitialized;
  TextGrid.WriteText(GlobalTextGrid, GlobalFrameBuffer, Text);
end;

procedure PortSaveFrameBufferAsPpm(const Path: string);
begin
  EnsureInitialized;
  Picture.SaveFrameBufferAsPpm(GlobalFrameBuffer, Path);
end;

end.
