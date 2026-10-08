// 3DS button bindings and bottom screen control.

#include <stdlib.h>
#include "wl_def.h"
#include "n3ds_input.h"

int n3dsbind[N3DS_NUMBUTTONS];

const char *n3dsbuttonname[N3DS_NUMBUTTONS] =
{
    "A", "B", "X", "Y", "L", "R", "ZL", "ZR", "SELECT"
};

static const u32 n3dsmask[N3DS_NUMBUTTONS] =
{
    KEY_A, KEY_B, KEY_X, KEY_Y, KEY_L, KEY_R, KEY_ZL, KEY_ZR, KEY_SELECT
};

void N3DS_DefaultBindings (void)
{
    n3dsbind[n3ds_A]      = bt_attack;
    n3dsbind[n3ds_B]      = bt_use;
    n3dsbind[n3ds_X]      = bt_strafe;
    n3dsbind[n3ds_Y]      = bt_run;
    n3dsbind[n3ds_L]      = bt_prevweapon;
    n3dsbind[n3ds_R]      = bt_nextweapon;
    n3dsbind[n3ds_ZL]     = bt_run;
    n3dsbind[n3ds_ZR]     = bt_attack;
    n3dsbind[n3ds_SELECT] = bt_pause;
}

u32 N3DS_ButtonMask (int button)
{
    return n3dsmask[button];
}

int N3DS_HeldButton (void)
{
    hidScanInput ();
    u32 held = hidKeysHeld ();
    for (int i = 0; i < N3DS_NUMBUTTONS; i++)
        if (held & n3dsmask[i])
            return i;
    return -1;
}

void N3DS_PollButtons (void)
{
    u32 held = hidKeysHeld ();
    for (int i = 0; i < N3DS_NUMBUTTONS; i++)
        if ((held & n3dsmask[i]) && n3dsbind[i] != bt_nobutton)
            buttonstate[n3dsbind[i]] = true;

    if (held & KEY_START)
        buttonstate[bt_esc] = true;
}

//
// Bottom screen: SDL draws its text console there. A tap turns the
// backlight off to hide it (and save battery); another tap turns it on.
//

static bool bottomon = true;
static bool lcdready = false;

static void RestoreBottomScreen (void)
{
    if (!lcdready)
        return;
    if (!bottomon)
        GSPLCD_PowerOnBacklight (GSPLCD_SCREEN_BOTTOM);
    gspLcdExit ();
    lcdready = false;
}

void N3DS_InitBottomScreen (void)
{
    if (R_SUCCEEDED (gspLcdInit ()))
    {
        lcdready = true;
        atexit (RestoreBottomScreen);   // never leave the HOME Menu with it off
    }
}

void N3DS_PollBottomScreenToggle (void)
{
    static bool wastouching = false;
    bool touching = (hidKeysHeld () & KEY_TOUCH) != 0;

    if (touching && !wastouching && lcdready)
    {
        bottomon = !bottomon;
        if (bottomon)
            GSPLCD_PowerOnBacklight (GSPLCD_SCREEN_BOTTOM);
        else
            GSPLCD_PowerOffBacklight (GSPLCD_SCREEN_BOTTOM);
    }
    wastouching = touching;
}
