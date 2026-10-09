#!/bin/sh
# Links the four game builds into one program with a game picker
# (launcher/launcher.cpp): wolf4sdl.3dsx.
#
# Wolf4SDL chooses the game at compile time, so each build defines the same
# functions and globals. This script renames every global symbol each build
# defines with a prefix (main -> w3d_main, ...) using objcopy, so the four
# can be linked together, and the picker calls the chosen <prefix>_main.
#
# Run it after building the four variants (BUILD=build-<variant>), inside the
# devkitARM environment:
#   tools/3ds/build-combined.sh
set -eu

BIN=$DEVKITARM/bin/arm-none-eabi
OUT=build-combined
ARCH="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft"
PORTLIBS=$DEVKITPRO/portlibs/3ds
CTRULIB=$DEVKITPRO/libctru
LIBS="-lSDL_mixer -lvorbisidec -lmikmod -lmad -logg -lSDL_ttf -lSDL_image -lSDL_gfx -lSDL -lfreetype -lpng -ljpeg -lz -lcitro3d -lctru -lm"

rm -rf "$OUT"
mkdir -p "$OUT"

for pair in wolf3d:w3d wolf3d-shareware:w3s sod:sod sod-demo:sdm; do
    variant=${pair%%:*}
    prefix=${pair##*:}
    [ -d "build-$variant" ] || { echo "build-$variant is missing: build that variant first"; exit 1; }
    mkdir -p "$OUT/$variant"

    # "old new" for every global symbol defined by this build.
    "$BIN-nm" -g --defined-only build-$variant/*.o \
        | awk -v p="$prefix" 'NF == 3 { print $3 " " p "_" $3 }' \
        | sort -u > "$OUT/$variant.syms"

    for obj in build-$variant/*.o; do
        "$BIN-objcopy" --redefine-syms="$OUT/$variant.syms" "$obj" "$OUT/$variant/$(basename "$obj")"
    done
done

"$BIN-g++" $ARCH -O2 -mword-relocations -D__3DS__ -I"$CTRULIB/include" \
    -c launcher/launcher.cpp -o "$OUT/launcher.o"

"$BIN-g++" -specs=3dsx.specs -g $ARCH -Wl,-Map,"$OUT/wolf4sdl.map" \
    "$OUT/launcher.o" "$OUT"/*/*.o \
    -L"$CTRULIB/lib" -L"$PORTLIBS/lib" $LIBS \
    -o wolf4sdl.elf

"$DEVKITPRO/tools/bin/smdhtool" --create "Wolf4SDL 3DS" \
    "Wolfenstein 3D and Spear of Destiny" "AlexanderV387, hax0kartik, Wolf4SDL" \
    assets/icon-all.png wolf4sdl.smdh
"$DEVKITPRO/tools/bin/3dsxtool" wolf4sdl.elf wolf4sdl.3dsx --smdh=wolf4sdl.smdh

"$BIN-size" wolf4sdl.elf
