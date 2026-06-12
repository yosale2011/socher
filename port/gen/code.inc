{ GENERATED FILE - do not edit. Produced by port/tools/transpile.py from CODE.PAS. }
function GetPressedKey: char; forward;

procedure WaitForKey;
var
  PressedKey: char;
begin
  repeat
    PressedKey := GetPressedKey;
  until ((PressedKey = Enter) or (PressedKey = Escape)) and not PortKeyPressed;
end;

procedure ShowMessage(WaitForKeyFlag: char; Line1, Line2, Line3: TMessageLine);
begin
  PortGetPic(PreMessageBackground, 10, 138 - 29, 10 + 221, 138);
  PortPutPic(MessageSign^, 10, 138);
  PortGotoXY(29 - Length(Line1), 15);
  Write(Line1);
  PortGotoXY(29 - Length(Line2), 16);
  Write(Line2);
  PortGotoXY(29 - Length(Line3), 17);
  Write(Line3);
  if (WaitForKeyFlag = 'W') or (WaitForKeyFlag = 'R') then
  begin
    WaitForKey;
    PortPutPic(PreMessageBackground, 10, 138);
  end;
end;

procedure ShowScrFile(Path: TPath);
var
  ScrFile: file;
  Contents: PScrBuffer;
begin
  Assign(ScrFile, Path);
  {$I-} Reset(ScrFile); {$I+}
  if IOResult <> 0 then
    Writeln('çâó ÖêâÄ ', Path, ' ïÅó')
  else
  begin
    New(Contents);
    BlockRead(ScrFile, Contents^, 126);
    Close(ScrFile);
    PortPutPic(Contents^, 0, 199);
    Dispose(Contents);
  end;
end;

function LoadWinFile(FileName: TPath): PWinBuffer;
var
  WinFile: file;
  Contents: PWinBuffer;
  NumBlocks: integer;
begin
  Assign(WinFile, FileName + '.win');
  {$I-} Reset(WinFile); {$I+}
  if IOResult <> 0 then
    Writeln('çâó ÖêâÄ ', FileName, '.win ïÅó')
  else
  begin
    New(Contents);
    NumBlocks := 8832 div 128;
    BlockRead(WinFile, Contents^, NumBlocks);
    Close(WinFile);
  end;
  PortGotoXY(1, 24);
  LoadWinFile := Contents;
end;

function LoadSgnFile(FileName: TPath): PSgnBuffer;
var
  SgnFile: file;
  Contents: PSgnBuffer;
  NumBlocks: integer;
begin
  if FileName <> 'day7' then
    Assign(SgnFile, ExtraFilesPath + FileName + '.sgn')
  else
    Assign(SgnFile, 'day7.sgn');

  {$I-} Reset(SgnFile); {$I+}
  if IOResult <> 0 then
    Writeln('çâó ÖêâÄ ', ExtraFilesPath, FileName, '.sgn ïÅó')
  else
  begin
    New(Contents);
    NumBlocks := FileSize(SgnFile);
    BlockRead(SgnFile, Contents^, NumBlocks);
    Close(SgnFile);
  end;
  LoadSgnFile := Contents;
end;

function LoadScrFile(FileName: TPath): PScrBuffer;
var
  ScrFile: file;
  Contents: PScrBuffer;
begin
  Assign(ScrFile, ExtraFilesPath + FileName + '.scr');
  {$I-} Reset(ScrFile); {$I+}
  if IOResult <> 0 then
    Writeln('çâó ÖêâÄ ', ExtraFilesPath, FileName, '.scr ïÅó')
  else
  begin
    New(Contents);
    BlockRead(ScrFile, Contents^, 126);
    Close(ScrFile);
  end;
  PortGotoXY(1, 24);
  LoadScrFile := Contents;
end;

function GetTotalCargoValue: real;
var
  Ware: integer;
  Total: real;
begin
  Total := 0;
  for Ware := Copper to Wheat do
    Total := Total + 1.0 * Cargo[Ware] * BaseWarePrice[Ware];
  GetTotalCargoValue := Total;
end;

function GetTotalCargoWeight: integer;
var
  Ware: integer;
  Total: integer;
begin
  Total := 0;
  for Ware := Copper to Wheat do
    Total := Total + Cargo[Ware];
  GetTotalCargoWeight := Total;
end;

function ToStrWithCommas(Number: real): TMoneyString;
var
  NumberString: TMoneyString;
  I: integer;

  function ZeroPad3(NumberSegment: real): TMoneyString;
  begin
    Str(NumberSegment:1:0, NumberString);
    while Length(NumberString) < 3 do { PORT PATCH: was for I := 1 to 2 do; FPC rejects a for-counter owned by the enclosing function; pads the same zeroes }
      if Length(NumberString) < 3 then
        NumberString := '0' + NumberString;
    ZeroPad3 := NumberString;
  end;

begin
  if Number <= 999 then
  begin
    Str(Number:1:0, NumberString);
    ToStrWithCommas := NumberString;
  end
  else
  begin
    ToStrWithCommas := ToStrWithCommas(Int(Number / 1000)) + ',' +
        ZeroPad3(1000 * Frac(Number / 1000));
  end;
end;

procedure GetMostValuableCargo(MinCargo: integer; var OutWare: integer; var OutValue: real);
var
  Ware: integer;
begin
  OutWare := 0;
  OutValue := 0;
  for Ware := Copper to Wheat do
  begin
    if (1.0 * Cargo[Ware] * BaseWarePrice[Ware] > OutValue) and
        (Cargo[Ware] >= MinCargo) then
    begin
      OutWare := Ware;
      OutValue := 1.0 * Cargo[Ware] * BaseWarePrice[Ware];
    end;
  end;
end;

procedure ShowStormScreen;
begin
  if StormCountryName = ' ' then
    ShowScrFile(ExtraFilesPath + 'goodwthr.scr')
  else
  begin
    PortGotoXY(28, 20);
    ShowScrFile(ExtraFilesPath + 'badwthr.scr');
    Write(StormCountryName:6);
  end;
  WaitForKey;
end;

procedure ShowHelpScreen;
var
  CurrentScreenContents: PScrBuffer;
  CurrentX: integer;
  CurrentY: integer;
  Buffer: string[8];
begin
  CurrentX := PortWhereX;
  CurrentY := PortWhereY;
  New(CurrentScreenContents);
  PortGetPic(CurrentScreenContents^, 0, 0, 319, 199);
  PortColorTable(0, 1, 2, 3);
  PortPutPic(HelpScreen^, 0, 199);
  PortGotoXY(9, 7);
  if StormCountryName = ' ' then
    Write('ÑÄê':17)
  else
    Write(StormCountryName + ' òÖÜÄÅ Ñòíë':17);
  PortGotoXY(9, 12);
  Write(BankBalance:17:0);
  PortGotoXY(9, 17);
  Str(ShipCapacity, Buffer);
  Write('èÖà ' + Buffer:17);
  PortGotoXY(9, 22);
  Str(ShipDamage:1:0, Buffer);
  if ShipDamage <= 0 then
    Write('èâóö':17)
  else
    Write('åóô ' + Buffer + ' åô óÜê':17);
  repeat
    LastPressedKey := PortReadKey;
  until not PortKeyPressed;
  PortColorTable(0, 1, 2, 3);
  PortPutPic(CurrentScreenContents^, 0, 199);
  PortGotoXY(CurrentX, CurrentY);
  Dispose(CurrentScreenContents);
end;

function GetPressedKey;
var
  PressedKey: char;
begin
  PressedKey := PortReadKey;
  if (PressedKey = Escape) and PortKeyPressed then
    PressedKey := PortReadKey;
  if PressedKey = F10 then
  begin
    ShowHelpScreen;
    PressedKey := GetPressedKey;
  end;
  GetPressedKey := PressedKey;
end;

{ Note: there is no default case - if CountryName matches no country,
  the result is left undefined (as in the original) }
function CountryNameToIndex(CountryName: TCountryName): integer;
begin
  if CountryName = TurkeyName then
    CountryNameToIndex := Turkey
  else if CountryName = IsraelName then
    CountryNameToIndex := Israel
  else if CountryName = EgyptName then
    CountryNameToIndex := Egypt;
end;

function CountryIndexToName(CountryIndex: integer): TCountryName;
begin
  case CountryIndex of
    Turkey: CountryIndexToName := TurkeyName;
    Israel: CountryIndexToName := IsraelName;
    Egypt: CountryIndexToName := EgyptName;
  end;
end;

{ Option count of 100 + N means that the option's unselected pic doesn't need to be restored
  when Enter is pressed, e.g., when the screen is exited afterwards }
function ShowOptions(OptionRects: TRectArray; OptionCount: integer): integer;
var
  SelectedOption: integer;
  SelectedOptionOriginalPic: array[1..800] of byte;
  PressedKey: char;
  NeedsRestoreOnEnter: boolean;
begin
  NeedsRestoreOnEnter := False;
  if OptionCount > 100 then
    OptionCount := OptionCount - 100
  else
    NeedsRestoreOnEnter := True;

  SelectedOption := 1;
  repeat
    with OptionRects[SelectedOption] do
    begin
      PortGetPic(SelectedOptionOriginalPic, X1, Y1, X2, Y2);
      PortColorTable(3, 2, 1, 0);
      PortPutPic(SelectedOptionOriginalPic, X1, Y2);
      repeat
        PressedKey := GetPressedKey;
      until PressedKey in [Up, Left, Down, Right, ' ', Enter, Escape];
      PortColorTable(0, 1, 2, 3);

      if (PressedKey <> Enter) or NeedsRestoreOnEnter then
        PortPutPic(SelectedOptionOriginalPic, X1, Y2);

      case PressedKey of
        Up, Left: SelectedOption := SelectedOption - 1;
        ' ', Down, Right: SelectedOption := SelectedOption + 1;
        Escape: SelectedOption := -1;
      end;

      if SelectedOption = 0 then
        SelectedOption := OptionCount
      else if SelectedOption = OptionCount + 1 then
        SelectedOption := 1;
    end;
  until (PressedKey = Enter) or (PressedKey = Escape);
  ShowOptions := SelectedOption;
end;

{ Min and max are ignored if 0 }
function InputReal(MinNumber, MaxNumber, MaxDigits: integer): real;
var
  PressedKey: char;
  CurrentNumber: real;
  ShouldReturn: boolean;
  CurrentX: integer;
  CurrentY: integer;
  Unused: array[1..4] of byte;
  DigitCount: integer;
  Unused2: integer;
  MaxNumberString: string[5];
  MinNumberString: string[5];
begin
  CurrentX := PortWhereX;
  CurrentY := PortWhereY;
  DigitCount := 0;
  repeat
  begin
    CurrentNumber := 0;
    { Original quirk: loop variable and upper bound are the same variable; this clears
      the previously entered digits with spaces before re-prompting }
    for DigitCount := 1 to DigitCount do
      Write(' ');
    ShouldReturn := False;
    DigitCount := 0;
    PortGotoXY(CurrentX, CurrentY);
    repeat
    begin
      PressedKey := GetPressedKey;
      case PressedKey of
        F10: ShowHelpScreen;
        '0': begin
          if DigitCount < MaxDigits then
          begin
            CurrentNumber := 10 * CurrentNumber + Ord(PressedKey) - 48;
            if (CurrentNumber <> 0) or (DigitCount = 0) then
              Write(PressedKey);
            DigitCount := DigitCount + 1;
          end;
        end;
        '1'..'9': begin
          if DigitCount < MaxDigits then
          begin
            CurrentNumber := 10 * CurrentNumber + Ord(PressedKey) - 48;
            Write(PressedKey);
            if CurrentNumber <> 0 then
              DigitCount := DigitCount + 1;
          end;
        end;
        Backspace: begin
          if DigitCount > 0 then
          begin
            CurrentNumber := Int(CurrentNumber / 10);
            Write(Backspace, ' ', Backspace);
            DigitCount := DigitCount - 1;
          end;
        end;
        Enter: begin
          if DigitCount > 0 then
            ShouldReturn := True;
        end;
        Escape: begin
          CurrentNumber := -1;
          ShouldReturn := True;
        end;
        { Port upgrade: quick-amount keys, active only when the caller armed them.
          The Hebrew letters are what the a/b/c keys produce on a Hebrew layout }
        '=', 'a', 'A', 'ô': begin
          if QuickAmountA > 0 then
          begin
            CurrentNumber := QuickAmountA;
            Write(CurrentNumber:1:0);
            ShouldReturn := True;
          end;
        end;
        'b', 'B', 'ê': begin
          if QuickAmountB > 0 then
          begin
            CurrentNumber := QuickAmountB;
            Write(CurrentNumber:1:0);
            ShouldReturn := True;
          end;
        end;
        'c', 'C', 'Å': begin
          if QuickAmountC > 0 then
          begin
            CurrentNumber := QuickAmountC;
            Write(CurrentNumber:1:0);
            ShouldReturn := True;
          end;
        end;
      end;
      if DigitCount = MaxDigits then
      begin
        PortSound(700);
        PortDelay(50);
        PortNoSound;
        PortDelay(80);
      end;
    end
    until ShouldReturn;
    { Port fix: Escape returns -1 as a cancel signal and must bypass the
      range check, otherwise prompts with a max (added by the port) trap
      the player in the "enter a number between" loop }
    if (CurrentNumber <> -1) and (not ((MinNumber = 0) and (MaxNumber = 0))) and
        (((CurrentNumber >= 0) and (CurrentNumber > MaxNumber)) or
        ((CurrentNumber <= 0) and (CurrentNumber < MinNumber))) then
    begin
      PortGotoXY(2, 10);
      Str(MinNumber, MinNumberString);
      Str(MaxNumber, MaxNumberString);
      ShowMessage('W', '', MaxNumberString + ' -å ' + MinNumberString + ' èâÅ òîëé ëêãÑ Äê', Bell);
      PortGotoXY(CurrentX, CurrentY);
      ShouldReturn := False;
    end;
  end
  until ShouldReturn;
  InputReal := CurrentNumber;
end;

{ Unused }
function InputInteger(MinNumber, MaxNumber, MaxDigits: integer): integer;
begin
  InputInteger := Trunc(InputReal(MinNumber, MaxNumber, MaxDigits));
end;

function InputYesNo: boolean;
var
  Ret: boolean;
  PressedKey: char;
  I: integer;
  InputLength: integer;
  X: integer;
  Y: integer;
begin
  X := PortWhereX;
  Y := PortWhereY;
  InputLength := 0;
  repeat
  begin
    PortGotoXY(X, Y);
    PressedKey := GetPressedKey;
    case PressedKey of
      'ã', 'f', 'F': begin
        Ret := True;
        InputLength := InputLength + 1;
      end;
      'å', 'k', 'K': begin
        Ret := False;
        InputLength := InputLength + 1;
      end;
      Escape, ' ': Ret := not Ret;
      else begin
        if PressedKey <> Enter then
        begin
          for I := 1 to 2 do
          begin
            PortSound(700);
            PortDelay(50);
            PortNoSound;
            PortDelay(80);
          end;
        end;
      end
    end;
    if InputLength > 0 then
    begin
      if Ret then
        Write('èã')
      else
        Write('Äå');
    end;
  end
  until (PressedKey = Enter) and (InputLength > 0);
  InputYesNo := Ret;
end;

procedure PlayDay;
var
  CopperPriceIsrael: integer;
  CopperPriceTurkey: integer;
  CopperPriceEgypt: integer;
  OlivesPriceIsrael: integer;
  OlivesPriceTurkey: integer;
  OlivesPriceEgypt: integer;
  WheatPriceIsrael: integer;
  WheatPriceTurkey: integer;
  WheatPriceEgypt: integer;
  MainMenuOptionCount: integer;
  ChosenMainMenuOption: integer;
  ShouldSkipDay: boolean;
  MainMenuOptionRects: TRectArray;
  CountryNameX1: integer;
  CountryNameX2: integer;
  Unused: array[1..4] of byte;

  procedure ShowHUD;
  begin
    PortGotoXY(73, 1);
    Write(CurrentCountryName:6);
    PortGotoXY(32, 3);
    Write(DayName[Day]);
    PortGotoXY(32, 5);
    Write(TimeOfDay:2);
    Write(':00');
    PortGotoXY(32, 17);
    Write(' ':8);
    PortGotoXY(34, 17);
    if Money >= 10000.0 then
      PortGotoXY(33, 17);
    if Money >= 100000.0 then
      PortGotoXY(32, 17);
    Write(ToStrWithCommas(Money):5);
    { Port fix: the original cleared stale digits only for wheat (below)
      and money (above); copper and olives kept leftover digits whenever
      the cargo count shrank to fewer digits. Clear widths are per-row:
      the bitmap labels start at column 35 (copper, 6 letters) and 36
      (olives, 5 letters), so wider clears would erase the labels }
    PortGotoXY(32, 9);
    Write(' ':3);
    PortGotoXY(32, 9);
    Write(Cargo[Copper]:3);
    PortGotoXY(32, 11);
    Write(' ':4);
    PortGotoXY(32, 11);
    Write(Cargo[Olives]:3);
    PortGotoXY(32, 13);
    Write(' ':5);
    PortGotoXY(32, 13);
    Write(Cargo[Wheat]:3);
    PortGotoXY(30, 21);
    Write(CopperPriceTurkey:4);
    PortGotoXY(30, 23);
    Write(OlivesPriceTurkey:4);
    PortGotoXY(30, 25);
    Write(WheatPriceTurkey:4);
    PortGotoXY(16, 21);
    Write(CopperPriceIsrael:4);
    PortGotoXY(16, 23);
    Write(OlivesPriceIsrael:4);
    PortGotoXY(16, 25);
    Write(WheatPriceIsrael:4);
    PortGotoXY(2, 21);
    Write(CopperPriceEgypt:4);
    PortGotoXY(2, 23);
    Write(OlivesPriceEgypt:4);
    PortGotoXY(2, 25);
    Write(WheatPriceEgypt:4);
    { Port upgrade: storm badge over the stormy country's panel caption,
      so the player can see the hazard before choosing a destination.
      Drawn at pixel positions (PortTextAt platform extension) because the
      bitmap captions sit between the 8-pixel text grid rows. }
    if StormCountryName <> ' ' then
    begin
      PortTextColor(2);
      if StormCountryName = EgyptName then
        PortTextAt(4, 147, 'Ñòíë')
      else if StormCountryName = IsraelName then
        PortTextAt(111, 147, 'Ñòíë')
      else
        PortTextAt(217, 147, 'Ñòíë');
      PortTextColor(3);
    end;
    { Port upgrade: show the ship's cargo capacity next to the inventory
      title, so the player knows how much still fits }
    PortGotoXY(31, 7);
    Write(ShipCapacity:3);
  end;

  { Unused }
  function IsSub(C: char): boolean;
  begin
    if C = #26 then
      IsSub := True
    else
      IsSub := False;
  end;

  function CannotAfford(LostAmount: real): boolean;
  begin
    if LostAmount > Money then
    begin
      ShowMessage('W', '', '!ìëã óâîëé äöÖôòÅ èâÄ', Bell);
      CannotAfford := True;
    end
    else
      CannotAfford := False;
  end;

  function OfferFlow(var InOutCopper: integer; var InOutOlives: integer; var InOutWheat: integer;
                     var InOutMoney: real): boolean;
  var
    OfferedCopper: integer;
    OfferedOlives: integer;
    OfferedWheat: integer;
    OfferedMoney: real;
    TotalOfferValue: real;
    AcceptThreshold: real;
    Unused: real;

    function CanAffordMoney(OfferedAmount: real; AvailableMoney: real): boolean;
    begin
      if OfferedAmount < 0 then
        OfferedAmount := 0;
      if OfferedAmount <= AvailableMoney then
        CanAffordMoney := True
      else
      begin
        ShowMessage('W', '', '!ÑÜ çÖãë äöÖôòÅ èâÄ', Bell);
        CanAffordMoney := False;
      end;
    end;

    function CanAffordWare(var OfferedAmount: integer; AvailableCargo: real): boolean;
    begin
      if OfferedAmount < 0 then
        OfferedAmount := 0;

      if OfferedAmount <= AvailableCargo then
        CanAffordWare := True
      else
      begin
        ShowMessage('W', '', '!ÑÜ çÖãë äöÖôòÅ èâÄ', Bell);
        CanAffordWare := False;
      end;
    end;

  begin
    OfferedCopper := 0;
    OfferedOlives := 0;
    OfferedWheat := 0;
    OfferedMoney := 0;
    if InOutCopper > 0 then
    begin
      repeat
      begin
        ShowMessage('N','?ööå èãÖé ÑöÄ' + WareName[Copper] + 'èÖà Ñéã', '', '');
        PortGotoXY(20, 16);
        repeat
          OfferedCopper := Trunc(InputReal(0, MaxSafeInteger, 5));
        until OfferedCopper >= 0;
      end
      until (OfferedCopper >= 0) and CanAffordWare(OfferedCopper, InOutCopper);
    end;
    if InOutOlives > 0 then
    begin
      repeat
      begin
        ShowMessage('N','?ööå èãÖé ÑöÄ' + WareName[Olives] + 'èÖà Ñéã', '', '');
        PortGotoXY(20, 16);
        repeat
          OfferedOlives := Trunc(InputReal(0, MaxSafeInteger, 5));
        until OfferedOlives >= 0;
      end
      until CanAffordWare(OfferedOlives, InOutOlives);
    end;
    if InOutWheat > 0 then
    begin
      repeat
      begin
        ShowMessage('N', '?ööå èãÖé ÑöÄ' + WareName[Wheat] + 'èÖà Ñéã', '', '');
        PortGotoXY(20, 16);
        repeat
          OfferedWheat := Trunc(InputReal(0, MaxSafeInteger, 5));
        until OfferedWheat >= 0;
      end
      until CanAffordWare(OfferedWheat, InOutWheat);
    end;
    if InOutMoney > GetTotalCargoValue / 10 then
    begin
      repeat
      begin
        ShowMessage('N', '?ööå èãÖé ÑöÄ ìëã Ñéã', '', '');
        PortGotoXY(20, 16);
        repeat
          OfferedMoney := InputReal(0, 0, 8);
        until OfferedMoney >= 0;
      end
      until CanAffordMoney(OfferedMoney, InOutMoney);
    end;
    TotalOfferValue := OfferedCopper * BaseWarePrice[Copper] +
        OfferedOlives * BaseWarePrice[Olives] + OfferedWheat * BaseWarePrice[Wheat] + OfferedMoney;
    AcceptThreshold := (GetTotalCargoValue + InOutMoney) / (Tp3Random(6) + 4);
    if TotalOfferValue > AcceptThreshold then
    begin
      InOutMoney := InOutMoney - OfferedMoney;
      InOutCopper := InOutCopper - OfferedCopper;
      InOutOlives := InOutOlives - OfferedOlives;
      InOutWheat := InOutWheat - OfferedWheat;
      OfferFlow := True;
    end
    else
      OfferFlow := False;
  end;

  { Outcome types for ApplyBadOutcome }
  const
    LoseCargo = 'é';
    SinkingShip = 'ô';
    AddDamage = 'ê';
    LoseMoney = 'ã';

  procedure ApplyBadOutcome(OutcomeType: char; var OutWare: integer;
                            var OutAmountString: TNumberString);
  var
    IsOutWareSet: boolean;
    Ware: integer;
    ExtraLostWare: integer;
    ExtraLostAmount: integer;
    LostAmount: integer;
  begin
    LostAmount := 0;
    IsOutWareSet := False;
    if OutcomeType = LoseCargo then
    begin
      for Ware := Copper to Wheat do
      begin
        if (not IsOutWareSet) and (Cargo[Ware] > 2) then
        begin
          LostAmount := Trunc((20.0 + Tp3Random(21)) / 100 * Cargo[Ware]);
          if LostAmount < 1 then
            LostAmount := 1;
          Cargo[Ware] := Cargo[Ware] - Trunc(LostAmount);
          OutWare := Ware;
          IsOutWareSet := True;
        end;
      end;
    end
    else if OutcomeType = SinkingShip then
    begin
      if GetTotalCargoWeight <= (3 * ShipCapacity) then
      begin
        ApplyBadOutcome(LoseCargo, OutWare, OutAmountString);
        Val(OutAmountString, LostAmount, Ware);
      end
      else
      begin
        OutWare := Copper;
        if Cargo[Olives] > Cargo[Copper] then
          OutWare := Olives;
        if Cargo[Wheat] > Cargo[OutWare] then
          OutWare := Wheat;
        LostAmount := GetTotalCargoWeight - 3 * ShipCapacity;
        if LostAmount >= Cargo[OutWare] then
        begin
          LostAmount := Cargo[OutWare];
          Cargo[OutWare] := 0;
        end
        else
        begin
          Cargo[OutWare] := Cargo[OutWare] - Trunc(LostAmount);
          ApplyBadOutcome(LoseCargo, ExtraLostWare, OutAmountString);
          Val(OutAmountString, ExtraLostAmount, Ware);
          if ExtraLostWare = OutWare then
            LostAmount := LostAmount + ExtraLostAmount
          else
            Cargo[ExtraLostWare] := Cargo[ExtraLostWare] + ExtraLostAmount;
        end;
      end;
    end
    else if OutcomeType = LoseMoney then
    begin
      LostAmount := 10 * Trunc(Money / (Tp3Random(40) + 40));
      Money := Money - LostAmount;
    end
    else if OutcomeType = AddDamage then
    begin
      LostAmount := 10 * Trunc((Money + GetTotalCargoValue) / (Tp3Random(30) + 50));
      if LostAmount < 100 then
        LostAmount := 100;
      ShipDamage := ShipDamage + LostAmount;
    end;
    Str(LostAmount, OutAmountString);
  end;

  procedure GenerateNewPrices;
  begin
    CopperPriceIsrael := BaseWarePrice[Copper] - 500 + Tp3Random(11) * 100;
    OlivesPriceIsrael := BaseWarePrice[Olives] - 150 + Tp3Random(7) * 50;
    WheatPriceIsrael := BaseWarePrice[Wheat] - 15 + Tp3Random(8) * 5;
    CopperPriceTurkey := BaseWarePrice[Copper] - 500 + Tp3Random(11) * 100;
    OlivesPriceTurkey := BaseWarePrice[Olives] - 150 + Tp3Random(7) * 50;
    WheatPriceTurkey := BaseWarePrice[Wheat] - 15 + Tp3Random(8) * 5;
    CopperPriceEgypt := BaseWarePrice[Copper] - 500 + Tp3Random(11) * 100;
    OlivesPriceEgypt := BaseWarePrice[Olives] - 150 + Tp3Random(7) * 50;
    WheatPriceEgypt := BaseWarePrice[Wheat] - 15 + Tp3Random(8) * 5;
  end;

  procedure UpdateStorm(var OutStormCountryName: TCountryName);
  var
    Unused: integer;
  begin
    if (Tp3Random(8) < 3) and (Day <> 1) then
      OutStormCountryName := CountryIndexToName(Tp3Random(3) + 1)
    else
      OutStormCountryName := ' ';
  end;

  procedure DoDayStartEvent;
  var
    RandomEvent: integer;
    SpecialPriceCountry: integer;
    SpecialPrice: integer;
    MerchantPrice: integer;
    ExpandPrice: integer;
    ExpandAmount: integer;
    MerchantSupply: integer;
    Ware: integer;
    FishingBoatDamage: integer;
    ChosenCrewOption: integer;
    ExpandWareCost: integer;
    LostAmountString: TNumberString;
    MerchantPriceString: TNumberString;
    StringBuffer: TNumberString;
    ExpandWareCostString: TMessageLine;
    CrewOptionRects: TRectArray;
    DidEventHappen: boolean;
    MerchantSupplyBaseValue: real;
    MostValuableCargoValue: real;
    ExpandWareCosts: array[Copper..Wheat] of integer;
    ExpandMoneyCost: integer;
    ExpandBankBalanceCost: integer;

    { Values for UpDownFlag }
    const
      PriceUp = 1;
      PriceDown = 2;

    procedure UpdateSpecialPrices(UpDownFlag: integer; Country: integer; NewPrice: integer;
                                  Ware: integer);
    var
      UpDownString: string[6];
      NewPriceString: TNumberString;
    begin
      if UpDownFlag = PriceUp then
        UpDownString := '-å ïîó'
      else
        UpDownString := '-å Éòâ';
      Str(NewPrice, NewPriceString);
      case Country of
        Turkey: begin
          case Ware of
            Copper: CopperPriceTurkey := NewPrice;
            Olives: OlivesPriceTurkey := NewPrice;
            Wheat: WheatPriceTurkey := NewPrice;
          end;
          PortGotoXY(7, 2 + 21 * (UpDownFlag - 1));
          Writeln('ÑâãòÖöÅ' + WareName[Ware] + Backspace + 'Ñ òâáé':27);
          PortGotoXY(7, 3 + 21 * (UpDownFlag - 1));
          Writeln('!!çâåóô ' + NewPriceString + UpDownString:25);
        end;
        Israel: begin
          case Ware of
            Copper: CopperPriceIsrael := NewPrice;
            Olives: OlivesPriceIsrael := NewPrice;
            Wheat: WheatPriceIsrael := NewPrice;
          end;
          PortGotoXY(7, 2 + 21 * (UpDownFlag - 1));
          Writeln('åÄòôâÅ' + WareName[Ware] + Backspace + 'Ñ òâáé':27);
          PortGotoXY(7, 3 + 21 * (UpDownFlag - 1));
          Writeln('!!çâåóô ' + NewPriceString + UpDownString:25);
        end;
        Egypt: begin
          case Ware of
            Copper: CopperPriceEgypt := NewPrice;
            Olives: OlivesPriceEgypt := NewPrice;
            Wheat: WheatPriceEgypt := NewPrice;
          end;
          PortGotoXY(7, 2 + 21 * (UpDownFlag - 1));
          Writeln('çâòñéÅ' + WareName[Ware] + Backspace + 'Ñ òâáé':27);
          PortGotoXY(7, 3 + 21 * (UpDownFlag - 1));
          Writeln('!!çâåóô ' + NewPriceString + UpDownString:25);
        end;
      end;
      WaitForKey;
    end;

  { Options in crew menu }
  const
    NewCrew = 1;
    Offer = 2;

  begin
    repeat
    begin
      DidEventHappen := True;
      repeat
        RandomEvent := Tp3Random(30) + 1;
      until (RandomEvent <> LastDayStartEvent) and
          (not ((RandomEvent in [1..2]) and (LastDayStartEvent in [1..2]))) and
          (not ((RandomEvent in [3..4]) and (LastDayStartEvent in [3..4]))) and
          (not ((RandomEvent in [5..6]) and (LastDayStartEvent in [5..6]))) and
          (not ((RandomEvent in [7..8]) and (LastDayStartEvent in [7..8]))) and
          (not ((RandomEvent in [9..10]) and (LastDayStartEvent in [9..10]))) and
          (not ((RandomEvent in [11..12]) and (LastDayStartEvent in [11..12]))) and
          (not ((RandomEvent in [13..18]) and (LastDayStartEvent in [13..18]))) and
          (not ((RandomEvent in [19..20]) and (LastDayStartEvent in [19..20]))) and
          (not ((RandomEvent in [21..22]) and (LastDayStartEvent in [21..22]))) and
          (not ((RandomEvent in [23..26]) and (LastDayStartEvent in [23..26]))) and
          (not ((RandomEvent in [27..28]) and (LastDayStartEvent in [27..28]))) and
          (not ((RandomEvent in [29..30]) and (LastDayStartEvent in [29..30])));
      if (Money > 20000.0) and (Tp3Random(6) = 1) then
        RandomEvent := 15;

      if (Money > 45000.0) and (Tp3Random(6) = 1) then
        RandomEvent := 15;

      case RandomEvent of
        1..2: begin
          PortPutPic(CopperUpScreen^, 0, 199);
          SpecialPriceCountry := Tp3Random(3) + 1;
          SpecialPrice := BaseWarePrice[Copper] + 1100 + 100 * Tp3Random(5);
          UpdateSpecialPrices(PriceUp, SpecialPriceCountry, SpecialPrice, Copper);
        end;
        3..4: begin
          PortPutPic(OlivesUpScreen^, 0, 199);
          SpecialPriceCountry := Tp3Random(3) + 1;
          SpecialPrice := BaseWarePrice[Olives] + 200 + 50 * Tp3Random(4);
          UpdateSpecialPrices(PriceUp, SpecialPriceCountry, SpecialPrice, Olives);
        end;
        5..6: begin
          PortPutPic(WheatUpScreen^, 0, 199);
          SpecialPriceCountry := Tp3Random(3) + 1;
          SpecialPrice := BaseWarePrice[Wheat] + 20 + Tp3Random(5) * 5;
          UpdateSpecialPrices(PriceUp, SpecialPriceCountry, SpecialPrice, Wheat);
        end;
        7..8: begin
          PortPutPic(CopperDownScreen^, 0, 199);
          SpecialPriceCountry := Tp3Random(3) + 1;
          SpecialPrice := BaseWarePrice[Copper] - 1100 - 100 * Tp3Random(5);
          UpdateSpecialPrices(PriceDown, SpecialPriceCountry, SpecialPrice, Copper);
        end;
        9..10: begin
          PortPutPic(OlivesDownScreen^, 0, 199);
          SpecialPriceCountry := Tp3Random(3) + 1;
          SpecialPrice := BaseWarePrice[Olives] - 150 - Tp3Random(3) * 50;
          UpdateSpecialPrices(PriceDown, SpecialPriceCountry, SpecialPrice, Olives);
        end;
        11..12: begin
          PortPutPic(WheatDownScreen^, 0, 199);
          SpecialPriceCountry := Tp3Random(3) + 1;
          SpecialPrice := BaseWarePrice[Wheat] - 15 - Tp3Random(3) * 5;
          UpdateSpecialPrices(PriceDown, SpecialPriceCountry, SpecialPrice, Wheat);
        end;
        13..18: begin
          ExpandMoneyCost := 0;
          ExpandBankBalanceCost := 0;
          Ware := 0;
          for Iter := Copper to Wheat do
            ExpandWareCosts[Iter] := 0;
          ExpandAmount := 50 * (Tp3Random(2) + 1);
          ExpandPrice := 10 * (Tp3Random(ExpandAmount) + 2 * ExpandAmount);
          if ExpandPrice <= Money + BankBalance + GetTotalCargoValue then
          begin
            PortPutPic(ExpandScreen^, 0, 199);
            PortGotoXY(29, 14);
            Write(ExpandAmount:3);
            if Money > ExpandPrice then
              ExpandMoneyCost := ExpandPrice
            else if BankBalance > ExpandPrice then
              ExpandBankBalanceCost := ExpandPrice
            else if Money + BankBalance > ExpandPrice then
            begin
              ExpandMoneyCost := Trunc(Money);
              ExpandBankBalanceCost := ExpandPrice - ExpandMoneyCost;
              Str(ExpandBankBalanceCost, StringBuffer);
            end
            else
            begin
              GetMostValuableCargo(1, Ware, MostValuableCargoValue);
              if MostValuableCargoValue >= ExpandPrice then
              begin
                ExpandWareCost := Round(1.0 * ExpandPrice / BaseWarePrice[Ware]);
                ExpandWareCosts[Ware] := ExpandWareCost;
                Str(ExpandWareCost, StringBuffer);
              end
              else
              begin
                Str(Cargo[Ware], StringBuffer);
                ExpandWareCosts[Ware] := Cargo[Ware];
                if (Money > BankBalance) or (Money >= ExpandMoneyCost) then
                begin
                  ExpandMoneyCost := ExpandPrice - Trunc(MostValuableCargoValue);
                  if ExpandMoneyCost > Money then
                    ExpandMoneyCost := Trunc(Money);
                  Str(ExpandMoneyCost, StringBuffer);
                end
                else
                begin
                  ExpandBankBalanceCost := ExpandPrice - Trunc(MostValuableCargoValue);
                  if ExpandBankBalanceCost > BankBalance then
                    ExpandBankBalanceCost := Trunc(BankBalance);
                  Str(ExpandBankBalanceCost, StringBuffer);
                end;
              end;
            end;
            PortGotoXY(18, 15);
            if ExpandMoneyCost + ExpandBankBalanceCost > 0 then
              Write('á"ô ', ExpandMoneyCost + ExpandBankBalanceCost);
            PortGotoXY(15, 16);
            if (Ware > 0) and (ExpandWareCosts[Ware] > 0) then
            begin
              Str(ExpandWareCosts[Ware], StringBuffer);
              ExpandWareCostString := '.' + WareName[Ware] + 'èÖà ' + StringBuffer;
              if ExpandMoneyCost + ExpandBankBalanceCost > 0 then
                ExpandWareCostString := ExpandWareCostString + ' ìëÖêÅÖ';
              Write(ExpandWareCostString:22);
            end;
            PortGotoXY(18, 18);
            if InputYesNo then
            begin
              ShipCapacity := ShipCapacity + ExpandAmount;
              Money := Money - ExpandMoneyCost;
              BankBalance := BankBalance - ExpandBankBalanceCost;
              for Iter := Copper to Wheat do
                Cargo[Iter] := Cargo[Iter] - ExpandWareCosts[Iter];
              PortPutPic(MessageSign^, 90, 194);
              PortGotoXY(18, 23);
              Write('.ÑíñÖÅ  ÑÅáòÑÑ');
              WaitForKey;
              Str(ExpandBankBalanceCost, StringBuffer);
              if ExpandBankBalanceCost > 0 then
              begin
                if ExpandMoneyCost > 0 then
                  ShowMessage('W', 'äêÖÅôáé Öáóåê åóô ' + StringBuffer, '.óêÅÅ', '')
                else
                  ShowMessage('W', '.óêÅÅ äêÖÅôáé áóåê ìëãÑ', '', '');
              end;
            end;
          end
          else
            DidEventHappen := False;
        end;
        19..20: begin
          if (Day < 6) and AllowCrewStrike then
          begin
            PortPutPic(CrewScreen^, 0, 199);
            PortGotoXY(7, 3);
            Write(Cargo[Copper]:5);
            PortGotoXY(7, 4);
            Write(Cargo[Olives]:5);
            PortGotoXY(7, 5);
            Write(Cargo[Wheat]:5);
            PortGotoXY(7, 6);
            Write(Money:5:0);
            CrewOptionRects[NewCrew].X1 := 54;
            CrewOptionRects[NewCrew].Y1 := 177;
            CrewOptionRects[NewCrew].X2 := 198;
            CrewOptionRects[NewCrew].Y2 := 183;
            CrewOptionRects[Offer].X1 := 78;
            CrewOptionRects[Offer].Y1 := 186;
            CrewOptionRects[Offer].X2 := 198;
            CrewOptionRects[Offer].Y2 := 192;
            repeat
              ChosenCrewOption := ShowOptions(CrewOptionRects, 2);
            until ChosenCrewOption > 0;
            if ChosenCrewOption = NewCrew then
            begin
              ShouldSkipDay := True;
              ShowMessage('W', '...öòáéå', '.ôÉá öÖÖñ çí ÉÅÖí ÑöÄ', '');
            end
            else if OfferFlow(Cargo[Copper], Cargo[Olives], Cargo[Wheat], Money) then
              ShowMessage('W', '.äöíñÑ öÄ åÅâó öÖÖñÑ', '', '')
            else
            begin
              ShowMessage('W', '.ÑíñÑå Åòë öÖÖñÑ', 'ÄñÖé ÑöÄÖ çââéÖâ ÖòÅí', 'ôÉá öÖÖñ');
              Day := Day + 1;
              ShouldSkipDay := True;
            end;
          end
          else
            DidEventHappen := False;
        end;
        21..22: begin
          ApplyBadOutcome(LoseMoney, Ware, LostAmountString);
          PortGotoXY(14, 24);
          if LostAmountString > '0' then
          begin
            PortPutPic(ThievesScreen^, 0, 199);
            Write('.åóô ' + LostAmountString:15);
          end
          else
          begin
            ApplyBadOutcome(LoseCargo, Ware, LostAmountString);
            if LostAmountString > '0' then
            begin
              PortPutPic(ThievesScreen^, 0, 199);
              Write('.' + WareName[Ware] + ' èÖà ' + LostAmountString:15);
            end
            else
              DidEventHappen := False;
          end;
          if DidEventHappen then
            WaitForKey;
        end;
        23..26: begin
          if Cargo[Copper] > 1 then
          begin
            PortPutPic(SocherScreen^, 0, 199);
            MerchantPrice := CopperPriceIsrael + 50 * Tp3Random(30) - 600;
            Str(MerchantPrice, MerchantPriceString);
            Str(Cargo[Copper], StringBuffer);
            PortGotoXY(3, 2);
            Writeln('.ÑòÖáëå öÖîâáÉÅ óÖóÜ òáÖë':34);
            PortGotoXY(3, PortWhereY);
            Writeln('á"ô ' + MerchantPriceString + ' äå íâñÑå èãÖé ÄÖÑ':34);
            PortGotoXY(3, PortWhereY);
            Writeln('Öå òÖãéö çÄ ,' + WareName[Copper] + 'åô èÖà åãå':34);
            PortGotoXY(3, PortWhereY);
            Writeln('.äåô èÖàÑ ' + StringBuffer + ' öÄ':34);
            Writeln;
            PortGotoXY(3, PortWhereY);
            Write('(å/ã) ? Öå òÖãéå èâêÖíé ÑöÄ çÄÑ':34);
            PortGotoXY(3, PortWhereY);
            if InputYesNo then
            begin
              Money := Money + 1.0 * MerchantPrice * Cargo[Copper];
              Cargo[Copper] := 0;
              PortGotoXY(3, PortWhereY + 2);
              Write('!!! òãéê':25);
              WaitForKey;
            end;
          end
          else if Cargo[Olives] > 1 then
          begin
            PortPutPic(SocherScreen^, 0, 199);
            MerchantPrice := OlivesPriceIsrael + 10 * Tp3Random(70) - 300;
            Str(MerchantPrice, MerchantPriceString);
            Str(Cargo[Olives], StringBuffer);
            PortGotoXY(3, 2);
            Writeln('.ÑòÖáëå öÖîâáÉÅ óÖóÜ òáÖë':34);
            PortGotoXY(3, PortWhereY);
            Writeln('á"ô ' + MerchantPriceString + ' äå íâñÑå èãÖé ÄÖÑ':34);
            PortGotoXY(3, PortWhereY);
            Writeln('Öå òÖãéö çÄ ,' + WareName[Olives] + 'åô èÖà åãå':34);
            PortGotoXY(3, PortWhereY);
            Writeln('.äåô èÖàÑ ' + StringBuffer + ' öÄ':34);
            Writeln;
            PortGotoXY(3, PortWhereY);
            Write('(å/ã) ? Öå òÖãéå èâêÖíé ÑöÄ çÄÑ':34);
            PortGotoXY(3, PortWhereY);
            if InputYesNo then
            begin
              Money := Money + 1.0 * MerchantPrice * Cargo[Olives];
              Cargo[Olives] := 0;
              PortGotoXY(3, PortWhereY + 2);
              Write('!!! òãéê':25);
              WaitForKey;
            end;
          end
          else if Cargo[Wheat] > 1 then
          begin
            PortPutPic(SocherScreen^, 0, 199);
            MerchantPrice := WheatPriceIsrael + 5 * Tp3Random(14) - 40;
            Str(MerchantPrice, MerchantPriceString);
            Str(Cargo[Wheat], StringBuffer);
            PortGotoXY(3, 2);
            Writeln('.ÑòÖáëå öÖîâáÉÅ óÖóÜ òáÖë':34);
            PortGotoXY(3, PortWhereY);
            Writeln('á"ô ' + MerchantPriceString + ' äå íâñÑå èãÖé ÄÖÑ':34);
            PortGotoXY(3, PortWhereY);
            Writeln('Öå òÖãéö çÄ ,' + WareName[Wheat] + 'åô èÖà åãå':34);
            PortGotoXY(3, PortWhereY);
            Writeln('.äåô èÖàÑ ' + StringBuffer + ' öÄ':34);
            Writeln;
            PortGotoXY(3, PortWhereY);
            Write('(å/ã) ? Öå òÖãéå èâêÖíé ÑöÄ çÄÑ':34);
            PortGotoXY(3, PortWhereY);
            if InputYesNo then
            begin
              Money := Money + 1.0 * MerchantPrice * Cargo[Wheat];
              Cargo[Wheat] := 0;
              PortGotoXY(3, PortWhereY + 2);
              Write('!!! òãéê':25);
              WaitForKey;
            end;
          end
          else
            DidEventHappen := False;
        end;
        27..28: begin
          Ware := Tp3Random(3) + 1;
          MerchantPrice := 5 * (BaseWarePrice[Ware] div 10 + Tp3Random(BaseWarePrice[Ware]) div 5);
          MerchantSupplyBaseValue := Tp3Random(Trunc(0.75 * Money) + 1) + 0.25 * Money;
          MerchantSupply := 5 * Trunc(MerchantSupplyBaseValue / BaseWarePrice[Ware] / 5 + 1);
          if (1.0 * MerchantPrice * MerchantSupply <= Money) and (Money > 0) then
          begin
            PortPutPic(SocherScreen^, 0, 199);
            Str(MerchantPrice, MerchantPriceString);
            Str(MerchantSupply, StringBuffer);
            if StringBuffer = '1' then
              StringBuffer := '';
            PortGotoXY(3, 2);
            Writeln(WareName[Ware] + ' èÖà ' + StringBuffer + ' äå òÖãéå èâêÖíé òáÖë':34);
            Writeln;
            PortGotoXY(3, PortWhereY);
            Writeln('èÖàå åóô ' + MerchantPriceString + ' òâáéÅ':34);
            Writeln;
            PortGotoXY(3, PortWhereY);
            Write('(å/ã) ? öÖêóå èâêÖíé ÑöÄ çÄÑ':34);
            PortGotoXY(3, PortWhereY);
            if InputYesNo then
            begin
              Money := Money - MerchantPrice * MerchantSupply;
              case Ware of
                1: Cargo[Copper] := Cargo[Copper] + MerchantSupply;
                2: Cargo[Olives] := Cargo[Olives] + MerchantSupply;
                3: Cargo[Wheat] := Cargo[Wheat] + MerchantSupply;
              end;
              Writeln;
              Writeln('            !!! öâêó':25);
              WaitForKey;
            end;
          end
          else
            DidEventHappen := False;
        end;
        29..30: begin
          FishingBoatDamage := 10 * Trunc((GetTotalCargoValue + Money) / (Tp3Random(80) + 150));
          if FishingBoatDamage > 0 then
          begin
            ShowScrFile(ExtraFilesPath + 'fishing.scr');
            ShipDamage := ShipDamage + FishingBoatDamage;
            PortGotoXY(20, 3);
            Write(FishingBoatDamage:4);
            WaitForKey;
          end
          else
            DidEventHappen := False;
        end;
      end;
    end
    until DidEventHappen;
    LastDayStartEvent := RandomEvent;
  end;

  procedure ShowNewDayScreen;
  var
    DaySignBuffer: PSgnBuffer;
    DayString: string[7];
  begin
    Str(Day, DayString);
    DaySignBuffer := LoadSgnFile('day' + DayString);
    ShowScrFile(ExtraFilesPath + 'newday.scr');
    PortPutPic(DaySignBuffer^, 165, 89);
    Dispose(DaySignBuffer);
    WaitForKey;
  end;

  procedure ShowBuyWindow(CopperPrice, OlivesPrice, WheatPrice: integer);
  var
    Unused: integer;
    ChosenWarePrice: integer;
    ChosenWare: integer;
    TotalCost: real;
    AmountToBuy: real;
    BuyOptionRects: TRectArray;
  begin
    AmountToBuy := 0;
    if Money < WheatPrice then
      ShowMessage('W', 'äå òôîÄé ÖêâÄ äîëã', 'öÖòÖáë öâêó', '')
    else
    begin
      case CountryNameToIndex(CurrentCountryName) of
        Turkey: begin
          PortPutPic(BuyTurkeyWindow^, 1, 142);
          BuyOptionRects[Copper].X1 := 177;
          BuyOptionRects[Copper].Y1 := 2;
          BuyOptionRects[Copper].X2 := 236;
          BuyOptionRects[Copper].Y2 := 18;
          BuyOptionRects[Olives].X1 := 72;
          BuyOptionRects[Olives].Y1 := 2;
          BuyOptionRects[Olives].X2 := 131;
          BuyOptionRects[Olives].Y2 := 18;
          BuyOptionRects[Wheat].X1 := 180;
          BuyOptionRects[Wheat].Y1 := 65;
          BuyOptionRects[Wheat].X2 := 237;
          BuyOptionRects[Wheat].Y2 := 80;
        end;
        Israel: begin
          PortPutPic(BuyIsraelWindow^, 1, 142);
          BuyOptionRects[Copper].X1 := 177;
          BuyOptionRects[Copper].Y1 := 2;
          BuyOptionRects[Copper].X2 := 236;
          BuyOptionRects[Copper].Y2 := 18;
          BuyOptionRects[Olives].X1 := 8;
          BuyOptionRects[Olives].Y1 := 2;
          BuyOptionRects[Olives].X2 := 67;
          BuyOptionRects[Olives].Y2 := 18;
          BuyOptionRects[Wheat].X1 := 179;
          BuyOptionRects[Wheat].Y1 := 86;
          BuyOptionRects[Wheat].X2 := 236;
          BuyOptionRects[Wheat].Y2 := 101;
        end;
        Egypt: begin
          PortPutPic(BuyEgyptWindow^, 1, 142);
          BuyOptionRects[Copper].X1 := 177;
          BuyOptionRects[Copper].Y1 := 2;
          BuyOptionRects[Copper].X2 := 236;
          BuyOptionRects[Copper].Y2 := 18;
          BuyOptionRects[Olives].X1 := 177;
          BuyOptionRects[Olives].Y1 := 33;
          BuyOptionRects[Olives].X2 := 236;
          BuyOptionRects[Olives].Y2 := 49;
          BuyOptionRects[Wheat].X1 := 179;
          BuyOptionRects[Wheat].Y1 := 86;
          BuyOptionRects[Wheat].X2 := 236;
          BuyOptionRects[Wheat].Y2 := 101;
        end;
      end;
      ChosenWare := ShowOptions(BuyOptionRects, 103);
      if ChosenWare > 0 then
      begin
        case ChosenWare of
          Copper: ChosenWarePrice := CopperPrice;
          Olives: ChosenWarePrice := OlivesPrice;
          Wheat: ChosenWarePrice := WheatPrice;
        end;
        if Money < ChosenWarePrice then
          ShowMessage('W', 'äå òôîÄé ÖêâÄ äîëã', WareName[ChosenWare] + 'öÖêóå', '')
        else
        begin
          repeat
          begin
            PortPutPic(BuySign^, 22, 138);
            PortGotoXY(6, 15);
            if Money / ChosenWarePrice > MaxSafeInteger then
              Write(MaxSafeInteger)
            else
              Write(Trunc(Money / ChosenWarePrice));
            { Port upgrade: quick-amount keys.
              A = everything you can afford, B = only what still fits the ship,
              C = everything while keeping a 10% (min 500) reserve for guard ships }
            QuickAmountA := Int(Money / ChosenWarePrice);
            if QuickAmountA > MaxSafeInteger then
              QuickAmountA := MaxSafeInteger;
            QuickAmountB := ShipCapacity - GetTotalCargoWeight;
            if QuickAmountB > QuickAmountA then
              QuickAmountB := QuickAmountA;
            if QuickAmountB < 0 then
              QuickAmountB := 0;
            QuickAmountC := Int(Money / 10);
            if QuickAmountC < 500 then
              QuickAmountC := 500;
            QuickAmountC := Int((Money - QuickAmountC) / ChosenWarePrice);
            if QuickAmountC > MaxSafeInteger then
              QuickAmountC := MaxSafeInteger;
            if QuickAmountC < 0 then
              QuickAmountC := 0;
            PortGotoXY(6, 18);
            Write('òéôé=C öåÖÅâó=B åãÑ=A');
            PortGotoXY(6, 17);
            AmountToBuy := InputReal(0, MaxSafeInteger, 5);
            QuickAmountA := 0;
            QuickAmountB := 0;
            QuickAmountC := 0;
            TotalCost := 1.0 * AmountToBuy * ChosenWarePrice;
          end
          until not CannotAfford(TotalCost);
        end;
        if AmountToBuy > 0 then
        begin
          Money := Money - TotalCost;
          Cargo[ChosenWare] := Cargo[ChosenWare] + Trunc(AmountToBuy);
        end;
      end;
    end;
  end;

  procedure ShowSellWindow(CopperPrice, OlivesPrice, WheatPrice: integer);
  var
    Unused: integer;
    ChosenWare: integer;
    Revenue: real;
    AmountToSell: real;
    SellOptionRects: TRectArray;

    function HasInsufficientCargo(Amount: real; Ware: integer): boolean;
    begin
      if Amount > Cargo[Ware] then
      begin
        ShowMessage('W', 'ÑÅòÑ äã-åã äå èâÄ', 'òÖãéå' + WareName[Ware], Bell);
        HasInsufficientCargo := True;
      end
      else
        HasInsufficientCargo := False;
    end;

  begin
    if (Cargo[Copper] = 0) and (Cargo[Olives] = 0) and (Cargo[Wheat] = 0) then
      ShowMessage('W', ' ', 'òÖãéå  ÑòÖáë  äå  èâÄ', ' ')
    else
    begin
      case CountryNameToIndex(CurrentCountryName) of
        Turkey: PortPutPic(SellTurkeyWindow^, 0, 142);
        Israel: PortPutPic(SellIsraelWindow^, 0, 142);
        Egypt: PortPutPic(SellEgyptWindow^, 0, 142);
      end;
      SellOptionRects[Copper].X1 := 19;
      SellOptionRects[Copper].Y1 := 26;
      SellOptionRects[Olives].X1 := 19;
      SellOptionRects[Olives].Y1 := 42;
      SellOptionRects[Wheat].X1 := 19;
      SellOptionRects[Wheat].Y1 := 58;
      if CurrentCountryName = EgyptName then
      begin
        SellOptionRects[Copper].X2 := 58;
        SellOptionRects[Copper].Y2 := 34;
        SellOptionRects[Olives].X2 := 57;
        SellOptionRects[Olives].Y2 := 50;
        SellOptionRects[Wheat].X2 := 57;
        SellOptionRects[Wheat].Y2 := 66;
      end
      else
      begin
        SellOptionRects[Copper].X2 := 59;
        SellOptionRects[Copper].Y2 := 35;
        SellOptionRects[Olives].X2 := 59;
        SellOptionRects[Olives].Y2 := 51;
        SellOptionRects[Wheat].X2 := 59;
        SellOptionRects[Wheat].Y2 := 67;
      end;
      ChosenWare := ShowOptions(SellOptionRects, 103);
      if ChosenWare > 0 then
      begin
        if Cargo[ChosenWare] > 0 then
        begin
          repeat
          begin
            PortPutPic(SellSign^, 120, 60);
            { Port upgrade: '=' sells the whole stock of the chosen ware }
            QuickAmountA := Cargo[ChosenWare];
            PortGotoXY(20, 8);
            Write('åãÑ = =');
            PortGotoXY(21, 7);
            AmountToSell := InputReal(0, MaxSafeInteger, 5);
            QuickAmountA := 0;
          end
          until not HasInsufficientCargo(AmountToSell, ChosenWare);
        end
        else
        begin
          ShowMessage('W', 'òÖãéå' + WareName[ChosenWare] + 'äå èâÄ', Bell, '');
          AmountToSell := 0;
        end;
        if AmountToSell = -1 then
          AmountToSell := 0;

        case ChosenWare of
          Copper: Revenue := 1.0 * AmountToSell * CopperPrice;
          Olives: Revenue := 1.0 * AmountToSell * OlivesPrice;
          Wheat: Revenue := 1.0 * AmountToSell * WheatPrice;
        end;
        Money := Money + Revenue;
        Cargo[ChosenWare] := Cargo[ChosenWare] - Trunc(AmountToSell);
      end;
    end;
  end;

  procedure ShowBankWindow;
  var
    ChosenOption: integer;
    DidFinishFlag: byte;
    BankOptionRects: TRectArray;
    Amount: real;

  { Options in bank menu }
  const
    Deposit = 1;
    Withdraw = 2;

  begin
    DidFinishFlag := 0;
    PortPutPic(BankWindow^, 0, 142);
    BankOptionRects[Deposit].X1 := 97;
    BankOptionRects[Deposit].Y1 := 7;
    BankOptionRects[Deposit].X2 := 149;
    BankOptionRects[Deposit].Y2 := 16;
    BankOptionRects[Withdraw].X1 := 16;
    BankOptionRects[Withdraw].Y1 := 7;
    BankOptionRects[Withdraw].X2 := 64;
    BankOptionRects[Withdraw].Y2 := 16;
    ChosenOption := ShowOptions(BankOptionRects, 102);
    case ChosenOption of
      Deposit: begin
        if Money <= 0 then
          ShowMessage('W', ' ', '! ÉâóîÑå ìëã äå èâÄ' + Bell, ' ')
        else
        begin
          repeat
          begin
            PortPutPic(DepositSign^, 11, 139);
            { Port upgrade: '=' deposits everything }
            QuickAmountA := Int(Money);
            PortGotoXY(16, 17);
            Write('åãÑ = =');
            PortGotoXY(5, 17);
            Amount := InputReal(0, 0, 8);
            QuickAmountA := 0;
            if Amount <= 0 then
              DidFinishFlag := 1
            else if Amount <= Money then
            begin
              BankBalance := BankBalance + Amount;
              Money := Money - Amount;
              ShowMessage('W', ' ', 'ìëãÑ öÄ åÅâó óêÅÑ Éâóî', ' ');
              DidFinishFlag := 1;
            end
            else
              ShowMessage('W', ' ', '! ìëã ÑÅòÑ äã åã äå èâÄ' + Bell, ' ');
          end
          until DidFinishFlag = 1;
        end;
      end;
      Withdraw: begin
        if BankBalance > 0 then
        begin
          repeat
          begin
            PortPutPic(WithdrawSign^, 11, 139);
            { Port upgrade: '=' withdraws everything }
            QuickAmountA := Int(BankBalance);
            PortGotoXY(16, 17);
            Write('åãÑ = =');
            PortGotoXY(5, 17);
            Amount := InputReal(0, 0, 8);
            QuickAmountA := 0;
            if Amount < 0 then
              DidFinishFlag := 1
            else if Amount <= BankBalance then
            begin
              PortGotoXY(1, 1);
              BankBalance := BankBalance - Amount;
              Money := Money + Amount;
              if not (Amount = 0) then
                ShowMessage('W', ' ', 'ìëãÑ öÄ öåÅâó', ' ');
              DidFinishFlag := 1;
            end
            else
              ShowMessage('W', ' ', '! óêÅÅ ÑÜã ìëã çÖãë äå èâÄ' + Bell, ' ');
          end
          until DidFinishFlag = 1;
        end
        else
          ShowMessage('W', ' ', '! óêÅÅ ìëã äå èâÄ' + Bell, ' ');
      end;
    end;
  end;

  function AskOverweight(TotalCargoWeight: integer): boolean;
  var
    Unused: array[1..3] of byte;
  begin
    if TotalCargoWeight > ShipCapacity then
    begin
      PortPutPic(OverweightWindow^, 0, 142);
      PortGotoXY(53, 16);
      if InputYesNo then
        AskOverweight := False
      else
        AskOverweight := True;
    end
    else
      AskOverweight := False;
  end;

  procedure ToggleCountryHighlight(CountryName: TCountryName);
  var
    CountryNamePic: PWinBuffer;
  begin
    case CountryNameToIndex(CountryName) of
      Turkey: begin
        CountryNameX1 := 256;
        CountryNameX2 := 303;
      end;
      Israel: begin
        CountryNameX1 := 147;
        CountryNameX2 := 190;
      end;
      Egypt: begin
        CountryNameX1 := 44;
        CountryNameX2 := 86;
      end;
    end;
    New(CountryNamePic);
    PortGetPic(CountryNamePic^, CountryNameX1, 146, CountryNameX2, 155);
    PortColorTable(3, 2, 1, 0);
    PortPutPic(CountryNamePic^, CountryNameX1, 155);
    PortColorTable(0, 1, 2, 3);
    Dispose(CountryNamePic);
  end;

  procedure TravelFlow(StormCountryNameArg: TCountryName);
  var
    ShouldAbortTravel: boolean;
    DidArriveSafely: boolean;
    TravelDuration: integer;
    ChosenDestination: integer;
    Unused: byte;
    FullTravelOptionRects: TRectArray;
    TravelOptionRects: TRectArray;
    ShipX: integer;
    ShipY: integer;
    { Negative number means travel was aborted }
    GuardShipCount: integer;
    TravelEndTime: byte;

    procedure AnimateTravel(FromX, FromY, ToX, ToY: integer; Unused: integer);
    var
      X: integer;
      Y: integer;
      ShipPic: array[1..129] of byte;
      Unused2: array[1..129] of byte;
      I: integer;
      TotalMinutes: integer;
      Minute: integer;
    begin
      PortGotoXY(34, 17);
      if Money > 10000.0 then
        PortGotoXY(33, 17);

      if Money > 100000.0 then
        PortGotoXY(32, 17);

      Write(ToStrWithCommas(Money):5);
      PortPutPic(MapWindow^, 0, 142);
      PortGetPic(ShipPic, FromX, FromY, FromX + 18, FromY + 14);
      Minute := 0;
      TotalMinutes := (TravelEndTime - TimeOfDay) * 60;
      X := FromX;
      Y := FromY + 14;
      for I := 1 to TotalMinutes do
      begin
        Minute := Minute + 1;
        if Minute < 60 then
        begin
          PortGotoXY(35, 5);
          if Minute < 10 then
            Write(0);
          if Minute mod 15 = 0 then
            Write(Minute);
        end
        else
        begin
          Minute := 0;
          TimeOfDay := TimeOfDay + 1;
          PortGotoXY(32, 5);
          Write(TimeOfDay:2, ':00');
        end;
        ShipX := Round(0.0 + FromX + I / TotalMinutes * (ToX - FromX));
        ShipY := Round(0.0 + FromY + I / TotalMinutes * (ToY - FromY) + 14);
        PortPutPic(ShipPic, ShipX, ShipY);
        PortDelay(15);
        X := ShipX;
        Y := ShipY;
        ShipY := ShipY - 14;
      end;
      PortGetPic(MapWindow^, 0, 0, 240, 142);
    end;

    procedure DoTravelEvent(ShipDamageArg: real; CargoWeightArg: integer;
                            TravelDurationArg: integer; RiskArg: integer);
    var
      Unused: array[1..4] of byte;
      Ware: integer;
      RandomEvent: integer;
      FoundAmount: integer;
      LostAmountString: TNumberString;
      OriginalDestination: TCountryName;
      OriginalTravelDestination: byte;
      CurrentShipX: integer;
      CurrentShipY: integer;
      Unused2: real;
      IsHighRiskEventForced: boolean;
      IsShipDamaged: boolean;

      procedure PiratesFlow(GuardShipCountArg: integer; CargoWeightNestedArg: integer);
      var
        RunAwayChance: integer;
        ChosenPiratesOption: integer;
        { Running away succeeds if this equals 0 }
        RunAwayControl: integer;
        { Running away fails if this is bigger than 0 }
        DamagedRunAwayControl: integer;
        GuardShipCountAdjustment: integer;
        StringBuffer: string[5];
        PiratesOptionRects: TRectArray;
        Booty: real;
        LostAmount: real;

        procedure ApplyConfrontDamage(DamageChance: integer);
        begin
          if Tp3Random(DamageChance) = 0 then
          begin
            LostAmount := 10 * Trunc((Tp3Random(15) + 5) / 1000.0 * (Money + GetTotalCargoValue));
            Str(LostAmount:1:0, StringBuffer);
            ShipDamage := ShipDamage + LostAmount;
            if DamageChance = 2 then
              ShowMessage('W', 'çòÇê ,ìëÖêÅ', 'âÖÖôÅ óÜê Ñêâîëå', '.çâåóô ' + StringBuffer)
            else
              ShowMessage('W', 'äöêâîë ,äåÜé íÖòå', '.ÅòóÑ âÉã äÖö ÑíÇîê',
                  '.á"ô ' + StringBuffer + ' óÜêÑ âÖÖô');
          end;
        end;

        procedure ConfrontPirates;
        var
          WinningDifficulty: integer;
          MostValuableCargoValue: real;
          Unused: byte;
        begin
          PortPutPic(ConfrontWindow^, 0, 142);
          WinningDifficulty := Tp3Random(4);
          if (WinningDifficulty < GuardShipCountArg - GuardShipCountAdjustment) or
              ((GuardShipCountArg - GuardShipCountAdjustment <= 0) and (Tp3Random(6) = 1)) then
          begin
            ShowMessage('W', '- äôÖéé Åòó òáÄå', '! çâàòâîÑ öÄ öëÅÑ', '');
            if Tp3Random(3) = 1 then
            begin
              ShipCapacity := ShipCapacity + 50;
              ShowMessage('W', 'öêâîë öÄ âÅôÅ öáóå', 'ÑêâîëÑ öåÖÅâóÖ ,çâÑ âÉÉÖô',
                  '.èÖà 50-Å ÑåÉÇ äåô');
            end
            else
            begin
              GuardShipCountArg := GuardShipCountArg + GuardShipCountAdjustment;
              Booty := Int((Money + GetTotalCargoValue + BankBalance) / 100) * (Tp3Random(15) + 5) +
                  GuardShipCountArg * (Money + GetTotalCargoValue + BankBalance) / 10;
              if Booty < 100 then
                Booty := 100;
              Booty := 5 * Int(Booty / 5);
              Str(Booty:1:0, StringBuffer);
              ShowMessage('W', ' åô èÖéàé öÄñé', 'çöêâîë åí åóô ' + StringBuffer, '');
              Money := Money + Booty;
            end;
            ApplyConfrontDamage(4);
          end
          else
          begin
            Str(GuardShipCountArg, StringBuffer);
            if GuardShipCountArg = 0 then
              ShowMessage('W', ',òéôé öÖêâîë öêÇÑ Äåå', 'çâàòâîÑ âÉâ åí öëÅÖÑ ÑöÄ', '')
            else if GuardShipCountArg = 1 then
              ShowMessage('W', 'öêâîë åí ÖòÅÇöÑ çâàòâîÑ', 'äåô òéôéÑ', '')
            else if GuardShipCountArg > 1 then
              ShowMessage('W', 'öÄ Öáñâê çâàòâîÑ', 'äåô òéôéÑ öÖêâîë ' + StringBuffer, '');
            GuardShipCountArg := GuardShipCountArg + GuardShipCountAdjustment;
            if GuardShipCountArg > 3 then
              GuardShipCountArg := 3;
            GetMostValuableCargo(5, Ware, MostValuableCargoValue);
            if MostValuableCargoValue > Money then
            begin
              LostAmount := 1.0 * (Tp3Random(15) + 5) * Cargo[Ware] / 100;
              if GuardShipCountArg < 3 then
                LostAmount := LostAmount + (3 - GuardShipCountArg) * Cargo[Ware] div 10;
              if LostAmount < 1 then
                LostAmount := 1;
              Cargo[Ware] := Cargo[Ware] - Trunc(LostAmount);
              Str(Trunc(LostAmount):1, LostAmountString);
              ShowMessage('W', '     Ñôó Åòó âòáÄ', ' çâÑ âÉÉÖô äéé ÖÅêÇ',
                  WareName[Ware] + 'èÖà ' + LostAmountString);
              ApplyConfrontDamage(2);
            end
            else if Money > 500 then
            begin
              LostAmount := 10 * Trunc(Tp3Random(Trunc(0.015 * Money)) + Money / 200);
              if GuardShipCountArg < 3 then
                LostAmount := LostAmount + (3 - GuardShipCountArg) * Trunc(Money / 10);
              LostAmount := Trunc(LostAmount);
              Money := Money - LostAmount;
              Str(LostAmount:1:0, LostAmountString);
              ShowMessage('W', 'çâåóô ' + LostAmountString + ' äéé ÖÅêÇ çÑ', '', '');
              ApplyConfrontDamage(2);
            end
            else
            begin
              LostAmount := Int((Tp3Random(15) + 35 - 10 * GuardShipCountArg) / 1000 *
                  (GetTotalCargoValue + Money + BankBalance)) * 10;
              if LostAmount < 100 then
                LostAmount := 100;
              Str(LostAmount:1:0, StringBuffer);
              ShipDamage := ShipDamage + LostAmount;
              ShowMessage('W', 'äöêâîë ,äåÜé íÖòå', '.ÅòóÑ âÉã äÖö ÑíÇîê',
                  'á"ô ' + StringBuffer + ' óÜêÑ âÖÖô');
            end;
          end;
          ShowHUD;
        end;

      { Options in pirates menu }
      const
        Confront = 1;
        RunAway = 2;
        Offer = 3;

      begin
        PortPutPic(PiratesWindow^, 0, 142);
        WaitForKey;
        PortPutPic(PiratesSign^, 4, 139);
        PortGotoXY(18, 17);
        Write(GuardShipCount:2);
        PiratesOptionRects[Confront].X1 := 108;
        PiratesOptionRects[Confront].Y1 := 117;
        PiratesOptionRects[Confront].X2 := 133;
        PiratesOptionRects[Confront].Y2 := 123;
        PiratesOptionRects[RunAway].X1 := 72;
        PiratesOptionRects[RunAway].Y1 := 117;
        PiratesOptionRects[RunAway].X2 := 96;
        PiratesOptionRects[RunAway].Y2 := 123;
        PiratesOptionRects[Offer].X1 := 7;
        PiratesOptionRects[Offer].Y1 := 117;
        PiratesOptionRects[Offer].X2 := 58;
        PiratesOptionRects[Offer].Y2 := 123;
        ChosenPiratesOption := ShowOptions(PiratesOptionRects, 3);
        case ChosenPiratesOption of
          Confront: begin
            GuardShipCountAdjustment := 0;
            ConfrontPirates;
          end;
          RunAway: begin
            DamagedRunAwayControl := 0;
            GuardShipCountAdjustment := 1;
            PortPutPic(RunAwayWindow^, 0, 142);
            if CargoWeightNestedArg > 0.81 * ShipCapacity then
              RunAwayControl := 0
            else
            begin
              RunAwayChance := ShipCapacity div 30 - CargoWeightNestedArg div 30 + 1;
              if RunAwayChance < 1 then
                RunAwayChance := 1;
              RunAwayControl := Tp3Random(RunAwayChance);
            end;
            if RunAwayControl > 0 then
              DamagedRunAwayControl := Tp3Random(Trunc(ShipDamage) div 30 + 1);
            if DamagedRunAwayControl > 0 then
              RunAwayControl := 0;
            if RunAwayControl = 0 then
            begin
              if CargoWeightNestedArg > 0.5 * ShipCapacity then
                ShowMessage('W', 'ÄåÖ ,âÉé ÑÉÅã äöêâîë', 'áÖòÅå ÑáâåñÑ', '')
              else if ShipDamage > 0 then
                ShowMessage('W', 'Åóí áÖòÅå öáåñÑ Äå', 'äöêâîëÅô óÜêÑ', '')
              else
                ShowMessage('W', 'ÑáâåñÑ ÑòâÑéÑ çâàòâîÑ öêâîë', 'äöêâîë öÄ ÇâôÑå', '');
              ConfrontPirates;
            end
            else
              ShowMessage('W', '', '! !  çÑé  àåéÑå  öáåñÑ', '');
          end;
          Offer: begin
            GuardShipCountAdjustment := 1;
            PortPutPic(OfferWindow^, 0, 142);
            if OfferFlow(Cargo[Copper], Cargo[Olives], Cargo[Wheat], Money) then
            begin
              ShowMessage('W', 'åÅóå ÖàâåáÑ çâÑ âÉÉÖô', 'äöíñÑ öÄ', '');
              ShowHUD;
            end
            else
            begin
              ShowMessage('W', 'öÄ çâáÖÉ çâÑ âÉÉÖô', '! äöÖÄ çâîóÖöÖ äöíñÑ', '');
              ConfrontPirates;
            end;
          end;
        end;
      end;

      function IsShipSinking(CargoWeightNestedArg: integer): boolean;
      var
        SinkingChance: integer;
      begin
        SinkingChance := 30000;
        if CargoWeightNestedArg > ShipCapacity + 70 then
          SinkingChance := 1
        else if CargoWeightNestedArg > ShipCapacity + 60 then
          SinkingChance := 2
        else if CargoWeightNestedArg > ShipCapacity + 40 then
          SinkingChance := 3
        else if CargoWeightNestedArg > ShipCapacity + 20 then
          SinkingChance := 4
        else if CargoWeightNestedArg > ShipCapacity then
          SinkingChance := 6;
        if Tp3Random(SinkingChance) = 0 then
          IsShipSinking := True
        else
          IsShipSinking := False;
      end;

      function ShouldDoHighRiskEvent(ShipDamageNestedArg: real; RiskNestedArg: integer): boolean;
      begin
        ShouldDoHighRiskEvent :=
            (Tp3Random(1 + Trunc((ShipDamageNestedArg + 399) / 400) + 2 * RiskNestedArg) > 0) and
            (Tp3Random(2) = 1);
      end;

      procedure ChangeTravelDestination;
      begin
        PortPutPic(ChangeWindow^, 0, 142);
        OriginalDestination := TravelDestination;
        if TravelDestination = IsraelName then
          TravelDestination := TurkeyName
        else
          TravelDestination := IsraelName;
        PortDelay(1500);
        ShowMessage('W', 'öÖóÜá öÖáÖòé ÑÄñÖöã', 'öíÇÑ ,àÖÖêÅ öÖíàÖ',
            OriginalDestination + 'å çÖóéÅ ' + TravelDestination + 'å');
        if (TravelStart = EgyptName) and (TravelDestination = TurkeyName) then
          TravelEndTime := TravelEndTime + 4;
      end;

    begin
      HadTravelEvent := False;
      DidArriveSafely := False;
      if ShouldDoHighRiskEvent(ShipDamageArg, RiskArg) then
      begin
        if ShipDamageArg > 0 then
          IsShipDamaged := True
        else
          IsHighRiskEventForced := Tp3Random(3) = 1;
        { IsShipDamaged might be uninitalized here. If it's a day travel, the value comes from
          TravelOptionRects and is either 182 if you're in Egypt/Israel or 237 if you're in turkey.
          If it's a night travel, the value is 0, due to reading the input in the Night yes/no
          screen, at least on DOSBox }
        if IsShipDamaged or IsHighRiskEventForced then
        begin
          ShowMessage('W', '', 'Ñ ò ñ í ê   Ñ ê â î ë Ñ', '');
          if (StormCountryName = CurrentCountryName) or (StormCountryName = TravelDestination) then
          begin
            PortPutPic(StormWindow^, 0, 142);
            WaitForKey;
            if GetTotalCargoValue > 10000 then
            begin
              ApplyBadOutcome(LoseCargo, Ware, LostAmountString);
              ShowMessage('W', 'äöêâîëé Öîàô çâÑÖÅÇÑ çâåÇÑ',
                  '.' + WareName[Ware] + ' èÖà ' + LostAmountString, '');
              ShowHUD;
            end
            else
            begin
              ApplyBadOutcome(AddDamage, Ware, LostAmountString);
              ShowMessage('W', 'äöêâîëå çòÇê ÑòíëÑ Åóí', '.á"ô ' + LostAmountString + ' âÖÖôÅ óÜê',
                  '');
            end;
          end
          else
          begin
            ApplyBadOutcome(AddDamage, Ware, LostAmountString);
            if TimeOfDay > 16 then
            begin
              PortPutPic(ShoalWindow^, 0, 142);
              ShowMessage('W', 'çâíÖòÇÑ öÖÄòÑ âÄêö Åóí', '.èÖàòô åí öâåí - ÑåâåÅ', '');
              ShowMessage('W', 'óÜê äöêâîëå çòÇê', '.á"ô ' + LostAmountString + ' âÖÖôÅ', '');
            end
            else
            begin
              PortPutPic(DamagedWindow^, 0, 142);
              if IsShipDamaged then
                ShowMessage('W', 'ÑóÖÜâêÑ äöêâîëÅ ÑíâëêÑ', LostAmountString + '-Å óÜêÑ öÄ ÑòâéáÑ',
                    '.çâîëÖê åóô')
              else
                ShowMessage('W', 'óÜê äöêâîëå çòÇê', '.á"ô ' + LostAmountString + ' âÖÖôÅ', '');
            end;
          end;
        end
        else if AllowNavigationError then
          ChangeTravelDestination;
        HadTravelEvent := True;
      end;
      if (not HadTravelEvent) and (CargoWeightArg > ShipCapacity) then
      begin
        if IsShipSinking(CargoWeightArg) then
        begin
          ShowMessage('W', '', 'Ñ ò ñ í ê   Ñ ê â î ë Ñ', '');
          PortPutPic(SinkingWindow^, 0, 142);
          WaitForKey;
          ApplyBadOutcome(SinkingShip, Ware, LostAmountString);
          ShowMessage('W', 'çâå äâåôÑå öñåÄê', WareName[Ware] + 'èÖà ' + LostAmountString, '');
          ShowHUD;
          HadTravelEvent := True;
        end;
      end;
      if (not HadTravelEvent) and
          (TimeOfDay + TravelDurationArg + TravelDurationArg div 2 <= 20) then
      begin
        if AllowClosedPorts and (Tp3Random(12) = 0) then
        begin
          CurrentShipX := ShipX;
          CurrentShipY := ShipY;
          OriginalTravelDestination := CountryNameToIndex(TravelDestination);
          AnimateTravel(CurrentShipX, CurrentShipY, MapCoordinates[OriginalTravelDestination].X,
              MapCoordinates[OriginalTravelDestination].Y, 9);
          if Tp3Random(2) = 1 then
            ShowMessage('W', 'öâêéÜ òÖÇë ' + TravelDestination + ' åéê', 'ÉÅã åîòí Åóí', '')
          else
          begin
            PortPutPic(StrikeWindow^, 0, 142);
            WaitForKey;
            ShowMessage('W', 'çâôÖÅâôÖ ÑöâÅô åôÅ', TravelDestination + ' åéê ,ÑÉÖÅíÅ', 'òÖÇë ');
          end;
          ShowMessage('W', '', (TravelStart + 'å òÖÜáå äâåí'), '');
          TravelDestination := TravelStart;
          TravelEndTime := TimeOfDay + TravelDurationArg div 2;
          AnimateTravel(MapCoordinates[OriginalTravelDestination].X,
              MapCoordinates[OriginalTravelDestination].Y, CurrentShipX, CurrentShipY, 9);
          TravelEndTime := TimeOfDay + TravelDurationArg div 2;
          HadTravelEvent := True;
        end;
      end;
      RandomEvent := Tp3Random(20) + 1;
      if TimeOfDay >= 16 then
        if RandomEvent > 15 then
          RandomEvent := 1;
      if not HadTravelEvent then
      begin
        case RandomEvent of
          1..6: begin
            ShowMessage('W', '', 'Ñ ò ñ í ê   Ñ ê â î ë Ñ', '');
            HadTravelEvent := True;
            PiratesFlow(GuardShipCount, CargoWeightArg);
          end;
          7: if AllowNavigationError then
            ChangeTravelDestination;
          9, 10: begin
            ShowMessage('W', '', 'Ñ ò ñ í ê   Ñ ê â î ë Ñ', '');
            PortPutPic(DesertedWindow^, 0, 142);
            Ware := 0;
            repeat
            begin
              Ware := Ware + 1;
              FoundAmount := Trunc((Tp3Random(25) + 11) / 100.0 * (Money + GetTotalCargoValue) /
                  BaseWarePrice[Ware]);
              if FoundAmount > 0 then
              begin
                PortGotoXY(45, 16);
                Write(WareName[Ware], ' èÖà ', FoundAmount);
                case Ware of
                  Copper: Cargo[Copper] := Cargo[Copper] + FoundAmount;
                  Olives: Cargo[Olives] := Cargo[Olives] + FoundAmount;
                  Wheat: Cargo[Wheat] := Cargo[Wheat] + FoundAmount;
                end;
                HadTravelEvent := True;
              end;
            end
            until HadTravelEvent or (Ware = Wheat);
            WaitForKey;
            ShowHUD;
          end;
          8, 11..20: DidArriveSafely := True;
        end;
      end;
    end;

    procedure DoTravel;
    var
      CurrentWaypoint: byte;
      DestinationIndex: byte;
      NextWaypoint: byte;
      HalfTravelTime: byte;

      { Risk > 0 means 1/3 higher guard ship cost }
      procedure ShowGuardShipWindow(var RiskArg: integer);
      var
        GuardShipPrice: real;
        Line1: TMessageLine;
        Line2: TMessageLine;
        Line3: TMessageLine;
        PriceString: TNumberString;
      begin
        PortPutPic(GuardsWindow^, 0, 142);
        if FixedGuardShipPrice then
          GuardShipPrice := 75
        else
        begin
          GuardShipPrice := 5 * Trunc((GetTotalCargoValue + Money) / (200 + Tp3Random(100)));
          if GuardShipPrice < 75 then
            GuardShipPrice := 75;
          if RiskArg > 0 then
            GuardShipPrice := GuardShipPrice + GuardShipPrice / 3;
          GuardShipPrice := 5 * Int(GuardShipPrice / 5);
        end;
        Str(GuardShipPrice:1:0, PriceString);
        PortGotoXY(1, 15);
        Line3 := '? ÑñÖò ÑöÄ öÖêâîë Ñéã';
        if GuardShipPrice <= Money then
        begin
          Line2 := 'á"ô ' + PriceString;
          if RiskArg = 0 then
            Line1 := 'òéôé öêâîë åã òâáé'
          else
          begin
            Line1 := 'çâôòÖÉ òéôéÑ öÖêâîë âåíÅ';
            if (StormCountryName = CurrentCountryName) or
                (StormCountryName = TravelDestination) then
              Line2 := '.òíÖëÑ çâÑ åôÅ ,' + Line2
            else
              Line2 := 'òÅÇÖéÑ èÖãâëÑ åôÅ ' + Line2;
          end;
          ShowMessage('N', Line1, Line2, Line3);
          repeat
          begin
            PortGotoXY(4, 17);
            Write('':4);
            PortGotoXY(4, 17);
            GuardShipCount := Trunc(InputReal(0, 999, 3));
          end
          until not CannotAfford(1.0 * GuardShipPrice * GuardShipCount);
        end
        else
        begin
          ShowMessage('N', 'çåôå ìëã äå èâÄ', 'òéôé öÖêâîëå', '');
          GuardShipCount := 0;
          repeat
            LastPressedKey := GetPressedKey;
          until LastPressedKey in [Enter, Escape];
          if LastPressedKey = Escape then
            GuardShipCount := -1;
        end;
        if GuardShipCount > 0 then
          Money := Money - GuardShipCount * GuardShipPrice;
      end;

    begin
      ShowGuardShipWindow(Risk);
      if GuardShipCount < 0 then
        TravelDestination := TravelStart
      else
      begin
        HalfTravelTime := (TravelEndTime - TimeOfDay) div 2;
        TravelEndTime := TravelEndTime - HalfTravelTime;
        CurrentWaypoint := CountryNameToIndex(TravelStart);
        DestinationIndex := CountryNameToIndex(TravelDestination);
        NextWaypoint := MapWaypoints[CurrentWaypoint, DestinationIndex];
        AnimateTravel(MapCoordinates[CurrentWaypoint].X, MapCoordinates[CurrentWaypoint].Y,
          MapCoordinates[NextWaypoint].X, MapCoordinates[NextWaypoint].Y, 9);
        TravelEndTime := TimeOfDay + HalfTravelTime;
        CurrentWaypoint := NextWaypoint;
        DoTravelEvent(ShipDamage, GetTotalCargoWeight, TravelDuration, Risk);
        DestinationIndex := CountryNameToIndex(TravelDestination);
        NextWaypoint := MapWaypoints[CurrentWaypoint, DestinationIndex];
        if NextWaypoint > 0 then
        begin
          HalfTravelTime := HalfTravelTime div 2;
          TravelEndTime := TravelEndTime - HalfTravelTime;
        end;
        while NextWaypoint > 0 do
        begin
          AnimateTravel(MapCoordinates[CurrentWaypoint].X, MapCoordinates[CurrentWaypoint].Y,
              MapCoordinates[NextWaypoint].X, MapCoordinates[NextWaypoint].Y, 9);
          TravelEndTime := TimeOfDay + HalfTravelTime;
          CurrentWaypoint := NextWaypoint;
          NextWaypoint := MapWaypoints[CurrentWaypoint, DestinationIndex];
        end;
        AnimateTravel(MapCoordinates[CurrentWaypoint].X, MapCoordinates[CurrentWaypoint].Y,
            MapCoordinates[DestinationIndex].X, MapCoordinates[DestinationIndex].Y, 9);
        if DidArriveSafely then
          ShowMessage('N', '', '! !  ç Ö å ô Å   ö í Ç Ñ', '');
      end;
    end;

  begin
    TravelDuration := 0;
    TravelStart := CurrentCountryName;
    ShouldAbortTravel := False;
    Risk := 0;
    PortPutPic(MapWindow^, 0, 142);
    FullTravelOptionRects[Turkey].X1 := 144;
    FullTravelOptionRects[Turkey].Y1 := 2;
    FullTravelOptionRects[Turkey].X2 := 182;
    FullTravelOptionRects[Turkey].Y2 := 9;
    FullTravelOptionRects[Israel].X1 := 228;
    FullTravelOptionRects[Israel].Y1 := 106;
    FullTravelOptionRects[Israel].X2 := 237;
    FullTravelOptionRects[Israel].Y2 := 137;
    FullTravelOptionRects[Egypt].X1 := 75;
    FullTravelOptionRects[Egypt].Y1 := 134;
    FullTravelOptionRects[Egypt].X2 := 111;
    FullTravelOptionRects[Egypt].Y2 := 141;
    Iter := Turkey;
    for CountryIter := Turkey to Egypt do
    begin
      if CountryIter <> CountryNameToIndex(TravelStart) then
      begin
        TravelOptionRects[Iter] := FullTravelOptionRects[CountryIter];
        Iter := Iter + 1;
      end;
    end;
    case CountryNameToIndex(CurrentCountryName) of
      Turkey: ChosenDestination := ShowOptions(TravelOptionRects, 2) + 1;
      Israel: ChosenDestination := ShowOptions(TravelOptionRects, 2) * 2 - 1;
      Egypt: ChosenDestination := ShowOptions(TravelOptionRects, 2);
    end;
    if ChosenDestination > 0 then
    begin
      TravelDestination := CountryIndexToName(ChosenDestination);
      if (TravelStart = IsraelName) or (TravelDestination = IsraelName) then
        TravelDuration := 4
      else
        TravelDuration := 8;
      if (not ShouldAbortTravel) and (TimeOfDay + TravelDuration > 20) then
      begin
        ShowMessage('W', 'öòáÖÄé ÑíôÑ', 'íâÇÑå åãÖö ÄåÖ', 'çÖâÑ çôå');
        TravelDestination := TravelStart;
        ShouldAbortTravel := True;
      end;
      if not ShouldAbortTravel then
      begin
        if TimeOfDay + TravelDuration > 16 then
        begin
          PortPutPic(NightWindow^, 0, 142);
          PortGotoXY(4, 17);
          if not InputYesNo then
            GuardShipCount := -1
          else
          begin
            TravelEndTime := TimeOfDay + TravelDuration;
            Risk := 2;
            if (StormCountryNameArg = CurrentCountryName) or
                (StormCountryNameArg = TravelDestination) then
              Risk := 4;
            DoTravel;
          end;
        end
        else
        begin
          if (StormCountryNameArg = CurrentCountryName) or
              (StormCountryNameArg = TravelDestination) then
            Risk := 3;
          TravelEndTime := TimeOfDay + TravelDuration;
          DoTravel;
        end;
      end;
      if GuardShipCount < 0 then
        TravelDestination := TravelStart;
      CurrentCountryName := TravelDestination;
      if GuardShipCount >= 0 then
      begin
        PortGotoXY(73, 1);
        Write(CurrentCountryName:6);
        ToggleCountryHighlight(TravelStart);
        ToggleCountryHighlight(CurrentCountryName);
        WaitForKey;
      end;
    end;
  end;

  procedure ShowRepairWindow(var OutShipDamage: real);
  var
    { Port fix: was a 16-bit integer, which both overflowed on Trunc and
      forced a 32767 input cap - but ship damage is real and can far
      exceed that (the damage formulas scale with the player's wealth) }
    DamageToRepair: real;
    RemainingDamageString: string[10];

    function CannotAfford(LostAmount: real): boolean;
    begin
      if LostAmount > 0 then
      begin
        if LostAmount > Money then
        begin
          ShowMessage('W', '', 'ìëã ÑÅòÑ äã-åã äå èâÄ', Bell);
          CannotAfford := True;
        end
        else
          CannotAfford := False;
      end;
    end;

  begin
    repeat
    begin
      PortPutPic(RepairWindow^, 0, 142);
      PortGotoXY(21, 17);
      Write(OutShipDamage:4:0);
      { Port upgrade: '=' repairs everything (the full damage, capped by cash) }
      QuickAmountA := Int(OutShipDamage);
      if QuickAmountA > Int(Money) then
        QuickAmountA := Int(Money);
      PortGotoXY(13, 18);
      Write('åãÑ = =');
      PortGotoXY(5, 17);
      DamageToRepair := InputReal(0, 0, 5);
      QuickAmountA := 0;
    end
    until not CannotAfford(DamageToRepair);
    if DamageToRepair >= 0 then
    begin
      Money := Money - DamageToRepair;
      OutShipDamage := OutShipDamage - DamageToRepair;
      if OutShipDamage < 0 then
        OutShipDamage := 0;

      Str(OutShipDamage:1:0, RemainingDamageString);
      if OutShipDamage = 0 then
        ShowMessage('W', '', 'èâàÖåáå Ñêâóö öíã ÑêâîëÑ', '')
      else
        ShowMessage('W', '', 'á"ô ' + RemainingDamageString + ' äëÅ óÜê òÄôê', '');
    end;
  end;

  function IsUnableToRepair: boolean;
  begin
    IsUnableToRepair := ShipDamage > Money + BankBalance + GetTotalCargoValue;
  end;

{ Options in main menu }
const
  Buy = 1;
  Sell = 2;
  Travel = 3;
  Bank = 4;
  Stay = 5;
  Repair = 6;
begin
  PortGotoXY(1, 2);
  TimeOfDay := 8;
  ShouldSkipDay := False;
  GenerateNewPrices;
  ShowNewDayScreen;
  if Day = 7 then
  begin
    ShowMessage('W', '.èÖòáÄÑ òáëéÑ çÖâ ÖÑÜ', 'çÖâÑ ìÖëÅ òÖãéå òÖãÜ', ',äöòÖáë åã öÄ');
    ShowMessage('W', 'òÖÅñöô öÖÉÖóêÑô èÖâãé', 'çÖãë âî åí ÑêíÅóö', '! äÉâÅ òöÖÖâô ìëãÑ');
  end;
  { Port upgrade (toggleable): 10% interest on money deposited overnight,
    whole shekels only - fractions are truncated, no coin fractions }
  if BankInterest and (Day > 1) and (Int(BankBalance / 10) > 0) then
  begin
    ShowMessage('W', 'öâÅâò 10% äå çåâô óêÅÑ', 'á"ô ' + ToStrWithCommas(Int(BankBalance / 10)) + ' :öîëÖö',
        'ÑåâåÑ öÉóîÑ åí');
    BankBalance := BankBalance + Int(BankBalance / 10);
  end;
  UpdateStorm(StormCountryName);
  ShowStormScreen;
  PortGotoXY(1, 10);
  if Day > 1 then
    DoDayStartEvent;
  if not ((Day > 7) or ShouldSkipDay) then
  begin
    PortPutPic(MainScreen^, 0, 199);
    PortGetPic(MainMenuPic^, 0, 0, 240, 142);
    ToggleCountryHighlight(CurrentCountryName);
    repeat
    begin
      PortPutPic(MainMenuPic^, 0, 142);
      ShowHUD;
      PortTextColor(3);
      MainMenuOptionRects[Buy].X1 := 131;
      MainMenuOptionRects[Buy].Y1 := 36;
      MainMenuOptionRects[Buy].X2 := 177;
      MainMenuOptionRects[Buy].Y2 := 45;
      MainMenuOptionRects[Sell].X1 := 126;
      MainMenuOptionRects[Sell].Y1 := 51;
      MainMenuOptionRects[Sell].X2 := 174;
      MainMenuOptionRects[Sell].Y2 := 60;
      MainMenuOptionRects[Travel].X1 := 125;
      MainMenuOptionRects[Travel].Y1 := 67;
      MainMenuOptionRects[Travel].X2 := 171;
      MainMenuOptionRects[Travel].Y2 := 76;
      MainMenuOptionRects[Bank].X1 := 81;
      MainMenuOptionRects[Bank].Y1 := 83;
      MainMenuOptionRects[Bank].X2 := 167;
      MainMenuOptionRects[Bank].Y2 := 92;
      MainMenuOptionRects[Stay].X1 := 42;
      MainMenuOptionRects[Stay].Y1 := 99;
      MainMenuOptionRects[Stay].X2 := 162;
      MainMenuOptionRects[Stay].Y2 := 108;
      MainMenuOptionRects[Repair].X1 := 33;
      MainMenuOptionRects[Repair].Y1 := 114;
      MainMenuOptionRects[Repair].X2 := 160;
      MainMenuOptionRects[Repair].Y2 := 123;
      if ShipDamage = 0 then
        MainMenuOptionCount := 5
      else
      begin
        PortPutPic(RepairOptionPic^, 35, 123);
        MainMenuOptionCount := 6;
      end;
      ChosenMainMenuOption := ShowOptions(MainMenuOptionRects, MainMenuOptionCount);
      case ChosenMainMenuOption of
        Buy: begin
          if CurrentCountryName = IsraelName then
            ShowBuyWindow(CopperPriceIsrael, OlivesPriceIsrael, WheatPriceIsrael)
          else if CurrentCountryName = TurkeyName then
            ShowBuyWindow(CopperPriceTurkey, OlivesPriceTurkey, WheatPriceTurkey)
          else
            ShowBuyWindow(CopperPriceEgypt, OlivesPriceEgypt, WheatPriceEgypt);
        end;
        Sell: begin
          if CurrentCountryName = IsraelName then
            ShowSellWindow(CopperPriceIsrael, OlivesPriceIsrael, WheatPriceIsrael);
          if CurrentCountryName = TurkeyName then
            ShowSellWindow(CopperPriceTurkey, OlivesPriceTurkey, WheatPriceTurkey);
          if CurrentCountryName = EgyptName then
            ShowSellWindow(CopperPriceEgypt, OlivesPriceEgypt, WheatPriceEgypt);
        end;
        Travel: begin
          if TimeOfDay > 16 then
            ShowMessage('W', 'íÖëêå èöâê Äå', 'öòáÖÄé Ñã ÑíôÅ', '')
          else if ShipDamage > 1000 then
            ShowMessage('W', 'ÖêâÄ äöêâîëå çòÇêô óÜêÑ', '.ÇâåîÑå äå òôîÄé', '' )
          else if not AskOverweight(GetTotalCargoWeight) then
            TravelFlow(StormCountryName);
        end;
        Bank: begin
          if ClosedBankDay = Day then
            ShowMessage('W', '', 'çÖâÑ  òÖÇë  óêÅÑ ', '')
          else
            ShowBankWindow;
        end;
        Stay: begin
          PortPutPic(StayWindow^, 0, 142);
          if Day = 7 then
            ShowMessage('N', ',çâöëÑ òáëéÑ íÖÅô', 'óáôéÑ çÖâëå  '#17'ƒƒƒŸ  ôóÑ', '');
          repeat
            LastPressedKey := GetPressedKey;
          until LastPressedKey in [Escape, Enter];
        end;
        Repair: begin
          if ClosedRepairDay <> Day then
            ShowRepairWindow(ShipDamage)
          else
            ShowMessage('W', 'òÖÇë åéêÅ çâêÖóâöÑ ìâñò', 'èâöéÑå ïåÄö - çÖâÑ', 'òáé Éí');
        end;
      end;
    end
    until (ChosenMainMenuOption = 5) and (LastPressedKey <> Escape);
    if (Day < 7) and IsUnableToRepair then
    begin
      Day := 7;
      ShowMessage('W', 'öÄ èóöå äöÖòôîÄÅ èâÄ', '-äöêâîëå çòÇêô óÜêÑ', '! !  ç â ö ë Ñ   ó á ô é Ñ');
    end;
  end;
end;

procedure Init;
var
  I: byte;
  J: byte;
  Line6File: file;
  CurrentDir: string[15];
begin
  PortGraphColorMode;
  PortTextColor(3);
  PortPalette(0);
  PortGraphBackground(1);
  CurrentDir := ''; { PORT PATCH: was GetDir(0, CurrentDir); exe runs with CWD=socher1, no drive check needed }
  if CurrentDir = 'A:\' then
  begin
    ExtraFilesPath := 'B:\';
  end
  else
  begin
    ExtraFilesPath := ''; { PORT PATCH: was ExtraFilesPath := '\kika\socher_b\'; all assets live in the working directory }
  end;
  ShowScrFile(ExtraFilesPath + 'mifne.scr');
  WaitForKey;
  ShowScrFile(ExtraFilesPath + 'credits.scr');
  BaseWarePrice[Copper] := 3000;
  BaseWarePrice[Olives] := 500;
  BaseWarePrice[Wheat] := 50;
  WareName[Copper] := ' öôÖáê ';
  WareName[Olives] := ' çâöâÜ ';
  WareName[Wheat] := ' Ñàâá ';
  DayName[1] := 'èÖôÄò';
  DayName[2] := '  âêô';
  DayName[3] := 'âôâåô';
  DayName[4] := 'âíâÅò';
  DayName[5] := 'âôâéá';
  DayName[6] := ' âôâô';
  DayName[7] := 'âíâÅô';
  MainScreen := LoadScrFile('mainscrn');
  NewDayScreen := LoadScrFile('newday');
  HelpScreen := LoadScrFile('help');
  CrewScreen := LoadScrFile('crew');
  SocherScreen := LoadScrFile('socher');
  ExpandScreen := LoadScrFile('expand');
  ThievesScreen := LoadScrFile('thieves');
  WheatUpScreen := LoadScrFile('wheatup');
  WheatDownScreen := LoadScrFile('wheatdn');
  OlivesUpScreen := LoadScrFile('olivesup');
  OlivesDownScreen := LoadScrFile('olivesdn');
  CopperUpScreen := LoadScrFile('copperup');
  CopperDownScreen := LoadScrFile('copperdn');
  New(MainMenuPic);
  StayWindow := LoadWinFile('stay');
  RepairWindow := LoadWinFile('repair');
  DesertedWindow := LoadWinFile('deserted');
  DamagedWindow := LoadWinFile('damaged');
  NightWindow := LoadWinFile('night');
  OverweightWindow := LoadWinFile('overwght');
  GuardsWindow := LoadWinFile('guards');
  PiratesWindow := LoadWinFile('pirates');
  ConfrontWindow := LoadWinFile('confront');
  RunAwayWindow := LoadWinFile('runaway');
  OfferWindow := LoadWinFile('offer');
  ChangeWindow := LoadWinFile('change');
  SinkingWindow := LoadWinFile('sinking');
  StormWindow := LoadWinFile('storm');
  ShoalWindow := LoadWinFile('sirton');
  StrikeWindow := LoadWinFile('strike');
  { FogWindow is loaded but never displayed - the fog travel event uses ShowMessage only }
  FogWindow := LoadWinFile('arafel');
  BankWindow := LoadWinFile('bank');
  BuyTurkeyWindow := LoadWinFile('buyturk');
  BuyIsraelWindow := LoadWinFile('buyisr');
  BuyEgyptWindow := LoadWinFile('buyegpt');
  SellTurkeyWindow := LoadWinFile('sellturk');
  SellIsraelWindow := LoadWinFile('sellisr');
  SellEgyptWindow := LoadWinFile('sellegpt');
  BuySign := LoadSgnFile('buy');
  SellSign := LoadSgnFile('sell');
  WithdrawSign := LoadSgnFile('withrow');
  DepositSign := LoadSgnFile('deposit');
  MessageSign := LoadSgnFile('message');
  PiratesSign := LoadSgnFile('pirates');
  New(RepairOptionPic);
  Assign(Line6File, 'line_6.lin');
  Reset(Line6File);
  BlockRead(Line6File, RepairOptionPic^, 3);
  Close(Line6File);
  Tp3Randomize;
  MapCoordinates[Turkey].X := 191;
  MapCoordinates[Turkey].Y := 23;
  MapCoordinates[Israel].X := 198;
  MapCoordinates[Israel].Y := 86;
  MapCoordinates[Egypt].X := 123;
  MapCoordinates[Egypt].Y := 113;
  MapCoordinates[4].X := 198;
  MapCoordinates[4].Y := 50;
  MapCoordinates[5].X := 161;
  MapCoordinates[5].Y := 96;
  MapCoordinates[6].X := 123;
  MapCoordinates[6].Y := 40;
  MapCoordinates[7].X := 123;
  MapCoordinates[7].Y := 70;
  MapCoordinates[8].X := 196;
  MapCoordinates[8].Y := 45;
  for I := 1 to 8 do
    for J := 1 to 8 do
      MapWaypoints[I, J] := 0;
  MapWaypoints[Turkey, Israel] := 4;
  MapWaypoints[Israel, Turkey] := 4;
  MapWaypoints[Egypt, Israel] := 5;
  MapWaypoints[Israel, Egypt] := 5;
  MapWaypoints[Turkey, Egypt] := 6;
  MapWaypoints[Egypt, Turkey] := 6;
  MapWaypoints[6, 2] := 7;
  MapWaypoints[5, 1] := 8;
  WaitForKey;
end;

procedure StartWeek;
begin
  ShowScrFile(ExtraFilesPath + 'intro.scr');
  MapWindow := LoadWinFile('map');
  Unused3 := 1;
  BankBalance := 0;
  ShipCapacity := 100;
  CurrentCountryName := IsraelName;
  StormCountryName := ' ';
  ShipDamage := 0;
  if AllowClosedShipyard then
    ClosedRepairDay := Tp3Random(7) + 1
  else
    ClosedRepairDay := 0;
  ClosedBankDay := Tp3Random(7) + 1;
  Cargo[Copper] := 0;
  Cargo[Olives] := 0;
  Cargo[Wheat] := 0;
  { Original bug: the random ClosedBankDay set above is immediately overwritten with 0,
    so the bank's "closed day" mechanic never triggers (ClosedRepairDay still works) }
  ClosedBankDay := 0;
  Money := 10000;
  LastDayStartEvent := 0;
  WaitForKey;
end;

procedure HighScoreFlow(FinalScore: real);
type
  TPlayerName = string[20];
  THighScoreEntry = record
    PlayerName: TPlayerName;
    Score: Tp3GameReal; { PORT PATCH: was Score: real; TP3 real = 6-byte Real48, keeps THighScoreEntry at 27 bytes for winners.win }
  end;
var
  WinnersFile: file of THighScoreEntry;
  HighScores: array[1..11] of THighScoreEntry;
  HighScoreCount: integer;
  NewHighScoreIndex: integer;
  I: integer;
  Unused: byte;
  Unused2: array[1..8] of byte;
  procedure InputString(var OutString: TPlayerName; MaxLength: integer);
  var
    PressedKey: char;
    InputLength: integer;
    HebrewIndex: integer;
    IsFinished: boolean;
    CurrentString: TPlayerName;
  const
    LowerCaseLatin: string[27] = 'tcdsvuzjyhlfkonibxg;p.mera,';
    UpperCaseLatin: string[27] = 'TCDSVUZJYHLFKONIBXG:P>MERA<';
    Hebrew: string[38] = 'ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôö1234567890 ';
  begin
    IsFinished := False;
    CurrentString := '';
    InputLength := 0;
    repeat
    begin
      PressedKey := GetPressedKey;
      HebrewIndex := Pos(PressedKey, LowerCaseLatin) + Pos(PressedKey, UpperCaseLatin) +
          Pos(PressedKey, Hebrew);
      if HebrewIndex > 0 then
      begin
        Write(Hebrew[HebrewIndex], Backspace, Backspace);
        InputLength := InputLength + 1;
        CurrentString := Hebrew[HebrewIndex] + CurrentString;
      end
      else if (PressedKey = Backspace) and (InputLength > 0) then
      begin
        Write('  ', Backspace);
        CurrentString := Copy(CurrentString, 2, InputLength);
        InputLength := InputLength - 1;
      end
      else if (PressedKey = Enter) and (InputLength > 0) then
        IsFinished := True;
      if InputLength = MaxLength then
        IsFinished := True;
    end
    until IsFinished;
    OutString := CurrentString;
  end;
begin
  Unused := 0;
  HighScoreCount := 1;
  NewHighScoreIndex := 0;
  Assign(WinnersFile, 'winners.win');
  {$I-} Reset(WinnersFile); {$I+}
  if IOResult <> 0 then
  begin
    { Port fix: the original also called Close here, so the unconditional
      Close below was a double-close - a latent I/O error 103 that never
      triggered in DOS only because WINNERS.WIN always shipped with the game }
    Rewrite(WinnersFile);
  end
  else
  begin
    while (not EOF(WinnersFile)) do
    begin
      Read(WinnersFile, HighScores[HighScoreCount]);
      if (NewHighScoreIndex = 0) and (Tp3GameRealToDouble(HighScores[HighScoreCount].Score) < FinalScore) then { PORT PATCH: FPC cannot compare Real48 directly }
      begin
        NewHighScoreIndex := HighScoreCount;
        HighScoreCount := HighScoreCount + 1;
        HighScores[HighScoreCount] := HighScores[(HighScoreCount - 1)];
        HighScores[NewHighScoreIndex].Score := DoubleToTp3GameReal(FinalScore); { PORT PATCH: FPC cannot assign a double directly to Real48 }
      end;
      HighScoreCount := HighScoreCount + 1;
    end;
  end;
  Close(WinnersFile);
  if (HighScoreCount <= 10) and (NewHighScoreIndex = 0) then
  begin
    NewHighScoreIndex := HighScoreCount;
    HighScoreCount := HighScoreCount + 1;
    HighScores[HighScoreCount] := HighScores[HighScoreCount - 1];
    HighScores[NewHighScoreIndex].Score := DoubleToTp3GameReal(FinalScore); { PORT PATCH: FPC cannot assign a double directly to Real48 }
  end;
  if HighScoreCount > 11 then
    HighScoreCount := 11;
  if NewHighScoreIndex <> 0 then
  begin
    ShowScrFile(ExtraFilesPath + 'in_10.scr');
    PortGotoXY(28, 20);
    InputString(HighScores[NewHighScoreIndex].PlayerName, 16);
    Rewrite(WinnersFile);
    for I := 1 to HighScoreCount - 1 do
      Write(WinnersFile, HighScores[I]);
    Close(WinnersFile);
  end;
  ShowScrFile(ExtraFilesPath + 'first_10.scr');
  for I := 1 to HighScoreCount - 1 do
  begin
    PortGotoXY(11, 3 + 2 * I);
    Write(Tp3GameRealToDouble(HighScores[I].Score):6:0); { PORT PATCH: FPC cannot Write a Real48 directly }
    PortGotoXY(20, 3 + 2 * I);
    Write(HighScores[I].PlayerName:16);
  end;
  if NewHighScoreIndex = 0 then
  begin
    ScoreSign := LoadSgnFile('score');
    PortPutPic(ScoreSign^, 7, 130);
    PortGotoXY(3, 10);
    if FinalScore > 999 then
      Write(ToStrWithCommas(FinalScore):6)
    else
      Write(ToStrWithCommas(FinalScore):4);
    Dispose(ScoreSign);
  end;
  WaitForKey;
end;

{ Port welcome screen (not part of the original game).
  Hebrew literals are in visual (reversed) order like the rest of the game. }
procedure ShowCustomScreen;
begin
  PortGraphColorMode;
  PortTextColor(3);
  PortPalette(0);
  PortGraphBackground(1);
  PortGotoXY(9, 8);
  Write('çâÑ òáÖëå çâÄÅÑ çâãÖòÅ');
  PortGotoXY(14, 10);
  Write('32 BIT öëòâÇ');
  PortGotoXY(6, 13);
  Write('ÑÅÖÑÄÑ öâëÖÉÑ ÑëòâÇÑ åí ëëÖÅé');
  PortGotoXY(15, 15);
  Write('Ñáâå ëê Äåô');
  PortGotoXY(12, 19);
  Write('äôéÑå  '#17'ƒƒƒŸ  ôóÑ');
  WaitForKey;
end;

{ POC: load saved port settings from port.cfg (one digit per line, in the game
  directory). A missing or invalid file falls back to original-game behavior. }
procedure LoadPortSettings;
var
  CfgFile: text;
  V1: integer;
  V2: integer;
  V3: integer;
  V4: integer;
  V5: integer;
  V6: integer;
begin
  { Defaults = original game behavior }
  AllowCrewStrike := True;
  AllowClosedShipyard := True;
  FixedGuardShipPrice := False;
  AllowClosedPorts := True;
  BankInterest := False;
  AllowNavigationError := True;
  Assign(CfgFile, 'port.cfg');
  {$I-} Reset(CfgFile); {$I+}
  if IOResult = 0 then
  begin
    {$I-}
    Readln(CfgFile, V1);
    Readln(CfgFile, V2);
    Readln(CfgFile, V3);
    Readln(CfgFile, V4);
    Readln(CfgFile, V5);
    Readln(CfgFile, V6);
    {$I+}
    if IOResult = 0 then
    begin
      AllowCrewStrike := V1 <> 0;
      AllowClosedShipyard := V2 <> 0;
      FixedGuardShipPrice := V3 <> 0;
      AllowClosedPorts := V4 <> 0;
      BankInterest := V5 <> 0;
      AllowNavigationError := V6 <> 0;
    end;
    Close(CfgFile);
  end;
end;

procedure SavePortSettings;
var
  CfgFile: text;
begin
  Assign(CfgFile, 'port.cfg');
  {$I-}
  Rewrite(CfgFile);
  if IOResult = 0 then
  begin
    Writeln(CfgFile, Ord(AllowCrewStrike));
    Writeln(CfgFile, Ord(AllowClosedShipyard));
    Writeln(CfgFile, Ord(FixedGuardShipPrice));
    Writeln(CfgFile, Ord(AllowClosedPorts));
    Writeln(CfgFile, Ord(BankInterest));
    Writeln(CfgFile, Ord(AllowNavigationError));
    Close(CfgFile);
  end;
  {$I+}
end;

{ POC: visual settings menu for the port build. Arrows move the cursor,
  Space or Enter toggles a mechanic on/off, Enter on START GAME (or Escape)
  saves the choices to port.cfg and starts playing. }
procedure ShowSettingsScreen;
const
  OptionCount = 6;
var
  Selected: integer;
  PressedKey: char;
  Values: array[1..OptionCount] of boolean;
  Names: array[1..OptionCount] of string[30];
  IsDone: boolean;
  I: integer;

  procedure DrawRow(Index: integer);
  begin
    PortGotoXY(4, 6 + 2 * Index);
    if Index = Selected then
      Write('>')
    else
      Write(' ');
    if Index <= OptionCount then
    begin
      Write(' ');
      if Values[Index] then
        Write('[X] ')
      else
        Write('[ ] ');
      Write(Names[Index]);
    end
    else
      Write('   START GAME');
  end;

begin
  LoadPortSettings;
  Names[1] := 'Crew strike';
  Names[2] := 'Shipyard closed days';
  Names[3] := 'Ports closed (fog/strike)';
  Names[4] := 'Fixed guard ship price';
  Names[5] := 'Bank interest 10%/night';
  Names[6] := 'Navigation errors (winds)';
  Values[1] := AllowCrewStrike;
  Values[2] := AllowClosedShipyard;
  Values[3] := AllowClosedPorts;
  Values[4] := FixedGuardShipPrice;
  Values[5] := BankInterest;
  Values[6] := AllowNavigationError;
  PortGraphColorMode;
  PortTextColor(3);
  PortPalette(0);
  PortGraphBackground(1);
  PortGotoXY(13, 3);
  Write('--- SETTINGS ---');
  PortGotoXY(3, 21);
  Write('Arrows: move   Space: on/off');
  PortGotoXY(3, 22);
  Write('Enter on START GAME to play');
  { Cursor starts on START GAME so a single Enter begins playing }
  Selected := OptionCount + 1;
  for I := 1 to OptionCount + 1 do
    DrawRow(I);
  IsDone := False;
  repeat
    PressedKey := GetPressedKey;
    case PressedKey of
      Up, Left: begin
        I := Selected;
        Selected := Selected - 1;
        if Selected < 1 then
          Selected := OptionCount + 1;
        DrawRow(I);
        DrawRow(Selected);
      end;
      Down, Right: begin
        I := Selected;
        Selected := Selected + 1;
        if Selected > OptionCount + 1 then
          Selected := 1;
        DrawRow(I);
        DrawRow(Selected);
      end;
      ' ': begin
        if Selected <= OptionCount then
        begin
          Values[Selected] := not Values[Selected];
          DrawRow(Selected);
        end;
      end;
      Enter: begin
        if Selected <= OptionCount then
        begin
          Values[Selected] := not Values[Selected];
          DrawRow(Selected);
        end
        else
          IsDone := True;
      end;
      Escape: IsDone := True;
    end;
  until IsDone;
  AllowCrewStrike := Values[1];
  AllowClosedShipyard := Values[2];
  AllowClosedPorts := Values[3];
  FixedGuardShipPrice := Values[4];
  BankInterest := Values[5];
  AllowNavigationError := Values[6];
  SavePortSettings;
end;

begin
  ShowCustomScreen;
  Init;
  ShowSettingsScreen;
  repeat
  begin
    Day := 1;
    StartWeek;
    repeat
    begin
      PlayDay;
      Day := Succ(Day);
    end
    until Day > 7;
    HighScoreFlow(Money + BankBalance);
    PortGraphColorMode;
    PortGraphBackground(1);
    ShowMessage('N', 'èâêÖíé ÑöÄ çÄÑ', '(å/ã) ? ìëÖê óáôéÅ', '');
    PortGotoXY(7, 16);
  end
  until not InputYesNo;
end.
