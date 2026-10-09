# Wolf4SDL-3DS

Port of Wolfenstein 3D and Spear of Destiny to the Nintendo 3DS, based on [Wolf4SDL](https://github.com/hax0kartik/wolf4sdl-3ds) by hax0kartik. This fork (branch `n3ds`) adds New 3DS controls, a HUD on the bottom screen and `.cia` builds.

Runs at full speed on New 3DS.

## Features

- **Dual stick (New 3DS):** Circle Pad moves and strafes, C-stick turns. Classic mode (Circle Pad turns) can be selected in the menu.
- **Touch turning:** drag left or right on the touch screen to turn, for a 3DS without C-stick (*Options > Control > Touch turning*), with its own speed setting (1-10).
- **Three run modes:** stick fully pushed, hold the run button, or press once to sprint until you stop.
- **Rebindable buttons:** every action can be bound to A, B, X, Y, L, R, ZL, ZR or SELECT from *Options > Control > Customize buttons*.
- **Nintendo-style menus:** A accepts, B goes back, START leaves the menu. Quitting is only through *Quit*.
- **HUD on the bottom screen** in full screen view, at the top, middle or bottom of it.
- A tap on the touch screen turns the bottom screen off or on (except while it shows the HUD).
- No DOS memory check screen at startup.
- **Spear of Destiny mission packs:** if the `sod` folder has more than one mission (*Return to Danger* `*.sd2`, *Ultimate Challenge* `*.sd3`), a menu at startup lets you choose. Each mission keeps its own config and savegames.
- `.3dsx` and `.cia` builds for Wolfenstein 3D (full and shareware) and Spear of Destiny (full and demo).

## Installing

Download the builds from the latest successful run in [Actions](../../actions) (branch `n3ds`).

| Build | Game data | Folder on the SD card |
|---|---|---|
| `wolf4sdl-wolf3d` | Wolfenstein 3D v1.4 (Activision, Steam, GOG): `*.wl6` | `/3ds/wolf4sdl/wolf3d/` |
| `wolf4sdl-wolf3d-shareware` | Shareware v1.4: `*.wl1` | `/3ds/wolf4sdl/wolf3d/` |
| `wolf4sdl-sod` | Spear of Destiny: `*.sod`, plus `*.sd2` / `*.sd3` for the mission packs | `/3ds/wolf4sdl/sod/` |
| `wolf4sdl-sod-demo` | Spear of Destiny demo: `*.sdm` | `/3ds/wolf4sdl/sod/` |

- `.cia`: install with FBI; it appears on the HOME Menu.
- `.3dsx`: copy to `/3ds/` and open it from the Homebrew Launcher.

Game data is not included: use your own copy.

## Default controls

| Button | Action |
|---|---|
| Circle Pad | Move and strafe (dual stick) |
| C-stick | Turn |
| D-pad | Move and turn |
| R, ZR | Fire |
| A | Open / use |
| B, ZL | Run |
| X / Y | Next / previous weapon |
| L | Strafe |
| SELECT | Pause |
| START | Menu |

## Building

GitHub Actions builds every variant with `devkitpro/devkitarm` and SDL 1.2 for 3DS. Locally:

```sh
make TARGET=wolf4sdl-wolf3d BUILD=build-wolf3d \
     EXTRA_CFLAGS="-DVERSIONALREADYCHOSEN -DCARMACIZED -DGOODTIMES"
```

`tools/3ds/make-cia.sh` packages the `.elf` as a `.cia` (needs makerom and bannertool).

## Credits

The original Wolf4SDL manual (features, command line options, credits) is in [`docs/wolf4sdl-original-manual.txt`](docs/wolf4sdl-original-manual.txt).


- id Software, for Wolfenstein 3D
- All Wolf4SDL contributors
- hax0kartik, for the original 3DS port
- devkitPro
- Keeganatorr for the wolf4sdl Switch port
- Druivensap, DreadKnight for testing
- Steveice10, for the buildtools RSF template (MIT)

This fork was developed by AlexanderV387 with the help of an AI assistant (Claude). The icon and banner art were also made with AI tools.

## License

Same as Wolf4SDL: GPL-2.0, with the original id Software license and the MAME OPL emulator license (see `license-*.txt`).
