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

// [test] The pacing variant (1 to 3) and the milliseconds per frame of the
// bottom screen's comparison, of making the top screen and of waiting for it.
extern int n3ds_pace_variant;
extern int n3ds_flip_ms, n3ds_top_ms, n3ds_wait_ms;

// [test] Times per frame for the log (/3ds/wolf4sdl/wide-log.txt), written
// once per variant: what each part of the frame took.
enum { WT_DRAW, WT_WIDEEND, WT_HUD, WT_FLIP, WT_WAIT, WT_TOP, WT_VBLANK, WT_BOTTOM, WT_COUNT };
void N3DS_WideTime (int part, unsigned long long ticks);
void N3DS_WideFrameDone (void);   // a frame shown: count it, log if due

#endif
