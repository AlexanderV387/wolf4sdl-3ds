// [3DS] The top screen at 800x240, its wide mode (not on the original 2DS):
// the 3D view drawn with twice the columns, the rest of the picture (menus,
// texts) twice as wide. Both screens are then made here, on the New 3DS's
// core 2, not by SDL.

#ifndef N3DS_WIDE_H
#define N3DS_WIDE_H

#include <SDL/SDL.h>

extern boolean widescreen800;  // the setting (from the next start)

// Whether this run uses it: the setting at the start, where the mode exists.
bool N3DS_Wide (void);

// The 3D view, 800 columns wide (the view's rows and columns, doubled across).
byte *N3DS_WideView (void);

// The video mode in wide mode: the 400x480 surface the game draws both
// screens into (as SDL_DUALSCR's).
SDL_Surface *N3DS_WideSetVideoMode (void);

// The frame to the screens, made on core 2: the bottom one from screen's
// bottom half (if it changed), the top one from its top 400x240 or, if
// view_ready, the wide view where the 3D view is (with source's palette).
void N3DS_WidePresent (SDL_Surface *screen, SDL_Surface *source, bool view_ready);

// Set by ThreeDRefresh before showing the frame it drew into the wide view.
extern bool n3ds_wide_view_ready;

#endif
