// [3DS] The top screen at 800x240, its wide mode (not on the original 2DS):
// the 3D view drawn with twice the columns, the rest of the picture (menus,
// texts) twice as wide. SDL then draws only the bottom screen; the top one
// is made here, on the New 3DS's core 2.

#ifndef N3DS_WIDE_H
#define N3DS_WIDE_H

#include <SDL/SDL.h>

extern boolean widescreen800;  // the setting (from the next start)

// Whether this run uses it: the setting at the start, where the mode exists.
bool N3DS_Wide (void);

// The 3D view, 800 columns wide (the view's rows and columns, doubled across).
byte *N3DS_WideView (void);

// The video mode in wide mode: SDL on the bottom screen only, and the
// 400x480 surface the game draws both screens into (as SDL_DUALSCR's).
SDL_Surface *N3DS_WideSetVideoMode (void);

// The frame to the screens: the bottom one through SDL, the top one made on
// core 2 from screen (its top 400x240) or, if view_ready, the wide view
// where the 3D view is (with source's palette).
void N3DS_WidePresent (SDL_Surface *screen, SDL_Surface *source, bool view_ready);

// Set by ThreeDRefresh before showing the frame it drew into the wide view.
extern bool n3ds_wide_view_ready;

#endif
