// Status bar (HUD) on the 3DS bottom screen.
//
// The video mode is 400x480: SDL shows the top half on the top screen and
// the bottom half on the bottom screen (its centre 320 columns). In full
// screen view (viewsize 21) the game skips the status bar; here it is
// drawn into its own surface and copied to the bottom half every frame.

#ifndef N3DS_HUD_H
#define N3DS_HUD_H

#include <SDL/SDL.h>

#define N3DS_VIDEO_HEIGHT   480     // top screen rows + bottom screen rows

void    N3DS_DrawHud (void);                    // once per frame in PlayLoop
void    N3DS_UpdateBottom (SDL_Surface *dest);  // before each SDL_Flip
boolean N3DS_HudVisible (void);

#endif
