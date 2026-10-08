// 3DS button bindings and bottom screen control.
//
// SDL 1.2 for 3DS reports the buttons as joystick buttons with START as
// button 0 and A as button 1, which made the menus accept with START and
// go back with A. This module reads the buttons directly with libctru.

#ifndef N3DS_INPUT_H
#define N3DS_INPUT_H

#include <3ds.h>

enum
{
    n3ds_A,
    n3ds_B,
    n3ds_X,
    n3ds_Y,
    n3ds_L,
    n3ds_R,
    n3ds_ZL,
    n3ds_ZR,
    n3ds_SELECT,
    N3DS_NUMBUTTONS
};

// Game action (bt_*) for each button; bt_nobutton when unused.
// START and the D-pad are not bindable: START opens and closes the menu,
// the D-pad moves.
extern int n3dsbind[N3DS_NUMBUTTONS];
extern const char *n3dsbuttonname[N3DS_NUMBUTTONS];

void N3DS_DefaultBindings (void);
u32  N3DS_ButtonMask (int button);
int  N3DS_HeldButton (void);           // first bindable button held, or -1
void N3DS_PollButtons (void);          // sets buttonstate[] from the bindings

void N3DS_InitBottomScreen (void);
void N3DS_PollBottomScreenToggle (void);  // a tap on the touch screen turns it off/on

#endif
