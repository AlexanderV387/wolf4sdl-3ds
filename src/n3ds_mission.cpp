// Spear of Destiny mission picker for 3DS.

#include <stdio.h>
#include <sys/stat.h>
#include "wl_def.h"
#include "n3ds_mission.h"

static bool HasMission (const char *ext)
{
    char path[64];
    struct stat st;
    snprintf (path, sizeof (path), DATADIR "vswap.%s", ext);
    return stat (path, &st) == 0;
}

int N3DS_ChooseSpearMission (void)
{
    static const char *names[4] =
    {
        "Spear of Destiny",
        "Spear of Destiny",
        "Mission 2: Return to Danger",
        "Mission 3: Ultimate Challenge"
    };
    int missions[3], count = 0;

    if (HasMission ("sod"))
        missions[count++] = 0;
    else if (HasMission ("sd1"))
        missions[count++] = 1;
    if (HasMission ("sd2"))
        missions[count++] = 2;
    if (HasMission ("sd3"))
        missions[count++] = 3;

    if (count <= 1)                 // nothing to choose (0 reports missing data)
        return count ? missions[0] : 0;

    // SDL is not started yet: use a libctru text console, then hand the
    // screens back to SDL.
    gfxInitDefault ();
    PrintConsole console;
    consoleInit (GFX_TOP, &console);

    int selected = 0, drawn = -1;
    while (aptMainLoop ())
    {
        hidScanInput ();
        u32 down = hidKeysDown ();
        if (down & KEY_DOWN)
            selected = (selected + 1) % count;
        if (down & KEY_UP)
            selected = (selected + count - 1) % count;
        if (down & KEY_A)
            break;

        if (selected != drawn)
        {
            consoleClear ();
            printf ("\n  SPEAR OF DESTINY\n\n  Choose a mission:\n\n");
            for (int i = 0; i < count; i++)
                printf ("  %s %s\n\n", i == selected ? ">" : " ", names[missions[i]]);
            printf ("\n  Up/Down: choose    A: start\n");
            drawn = selected;
        }
        gspWaitForVBlank ();
    }

    gfxExit ();
    return missions[selected];
}
