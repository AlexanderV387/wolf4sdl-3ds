// 3DS button bindings and bottom screen control.

#include <stdlib.h>
#include <string.h>
#include "wl_def.h"
#include "n3ds_input.h"
#include "n3ds_hud.h"
#include "n3ds_keyboard.h"

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

// The keys of the keyboard on the bottom screen, as SDL key events: the
// game reads them as it would a PC keyboard's.
static void SendKey (int key, bool down)
{
    SDLKey sym;

    if (key >= 'A' && key <= 'Z')
        sym = (SDLKey) (SDLK_a + key - 'A');
    else if (key >= '0' && key <= '9')
        sym = (SDLKey) (SDLK_0 + key - '0');
    else
    {
        switch (key)
        {
            case ' ': sym = SDLK_SPACE; break;
            case '-': sym = SDLK_MINUS; break;
            case '.': sym = SDLK_PERIOD; break;
            case n3ds_keyboard::key_backspace: sym = SDLK_BACKSPACE; break;
            case n3ds_keyboard::key_enter: sym = SDLK_RETURN; break;
            case n3ds_keyboard::key_tab: sym = SDLK_TAB; break;
            case n3ds_keyboard::key_shift: sym = SDLK_LSHIFT; break;
            case n3ds_keyboard::key_ctrl: sym = SDLK_LCTRL; break;
            case n3ds_keyboard::key_alt: sym = SDLK_LALT; break;
            case n3ds_keyboard::key_f10: sym = SDLK_F10; break;
            default: return;
        }
    }

    // SHIFT gives the shifted characters.
    if (sym == SDLK_LSHIFT)
        SDL_SetModState (down ? KMOD_LSHIFT : KMOD_NONE);

    SDL_Event event;
    memset (&event, 0, sizeof (event));
    event.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.type = event.type;
    event.key.state = down ? SDL_PRESSED : SDL_RELEASED;
    event.key.keysym.sym = sym;
    SDL_PushEvent (&event);
}

void N3DS_InitBottomScreen (void)
{
    aptHook (&apthook, AptHook, NULL);
    atexit (RestoreBottomScreen);
    n3ds_keyboard::SetSendKey (SendKey);
}

void N3DS_ShowKeyboard (boolean show)
{
    n3ds_keyboard::Open (show != 0);
    if (show)
        N3DS_BottomScreenOn ();
}

void N3DS_PollBottomScreenToggle (void)
{
    static bool wastouching = false;
    bool touching = (hidKeysHeld () & KEY_TOUCH) != 0;
    touchPosition touch = {0, 0};

    if (touching)
        hidTouchRead (&touch);

    // The keyboard's tab and the open keyboard take their touches; the
    // keyboard keeps the bottom screen on.
    if (n3ds_keyboard::Update (touching, touch.px, touch.py))
    {
        if (n3ds_keyboard::IsOpen ())
            N3DS_BottomScreenOn ();
        wastouching = touching;
        return;
    }

    // The HUD (full screen view) and the map keep the bottom screen on.
    // With touch turning, a drag in a game turns instead of toggling.
    if (touching && !wastouching
            && (!bottomon || !(N3DS_HudVisible () || N3DS_MapVisible ()))
            && !(touchturn && ingame))
    {
        bottomon = !bottomon;
        SetBottomBacklight (bottomon);
    }
    wastouching = touching;
}
