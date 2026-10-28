// Status bar (HUD), map and frame counter on the 3DS bottom screen.
//
// The video mode is 400x480: SDL shows the top half on the top screen and
// the bottom half on the bottom screen (its centre 320 columns). In full
// screen view (viewsize 21) the game skips the status bar; here it is
// drawn into its own surface and copied to the bottom half every frame.

#ifndef N3DS_HUD_H
#define N3DS_HUD_H

#include <SDL/SDL.h>

#define N3DS_VIDEO_HEIGHT   480     // top screen rows + bottom screen rows

enum { hudpos_top, hudpos_middle, hudpos_bottom, NUMHUDPOS };   // middle: older settings only
extern int hudpos;              // where on the bottom screen (with the map: above or below it)
extern boolean showmap;         // map of what was seen on the bottom screen, in any view size
extern boolean showfps;         // small frame counter on the bottom screen
extern boolean weaponbar;       // weapon buttons at the bottom of the bottom screen
#define NUMMAPZOOMS 5
extern int mapzoom;             // the map's zoom, 0 to NUMMAPZOOMS - 1 (2: 4 pixels a tile)

void    N3DS_DrawHud (void);                    // once per frame in PlayLoop
void    N3DS_Flip (SDL_Surface *dest);          // use instead of SDL_Flip(screen)
boolean N3DS_HudVisible (void);                 // status bar on the bottom screen
boolean N3DS_MapVisible (void);                 // map on the bottom screen
void    N3DS_ResetMap (void);                   // a level starts: nothing seen yet

// The map's zoom buttons (- and +): given every touch read, held or not;
// true when the touch is theirs (the game must not use it).
bool    N3DS_MapZoomUpdate (bool touching, int x, int y);
bool    N3DS_MapOwnsTouch (void);               // the touch held is theirs

#endif
