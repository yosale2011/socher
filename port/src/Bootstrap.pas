unit Bootstrap;

{ Single-exe support: on startup, extract the embedded game assets to a
  persistent per-user directory and chdir into it, so the unmodified game
  code finds its files with plain relative paths. The player's own files
  (winners.win high scores, port.cfg settings) live in the same directory
  and are never part of the bundle, so they survive between runs and
  between game versions. }

{$mode objfpc}{$H+}

interface

implementation

uses
  SysUtils, AssetData;

procedure ExtractAssets;
var
  Dir: string;
  I: Integer;
  F: file;
begin
  Dir := SysUtils.GetEnvironmentVariable('LOCALAPPDATA');
  if Dir = '' then
    Dir := SysUtils.GetEnvironmentVariable('TEMP');
  if Dir = '' then
    Dir := ExtractFilePath(ParamStr(0));
  Dir := IncludeTrailingPathDelimiter(Dir) + 'SocherHayam';
  if not DirectoryExists(Dir) then
    if not CreateDir(Dir) then
      raise Exception.Create('Cannot create ' + Dir);
  for I := 0 to AssetCount - 1 do
  begin
    Assign(F, Dir + DirectorySeparator + AssetName(I));
    Rewrite(F, 1);
    BlockWrite(F, AssetPtr(I)^, AssetSize(I));
    Close(F);
  end;
  { First run only: seed the original game's shipped high-score table.
    Never overwritten afterwards - it is the player's score file. }
  if not FileExists(Dir + DirectorySeparator + 'winners.win') then
  begin
    Assign(F, Dir + DirectorySeparator + 'winners.win');
    Rewrite(F, 1);
    BlockWrite(F, FactoryWinners[0], Length(FactoryWinners));
    Close(F);
  end;
  if not SetCurrentDir(Dir) then
    raise Exception.Create('Cannot enter ' + Dir);
end;

initialization
  ExtractAssets;
end.
