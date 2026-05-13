# th06 3DS port

This is a Nintendo 3DS port of 東方紅魔郷　～ the Embodiment of Scarlet Devil 1.02h.

The original codebase was taken from [EoSD-portable](https://github.com/GensokyoClub/th06/tree/portable).

## TODO:
- shamelessly steal the batch rendering code from th08 decomp (because the game currently doesnt do that and it runs like garbage on real hardware)
- find whichever unaligned accesses still remain
- better pre-launch check process

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

Place the following game assets as taken from an original copy of the game (ver 1.02h) into `sdmc:/3ds/eosd3ds`:
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
- `3ds-zlib`
- `3ds-freetype`
- `3ds-libpng`
- `3ds-libjpeg-turbo`

### build from source:

- [`my fork of picaGL`](https://github.com/shidemuri/picaGL)
- `SDL2`
- `SDL2_image`
- `SDL2_ttf`
- `makerom` (optional, but required if you want to build .cia)


#### devkitPro doesn't ship SDL2 packages (and makerom neither) on it's pacman repository. To build them, do the following:
1. Install the other dependencies
2. Clone the repositories
  
<code>git clone https://github.com/libsdl-org/\<SDL or SDL_image or SDL_ttf\>.git -b SDL2
cd SDL
mkdir build</code>

3. and then [build and install as instructed here](https://wiki.libsdl.org/SDL2/README-n3ds#building)
> [!IMPORTANT]
> For `SDL2_image` and `SDL2_ttf`, add `-DSDL2IMAGE_SAMPLES=OFF` and `-DSDLTTF_SAMPLES=OFF` (respectively) to the first `cmake` command. This is to prevent CMake from building the SDL2 samples, which would require extra libraries that are unnecessary here.

#### makerom

To build `makerom`, follow [this](https://github.com/3DSGuy/Project_CTR/blob/master/makerom/BUILDING.md) and then put the executable somewhere that `make` can read, like `$PATH`, `$DEVKITPRO/tools/bin` or even the repository root.

# Building

In the repository root directory, simply run `make` and the ".elf", ".3dsx" and ".cia" executables will be generated.

# Credits

All credits for the original codebase goes to [GensokyoClub/th06 (portable)](https://github.com/GensokyoClub/th06/tree/portable)

picaGL was originally written by [masterfeizz](https://github.com/masterfeizz/picaGL)

Touhou Project is © Team Shanghai Alice
