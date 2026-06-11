{ Tp3Compat - Turbo Pascal 3 runtime compatibility helpers for the port.

  * Tp3GameReal: the on-disk 'real' type.  TP3 real is the 6-byte
    Real48; FPC's real is an 8-byte double.  THighScoreEntry (stored in
    winners.win as 'file of') must stay 27 bytes, so the record field is
    declared Tp3GameReal.  FPC reads Real48 in expressions natively but
    cannot assign a double TO one, hence DoubleToTp3GameReal.
  * Tp3Random / Tp3Randomize: TP3 Random(N) equivalents.  Tp3Randomize
    honours the SOCHER_SEED environment variable (decimal integer) so
    scripted runs are deterministic; otherwise it seeds from the clock. }
unit Tp3Compat;

{$MODE TP}

interface

type
  Tp3GameReal = Real48;

function DoubleToTp3GameReal(D: Double): Tp3GameReal;
function Tp3GameRealToDouble(R: Tp3GameReal): Double;
function Tp3Random(Range: Integer): Integer;
procedure Tp3Randomize;

implementation

uses
  SysUtils;

function DoubleToTp3GameReal(D: Double): Tp3GameReal;
var
  Bits, Mantissa: QWord;
  Sign: Byte;
  Exp: LongInt;
  Res: Tp3GameReal;
  R: array[0..5] of Byte absolute Res;
begin
  Move(D, Bits, 8);
  Sign := Byte(Bits shr 63);
  Exp := LongInt((Bits shr 52) and $7FF);
  Mantissa := Bits and QWord($000FFFFFFFFFFFFF);
  FillChar(R, SizeOf(R), 0);
  if (Exp > 0) and (Exp < $7FF) then          { zero/denormal/inf/nan -> 0 }
  begin
    { rebias: double 2^(Exp-1023) -> real48 2^(Exp48-129) }
    Exp := Exp - 1023 + 129;
    { keep the top 39 of the 52 stored mantissa bits, round to nearest }
    Mantissa := Mantissa shr 13;
    if (Bits and (QWord(1) shl 12)) <> 0 then
    begin
      Inc(Mantissa);
      if Mantissa = (QWord(1) shl 39) then    { rounding carried out }
      begin
        Mantissa := 0;
        Inc(Exp);
      end;
    end;
    if Exp >= 1 then                          { underflow -> 0 }
    begin
      if Exp > 255 then                       { overflow -> saturate }
      begin
        Exp := 255;
        Mantissa := (QWord(1) shl 39) - 1;
      end;
      R[0] := Byte(Exp);
      R[1] := Byte(Mantissa);
      R[2] := Byte(Mantissa shr 8);
      R[3] := Byte(Mantissa shr 16);
      R[4] := Byte(Mantissa shr 24);
      R[5] := Byte((Mantissa shr 32) and $7F) or Byte(Sign shl 7);
    end;
  end;
  DoubleToTp3GameReal := Res;
end;

{ FPC converts Real48 -> double on assignment but does not accept a
  Real48 operand in comparisons or Write; the game's two read sites go
  through this helper. }
function Tp3GameRealToDouble(R: Tp3GameReal): Double;
var
  D: Double;
begin
  D := R;
  Tp3GameRealToDouble := D;
end;

function Tp3Random(Range: Integer): Integer;
begin
  Tp3Random := System.Random(Range);
end;

procedure Tp3Randomize;
var
  SeedText: string;
  SeedValue: LongInt;
  ErrPos: Integer;
begin
  SeedText := GetEnvironmentVariable('SOCHER_SEED');
  if SeedText <> '' then
  begin
    Val(SeedText, SeedValue, ErrPos);
    if ErrPos = 0 then
    begin
      RandSeed := SeedValue;
      Exit;
    end;
  end;
  Randomize;
end;

end.
