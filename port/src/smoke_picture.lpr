program SmokePicture;

{$mode objfpc}{$H+}

uses
  Picture;

var
  FrameBuffer: TFrameBuffer;
  MainScreen: TPicture;
  MapWindow: TPicture;

begin
  InitFrameBuffer(FrameBuffer, 320, 200, 1);
  MainScreen := LoadPicture('socher1/MAINSCRN.SCR');
  MapWindow := LoadPicture('socher1/MAP.WIN');
  PutPicture(FrameBuffer, MainScreen, 0, 199);
  PutPicture(FrameBuffer, MapWindow, 0, 142);
  Writeln('Rendered ', FrameBuffer.Width, 'x', FrameBuffer.Height,
    ' framebuffer with ', Length(FrameBuffer.Pixels), ' pixels.');
end.
