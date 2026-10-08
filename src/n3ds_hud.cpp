// Status bar (HUD) on the 3DS bottom screen.

#include "wl_def.h"
#include "n3ds_hud.h"
#include "n3ds_input.h"

static SDL_Surface *hud = NULL;     // same size as screenBuffer, 8 bit
static boolean hudready = false;

boolean N3DS_HudVisible (void)
{
    return hudready && ingame && viewsize == 21;
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

void N3DS_UpdateBottom (SDL_Surface *dest)
{
    static int laststate = -1;
    int state = N3DS_HudVisible () ? 1 : 0;

    if (dest->h < N3DS_VIDEO_HEIGHT)
        return;

    if (state != laststate)         // clear the bottom screen when it changes
    {
        SDL_Rect all = { 0, (Sint16) screenHeight, (Uint16) dest->w, (Uint16) screenHeight };
        SDL_FillRect (dest, &all, SDL_MapRGB (dest->format, 0, 0, 0));
        laststate = state;
        if (state)
            N3DS_BottomScreenOn ();     // the HUD is never hidden
    }

    if (state)
    {
        // The bottom screen shows columns 40-359 of the 400 wide surface.
        SDL_Rect src = { (Sint16) ((screenWidth - scaleFactor*320) / 2),
                         (Sint16) (screenHeight - scaleFactor*STATUSLINES),
                         (Uint16) (scaleFactor*320), (Uint16) (scaleFactor*STATUSLINES) };
        SDL_Rect dst = { (Sint16) ((dest->w - 320) / 2),
                         (Sint16) (screenHeight + (240 - scaleFactor*STATUSLINES) / 2), 0, 0 };
        SDL_BlitSurface (hud, &src, dest, &dst);
    }
}
