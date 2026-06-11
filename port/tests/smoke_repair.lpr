program SmokeRepair;
{ Visual smoke test: draws the real REPAIR.WIN window with the damage value
  and the '= = הכל' quick-key hint exactly where ShowRepairWindow puts them.
  Must be reencoded to CP862 before compiling (the Hebrew literal below). }
{$MODE TP}

uses
  PlatformWin32;

var
  Buf: array[1..8832] of byte;
  F: file;

begin
  PortGraphColorMode;
  PortTextColor(3);
  PortPalette(0);
  PortGraphBackground(1);
  Assign(F, 'repair.win');
  Reset(F);
  BlockRead(F, Buf, 8832 div 128);
  Close(F);
  PortPutPic(Buf, 0, 142);
  PortGotoXY(21, 17);
  Write(1060:4);
  PortGotoXY(13, 18);
  Write('לכה = =');
  PortGotoXY(5, 17);
  PortReadKey;
end.
