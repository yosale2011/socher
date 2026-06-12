program ReproFixed;
type TRec = record A: array[1..27] of byte; end;
var F: file of TRec;
begin
  Assign(F, 'no_such_file2.bin');
  {$I-} Reset(F); {$I+}
  if IOResult <> 0 then
    Rewrite(F);
  Close(F);
  Writeln('no crash');
end.
