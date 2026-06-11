# Platform inventory

This is the first pass over the calls that must be replaced or wrapped for a
32-bit/64-bit port.

## Graphics

- `GraphColorMode`
- `GraphBackground`
- `Palette`
- `ColorTable`
- `PutPic`
- `GetPic`
- `TextColor`
- `GotoXY`
- `WhereX`
- `WhereY`
- `Write` while in graphics mode

## Input

- `Read(KBD, ch)`
- `KeyPressed`
- DOS-style extended key handling:
  - up: `#72`
  - down: `#80`
  - left: `#75`
  - right: `#77`
  - F10/help: `#68`

## Timing and sound

- `Delay`
- `Sound`
- `NoSound`

## Files

- Untyped `file` with `BlockRead`
- `file of THighScoreEntry` for `WINNERS.WIN`
- Hard-coded original asset paths in `Init`

## Runtime compatibility

- TP3-compatible `Randomize`/`Random`
- TP3-compatible `Real` behavior where it affects displayed values or ranking
- CP862 asset/text encoding boundaries
