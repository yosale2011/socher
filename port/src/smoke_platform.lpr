program SmokePlatform;

{$mode objfpc}{$H+}

uses
  Classes, Platform;

type
  TScrBuffer = array[1..16128] of Byte;
  TWinBuffer = array[1..8832] of Byte;

var
  MainScreen: TScrBuffer;
  MapWindow: TWinBuffer;

procedure LoadRaw(const Path: string; var Buffer; Size: Integer);
var
  Stream: TFileStream;
begin
  Stream := TFileStream.Create(Path, fmOpenRead or fmShareDenyWrite);
  try
    Stream.ReadBuffer(Buffer, Size);
  finally
    Stream.Free;
  end;
end;

begin
  PortGraphColorMode;
  PortGraphBackground(1);
  LoadRaw('socher1/MAINSCRN.SCR', MainScreen, SizeOf(MainScreen));
  LoadRaw('socher1/MAP.WIN', MapWindow, SizeOf(MapWindow));
  PortPutPic(MainScreen, 0, 199);
  PortPutPic(MapWindow, 0, 142);
  PortGotoXY(32, 5);
  PortTextColor(3);
  PortWriteText('8:00');

  PortSaveFrameBufferAsPpm('port/smoke-platform.ppm');
  Writeln('Wrote port/smoke-platform.ppm');
end.
