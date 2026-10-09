// 3DS button bindings and bottom screen control.

#include <stdlib.h>
#include "wl_def.h"
#include "n3ds_input.h"
#include "n3ds_hud.h"

int n3dsbind[N3DS_NUMBUTTONS];

extern "C"
{
    void (*N3DS_QuitHook) (void) = NULL;
}

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
    n3dsbind[n3ds_R]      = bt_attack;      // R fires, as in most FPS
    n3dsbind[n3ds_ZR]     = bt_attack;
    n3dsbind[n3ds_A]      = bt_use;
    n3dsbind[n3ds_B]      = bt_run;
    n3dsbind[n3ds_ZL]     = bt_run;
    n3dsbind[n3ds_X]      = bt_nextweapon;
    n3dsbind[n3ds_Y]      = bt_prevweapon;
    n3dsbind[n3ds_L]      = bt_strafe;
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
// The gsp::Lcd session is opened only around each call: keeping it open
// can block the HOME Menu, which needs the same service.
//

static bool bottomon = true;
static aptHookCookie apthook;

static void SetBottomBacklight (bool on)
{
    if (R_FAILED (gspLcdInit ()))
        return;
    if (on)
        GSPLCD_PowerOnBacklight (GSPLCD_SCREEN_BOTTOM);
    else
        GSPLCD_PowerOffBacklight (GSPLCD_SCREEN_BOTTOM);
    gspLcdExit ();
}

// Never leave the HOME Menu or sleep mode with the bottom screen off.
static void AptHook (APT_HookType hook, void *)
{
    switch (hook)
    {
        case APTHOOK_ONSUSPEND:
        case APTHOOK_ONSLEEP:
        case APTHOOK_ONEXIT:
            if (!bottomon)
                SetBottomBacklight (true);
            break;
        case APTHOOK_ONRESTORE:
        case APTHOOK_ONWAKEUP:
            if (!bottomon)
                SetBottomBacklight (false);
            break;
        default:
            break;
    }
}

void N3DS_BottomScreenOn (void)
{
    if (!bottomon)
    {
        bottomon = true;
        SetBottomBacklight (true);
    }
}

static void RestoreBottomScreen (void)
{
    if (!bottomon)
        SetBottomBacklight (true);
}

void N3DS_InitBottomScreen (void)
{
    aptHook (&apthook, AptHook, NULL);
    atexit (RestoreBottomScreen);
}

void N3DS_PollBottomScreenToggle (void)
{
    static bool wastouching = false;
    bool touching = (hidKeysHeld () & KEY_TOUCH) != 0;

    // In full screen view the bottom screen shows the HUD: keep it on.
    // With touch turning, a drag in a game turns instead of toggling.
    if (touching && !wastouching && (!bottomon || !N3DS_HudVisible ())
            && !(touchturn && ingame))
    {
        bottomon = !bottomon;
        SetBottomBacklight (bottomon);
    }
    wastouching = touching;
}
