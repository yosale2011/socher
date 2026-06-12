program ReproCorrupt;
var
  F: text;
  Version, IntVal: integer;
  RealVal: real;
  L: longint;
begin
  Assign(F, 'corrupt_test.dat');
  Rewrite(F);
  Writeln(F, 'garbage');
  Writeln(F, 'not a number');
  Close(F);
  Assign(F, 'corrupt_test.dat');
  {$I-} Reset(F); {$I+}
  if IOResult <> 0 then begin Writeln('no file'); Halt(1); end;
  {$I-}
  Readln(F, Version);
  Readln(F, IntVal);
  Readln(F, RealVal);
  Readln(F, L);
  if (IOResult = 0) and (Version = 1) then
    Writeln('valid')
  else
    Writeln('invalid - fell back ok');
  Close(F);
  if IOResult <> 0 then ;
  {$I+}
  Writeln('done, no crash');
end.
