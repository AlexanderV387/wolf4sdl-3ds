// Status bar (HUD), map and frame counter on the 3DS bottom screen.

#include <math.h>
#include <stdio.h>
#include "wl_def.h"
#include "n3ds_hud.h"
#include "n3ds_input.h"

static SDL_Surface *hud = NULL;     // same size as screenBuffer, 8 bit
static boolean hudready = false;
int hudpos = hudpos_top;
boolean showmap = true;
boolean showfps = false;

// Tiles seen since the level started: the original has no map, so this one
// shows only what the player has seen (spotvis, cleared every frame).
static byte seen[MAPSIZE][MAPSIZE];

boolean N3DS_HudVisible (void)
{
    return hudready && ingame && viewsize == 21;
}

// In any view size: with a smaller view the status bar is on the top screen
// and the map fills the bottom screen.
boolean N3DS_MapVisible (void)
{
    return showmap && ingame && player;
}

void N3DS_ResetMap (void)
{
    memset (seen, 0, sizeof (seen));
}

void N3DS_DrawHud (void)
{
    if (viewsize != 21)
        return;

    if (!hud)
    {
        hud = SDL_CreateRGBSurface (SDL_SWSURFACE, screenWidth, screenHeight, 8, 0, 0, 0, 0);
        if (!hud)
            return;
        SDL_SetColors (hud, gamepal, 0, 256);
    }

    // Draw with the normal status bar code into the HUD surface. The Draw*
    // functions return early when viewsize is 21, so pretend it is 20.
    SDL_Surface *savedsurface = curSurface;
    unsigned savedpitch = curPitch;
    int savedviewsize = viewsize;

    curSurface = hud;
    curPitch = hud->pitch;
    viewsize = 20;

    VWB_DrawPicScaledCoord ((screenWidth-scaleFactor*320)/2,
        screenHeight-scaleFactor*STATUSLINES, STATUSBARPIC);
    DrawFace ();
    DrawHealth ();
    DrawLives ();
    DrawLevel ();
    DrawAmmo ();
    DrawKeys ();
    DrawWeapon ();
    DrawScore ();

    viewsize = savedviewsize;
    curSurface = savedsurface;
    curPitch = savedpitch;
    hudready = true;
}

// ==========================================================================
// Drawing on the bottom half of the 400x480 surface (32 bit). The bottom
// screen shows its centre 320 columns.

static SDL_Surface *target;
static int targetx, targety;            // bottom screen origin on the surface
static int cliptop, clipbottom;         // rows of the bottom screen to draw on

static void Fill (int x, int y, int w, int h, Uint32 color)
{
    int x0 = x < 0 ? 0 : x;
    int y0 = y < cliptop ? cliptop : y;
    int x1 = x + w > 320 ? 320 : x + w;
    int y1 = y + h > clipbottom ? clipbottom : y + h;

    for (int j = y0; j < y1; j++)
    {
        Uint32 *row = (Uint32 *) ((Uint8 *) target->pixels + (targety + j) * target->pitch) + targetx;
        for (int i = x0; i < x1; i++)
            row[i] = color;
    }
}

// 3x5 digits, as in the n3ds-ports hello world.
static void DrawNumber (int number, int x, int y, int scale, Uint32 color)
{
    static const byte digits[10][5] =
    {
        {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1},
        {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7},
    };

    char text[12];
    snprintf (text, sizeof (text), "%d", number);

    for (int i = 0; text[i]; i++)
        for (int row = 0; row < 5; row++)
            for (int col = 0; col < 3; col++)
                if (digits[text[i] - '0'][row] & (4 >> col))
                    Fill (x + (i * 4 + col) * scale, y + row * scale, scale, scale, color);
}

static boolean IsSeen (int x, int y)
{
    return x >= 0 && x < MAPSIZE && y >= 0 && y < MAPSIZE && seen[x][y];
}

// Between maptop and mapbottom, centred on the player, 4 pixels per tile.
static void DrawMap (int maptop, int mapbottom)
{
    const int tilesize = 4;
    SDL_PixelFormat *format = target->format;
    Uint32 background = SDL_MapRGB (format, 0x08, 0x08, 0x10);
    Uint32 floorcolor = SDL_MapRGB (format, 0x28, 0x38, 0x50);
    Uint32 wallcolor = SDL_MapRGB (format, 0xa8, 0xa8, 0xb8);
    Uint32 doorcolor = SDL_MapRGB (format, 0xe0, 0xb0, 0x30);
    Uint32 playercolor = SDL_MapRGB (format, 0xff, 0x40, 0x40);

    cliptop = maptop;
    clipbottom = mapbottom;
    Fill (0, maptop, 320, mapbottom - maptop, background);

    double px = (double) player->x / TILEGLOBAL;
    double py = (double) player->y / TILEGLOBAL;
    double centerx = 160;
    double centery = (maptop + mapbottom) / 2.0;

    for (int x = 0; x < MAPSIZE; x++)
    {
        for (int y = 0; y < MAPSIZE; y++)
        {
            Uint32 color = floorcolor;
            unsigned tile = tilemap[x][y];

            if (!tile)
            {
                if (!seen[x][y])
                    continue;
            }
            else
            {
                // Walls and doors appear once a tile next to them was seen.
                if (!IsSeen (x, y) && !IsSeen (x - 1, y) && !IsSeen (x + 1, y)
                        && !IsSeen (x, y - 1) && !IsSeen (x, y + 1))
                    continue;
                color = (tile & 0x80) ? doorcolor : wallcolor;
            }

            Fill ((int) floor (centerx + (x - px) * tilesize),
                  (int) floor (centery + (y - py) * tilesize),
                  tilesize, tilesize, color);
        }
    }

    // The player: a dot and a line toward where they look (angle in
    // degrees, 0 is east, counterclockwise).
    double angle = player->angle * M_PI / 180;
    double dx = cos (angle), dy = -sin (angle);

    for (int i = 0; i <= 8; i++)
        Fill ((int) (centerx + dx * i), (int) (centery + dy * i), 1, 1, playercolor);
    Fill ((int) centerx - 1, (int) centery - 1, 3, 3, playercolor);
}

// ==========================================================================
// Frame counter: small, in a corner of the bottom screen.

static int fpsframes = 0;
static uint32_t fpstime = 0;
static int fps = 0, framems = 0;

static void CountFrame (void)
{
    fpsframes++;
    uint32_t now = SDL_GetTicks ();
    uint32_t elapsed = now - fpstime;
    if (elapsed < 1000)
        return;
    fps = fpsframes * 1000 / elapsed;
    framems = elapsed / fpsframes;
    fpsframes = 0;
    fpstime = now;
}

static void UpdateBottom (SDL_Surface *dest)
{
    static int laststate = -1;
    boolean hudvisible = N3DS_HudVisible ();
    boolean mapvisible = N3DS_MapVisible ();
    // The state includes the layout so a change clears the old one.
    int state = (hudvisible ? 1 + hudpos : 0) + 4 * mapvisible + 16 * showfps;

    if (dest->h < N3DS_VIDEO_HEIGHT)
        return;

    target = dest;
    targetx = (dest->w - 320) / 2;
    targety = screenHeight;

    if (state != laststate)         // clear the bottom screen when it changes
    {
        SDL_Rect all = { 0, (Sint16) screenHeight, (Uint16) dest->w, (Uint16) screenHeight };
        SDL_FillRect (dest, &all, SDL_MapRGB (dest->format, 0, 0, 0));
        laststate = state;
        if (hudvisible || mapvisible)
            N3DS_BottomScreenOn ();     // the HUD and the map are never hidden
    }

    if (SDL_MUSTLOCK (dest) && SDL_LockSurface (dest) < 0)
        return;

    int statusrows = scaleFactor * STATUSLINES;
    int statusy = 0;

    // What the player sees is remembered even with the map hidden.
    if (ingame && player)
    {
        for (int x = 0; x < MAPSIZE; x++)
            for (int y = 0; y < MAPSIZE; y++)
                if (spotvis[x][y])
                    seen[x][y] = 1;
    }

    if (hudvisible)
    {
        int freerows = 240 - statusrows;

        if (mapvisible)
        {
            // Stats above or below the map, which fills the rest.
            statusy = hudpos == hudpos_bottom ? freerows : 0;

            if (statusy == 0)
                DrawMap (statusrows, 240);
            else
                DrawMap (0, freerows);
        }
        else
            statusy = hudpos == hudpos_top ? 0 : (hudpos == hudpos_middle ? freerows / 2 : freerows);
    }
    else if (mapvisible)
        DrawMap (0, 240);

    if (showfps)
    {
        // Frames per second (white) and milliseconds per frame (green),
        // in the right corner away from the stats.
        int y = hudvisible && statusy == 0 ? statusrows + 4 : 4;
        cliptop = 0;
        clipbottom = 240;
        Fill (256, y, 62, 13, SDL_MapRGB (dest->format, 0, 0, 0));
        DrawNumber (fps, 259, y + 2, 2, SDL_MapRGB (dest->format, 0xff, 0xff, 0xff));
        DrawNumber (framems, 289, y + 2, 2, SDL_MapRGB (dest->format, 0x80, 0xff, 0x80));
    }

    if (SDL_MUSTLOCK (dest))
        SDL_UnlockSurface (dest);

    if (hudvisible)
    {
        SDL_Rect src = { (Sint16) ((screenWidth - scaleFactor*320) / 2),
                         (Sint16) (screenHeight - statusrows),
                         (Uint16) (scaleFactor*320), (Uint16) statusrows };
        SDL_Rect dst = { (Sint16) targetx, (Sint16) (screenHeight + statusy), 0, 0 };
        SDL_BlitSurface (hud, &src, dest, &dst);
    }
}

void N3DS_Flip (SDL_Surface *dest)
{
    CountFrame ();
    UpdateBottom (dest);
    SDL_Flip (dest);

    // SDL presents from its own thread, so nothing paced the game to the
    // screen: the game ran at its own 70 Hz and the screen showed 60 of
    // those frames unevenly. Wait for the vertical blank here instead (not
    // when the HOME Menu closes the game: the screens are no longer ours).
    if (!aptShouldClose ())
        gspWaitForVBlank ();
}
