# th06 Wii port

This is a Wii port of 東方紅魔郷　～ the Embodiment of Scarlet Devil 1.02h.

The original codebase was taken from [EoSD-portable](https://github.com/GensokyoClub/th06/tree/portable).

For the N3DS version, check out the main (portable) branch from this current repo

> [!WARNING]
> Big endian support from upstream is still experimental, and as such crashes and unintended behavior are to be expected.
> - For example, scores/replays may not work properly and getting to watch the demo play will softlock/crash the game.
> - You may need to manually turn BGM off and back on to wav for the music to start playing
> - You need to turn on `No Depth test` in th06-config or else graphics will not render properly (this is probably an issue with the graphics library itself)

### Usage

> [!IMPORTANT]
> File names for .DAT files will vary depending on the `TH_LANG` string in `i18n.hpp`.
>
> `TH_LANG` is currently `TH_JP` 
> 
> If it is `TH_EN` then they should be named `KOUMAKYO_<name>.DAT`
>
> If it is `TH_JP` then they should be named `紅魔郷<name>.DAT`
>
> (you can also know which one to use by looking at the file name mentioned on the pre-launch error)
>
> When extracting assets from an original copy of the game, they will (or at least should??) be following the notation for `TH_JP`, so when transfering them when `TH_LANG` is `TH_EN`, they should be renamed accordingly.
> 
> For the sake of notation, .DAT files will be referred to here as `<name>.DAT` rather than `KOUMAKYO_<name>.DAT` or `紅魔郷<name>.DAT`, but when transfering to your SD card they should follow the naming convention named above.
>
> It is important to mention that `.DAT` is indeed uppercase.

Place the following game assets as taken from an original copy of the game (ver 1.02h) into `sd:/th06`:
- `bgm` (as in the folder with the 17 .wav files inside)
- `CM.DAT`
- `ED.DAT`
- `IN.DAT`
- `MD.DAT`
- `ST.DAT`
- `TL.DAT`
- `msgothic.ttc` OR `NotoSansJP-Regular.ttf` (fonts)

# Dependencies
This project has the following dependencies:

### available on (dkp)-pacman:
- `libogc`
- `libfat-ogc`
- `ppc-brotli`
- `ppc-freetype`
- `ppc-harfbuzz`
- `ppc-libjpeg-turbo`
- `ppc-libpng`
- `wii-opengx`
- `wii-sdl2`
- `wii-sdl2_image`
- `wii-sdl2_ttf`
# Building

In the repository root directory, simply run `make` (or `make -f config.mak` for th06_config) and the ".elf" and ".dol" executables will be generated.

Place either one on `sd:/apps/th06` (or `/apps/th06_config`) (NOT `sd:/th06`, that one is only for game assets)

# Credits

All credits for the original codebase goes to [GensokyoClub/th06 (portable)](https://github.com/GensokyoClub/th06/tree/portable)

Touhou Project is © Team Shanghai Alice
