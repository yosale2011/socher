{ Tests for Tp3Compat: Real48 size/layout, DoubleToTp3GameReal round
  trips, and that THighScoreEntry (as declared by the game) is 27 bytes
  so the original winners.win is read correctly. }
program test_tp3compat;

{$MODE TP}{$R-}{$Q-}

uses
  Tp3Compat;

type
  TPlayerName = string[20];
  THighScoreEntry = record
    PlayerName: TPlayerName;
    Score: Tp3GameReal;
  end;

var
  Failures: Integer;

procedure Check(Cond: Boolean; Msg: string);
begin
  if not Cond then
  begin
    Writeln('FAIL: ', Msg);
    Failures := Failures + 1;
  end;
end;

procedure CheckRoundTrip(V: Double);
var
  R: Tp3GameReal;
  Back: Double;
begin
  R := DoubleToTp3GameReal(V);
  Back := R; { Real48 -> double is native in FPC }
  if Back <> V then
  begin
    Writeln('FAIL: round trip ', V, ' -> ', Back);
    Failures := Failures + 1;
  end;
end;

var
  R: Tp3GameReal;
  B: array[0..5] of Byte absolute R;
begin
  Failures := 0;

  Check(SizeOf(Tp3GameReal) = 6, 'SizeOf(Tp3GameReal) = 6');
  Check(SizeOf(THighScoreEntry) = 27, 'SizeOf(THighScoreEntry) = 27');

  { 1.0 in TP real48: exponent byte $81, mantissa 0, sign 0 }
  R := DoubleToTp3GameReal(1.0);
  Check((B[0] = $81) and (B[1] = 0) and (B[2] = 0) and (B[3] = 0)
        and (B[4] = 0) and (B[5] = 0), '1.0 encodes as 81 00 00 00 00 00');

  { -1.0: same but sign bit set }
  R := DoubleToTp3GameReal(-1.0);
  Check((B[0] = $81) and (B[5] = $80), '-1.0 encodes as 81 .. 80');

  { 0.0: all zero }
  R := DoubleToTp3GameReal(0.0);
  Check((B[0] = 0) and (B[1] = 0) and (B[2] = 0) and (B[3] = 0)
        and (B[4] = 0) and (B[5] = 0), '0.0 encodes as all zeroes');

  { values exactly representable in 39 mantissa bits round trip }
  CheckRoundTrip(0.0);
  CheckRoundTrip(1.0);
  CheckRoundTrip(-1.0);
  CheckRoundTrip(2000.0);
  CheckRoundTrip(123456.0);
  CheckRoundTrip(99999999.0);
  CheckRoundTrip(0.5);
  CheckRoundTrip(-345678.25);

  { Tp3Random stays in range }
  Tp3Randomize;
  Check((Tp3Random(6) >= 0) and (Tp3Random(6) < 6), 'Tp3Random(6) in 0..5');

  if Failures = 0 then
  begin
    Writeln('test_tp3compat: all checks passed');
    Halt(0);
  end;
  Writeln('test_tp3compat: ', Failures, ' failure(s)');
  Halt(1);
end.
