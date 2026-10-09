Wolfenstein 3D and Spear of Destiny for the Nintendo 3DS, with New 3DS controls, a HUD on the bottom screen and `.cia` builds. Based on Wolf4SDL and hax0kartik's 3DS port.

## New in 1.2.3

- Own icon and banner for `wolf4sdl-all` (both games).

## New in 1.2.2

- Spear of Destiny banner with its own title.

## New in 1.2.1

- Spear of Destiny menus: the 320x200 backdrop was drawn in the top left corner of the 400x240 screen, out of line with the centered menus and leaving the rest of the screen with old images. It is now centered and repeated to fill the screen.
- Sound, load/save and episode menus centered like the main menu (all games).
- Spear of Destiny `.cia` and `.3dsx` have their own icon and banner.

## New in 1.2.0

- `wolf4sdl-all`: Wolfenstein 3D, the shareware, Spear of Destiny (and its mission packs) and the Spear of Destiny demo in a single `.cia`. At startup a menu lists the games whose data is on the SD card; with only one it starts directly. The separate `.cia` files are still available; both kinds can be installed at the same time.

## New in 1.1.0

- Spear of Destiny mission picker: with the mission packs (`*.sd2`, `*.sd3`) in `/3ds/wolf4sdl/sod/` next to `*.sod`, a menu on the top screen chooses the mission at startup (Up/Down, A). With only `*.sod` the game starts directly, as before.

## Downloads

| File | Game data | Folder on the SD card |
|---|---|---|
| `wolf4sdl-all.cia` / `.3dsx` | Any of the below (one menu for all) | Both folders |
| `wolf4sdl-wolf3d.cia` / `.3dsx` | Wolfenstein 3D v1.4 (Activision, Steam, GOG): `*.wl6` | `/3ds/wolf4sdl/wolf3d/` |
| `wolf4sdl-wolf3d-shareware.cia` / `.3dsx` | Shareware v1.4: `*.wl1` | `/3ds/wolf4sdl/wolf3d/` |
| `wolf4sdl-sod.cia` / `.3dsx` | Spear of Destiny: `*.sod` (mission packs: `*.sd2`, `*.sd3`) | `/3ds/wolf4sdl/sod/` |
| `wolf4sdl-sod-demo.cia` / `.3dsx` | Spear of Destiny demo: `*.sdm` | `/3ds/wolf4sdl/sod/` |

Install the `.cia` with FBI, or copy the `.3dsx` to `/3ds/` and open it from the Homebrew Launcher. Game data is not included: use your own copy.

## What's new compared to the original 3DS port

- Dual stick on New 3DS: Circle Pad moves and strafes, C-stick turns (classic mode available)
- Touch turning for consoles without C-stick, with its own speed setting
- Three run modes: stick fully pushed, hold the button, or press once to sprint
- Rebindable buttons for every action; R fires by default
- Nintendo-style menus: A accepts, B goes back, START leaves the menu; quitting only through Quit
- HUD on the bottom screen in full screen view (top, middle or bottom)
- Tap the touch screen to turn the bottom screen off or on
- No DOS memory check screen at startup
- Spear of Destiny and Spear of Destiny demo builds
- `.cia` builds with New 3DS speed (804 MHz, L2 cache)

## Tested

Wolfenstein 3D (Steam data) on a New Nintendo 3DS with Luma3DS 13.4. The shareware and Spear of Destiny builds compile but have not been tested on hardware yet.

Developed with the help of an AI assistant (Claude); the icon and banner were made with AI tools.
