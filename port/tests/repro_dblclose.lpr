program ReproDoubleClose;
type TRec = record A: array[1..27] of byte; end;
var F: file of TRec;
begin
  Assign(F, 'no_such_file.bin');
  {$I-} Reset(F); {$I+}
  if IOResult <> 0 then
  begin
    Rewrite(F);
    Close(F);
  end;
  Close(F);
  Writeln('no crash');
end.
