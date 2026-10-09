// Game picker for the combined Wolf4SDL 3DS build.
//
// Wolf4SDL chooses the game when it is compiled (Wolfenstein 3D or Spear of
// Destiny, full or shareware/demo), so the combined build links the four
// builds into one program, each with its symbols renamed
// (tools/3ds/build-combined.sh): w3d_main, w3s_main, sod_main, sdm_main.
// This picker lists the games whose data is on the SD card and starts one.

#include <stdio.h>
#include <sys/stat.h>
#include <3ds.h>

// main has C linkage, so the renamed ones do too.
extern "C"
{
    int w3d_main (int argc, char *argv[]);
    int w3s_main (int argc, char *argv[]);
    int sod_main (int argc, char *argv[]);
    int sdm_main (int argc, char *argv[]);
}

typedef int (*GameMain) (int argc, char *argv[]);

struct Game
{
    const char *name;
    const char *data[2];    // any of these files means the game is there
    GameMain main;
    const char *mission;    // Spear of Destiny mission (--mission), or NULL.
                            // "0" is *.sod, or *.sd1 when there is no *.sod.
};

static const Game games[] =
{
    {"Wolfenstein 3D",
        {"/3ds/wolf4sdl/wolf3d/vswap.wl6", NULL}, w3d_main, NULL},
    {"Wolfenstein 3D (shareware)",
        {"/3ds/wolf4sdl/wolf3d/vswap.wl1", NULL}, w3s_main, NULL},
    {"Spear of Destiny",
        {"/3ds/wolf4sdl/sod/vswap.sod", "/3ds/wolf4sdl/sod/vswap.sd1"}, sod_main, "0"},
    {"SoD Mission 2: Return to Danger",
        {"/3ds/wolf4sdl/sod/vswap.sd2", NULL}, sod_main, "2"},
    {"SoD Mission 3: Ultimate Challenge",
        {"/3ds/wolf4sdl/sod/vswap.sd3", NULL}, sod_main, "3"},
    {"Spear of Destiny (demo)",
        {"/3ds/wolf4sdl/sod/vswap.sdm", NULL}, sdm_main, NULL},
};

static const int numgames = sizeof (games) / sizeof (games[0]);

static bool Exists (const char *path)
{
    struct stat st;
    return path && stat (path, &st) == 0;
}

static bool HasData (const Game &game)
{
    return Exists (game.data[0]) || Exists (game.data[1]);
}

// Text menu on the top screen. Returns the game index, or -1 to quit.
static int PickGame (const int *available, int count)
{
    gfxInitDefault ();
    PrintConsole console;
    consoleInit (GFX_TOP, &console);

    int selected = 0, drawn = -1, result = -1;
    while (aptMainLoop ())
    {
        hidScanInput ();
        u32 down = hidKeysDown ();

        if (count == 0)
        {
            if (drawn != 0)
            {
                consoleClear ();
                printf ("\n  WOLF4SDL 3DS\n\n"
                        "  No game data found. Copy it to:\n\n"
                        "  Wolfenstein 3D:\n"
                        "    /3ds/wolf4sdl/wolf3d/  (*.wl6 or *.wl1)\n\n"
                        "  Spear of Destiny:\n"
                        "    /3ds/wolf4sdl/sod/     (*.sod, *.sd2, *.sd3\n"
                        "                            or *.sdm)\n\n"
                        "  START: exit\n");
                drawn = 0;
            }
            if (down & KEY_START)
                break;
        }
        else
        {
            if (down & (KEY_DOWN | KEY_CPAD_DOWN))
                selected = (selected + 1) % count;
            if (down & (KEY_UP | KEY_CPAD_UP))
                selected = (selected + count - 1) % count;
            if (down & KEY_A)
            {
                result = available[selected];
                break;
            }
            if (down & KEY_START)
                break;

            if (selected != drawn)
            {
                consoleClear ();
                printf ("\n  WOLF4SDL 3DS\n\n  Choose a game:\n\n");
                for (int i = 0; i < count; i++)
                    printf ("  %s %s\n\n", i == selected ? ">" : " ",
                            games[available[i]].name);
                printf ("\n  Up/Down: choose  A: start  START: exit\n");
                drawn = selected;
            }
        }
        gspWaitForVBlank ();
    }

    gfxExit ();     // SDL sets the screens up again
    return result;
}

int main (int argc, char *argv[])
{
    int available[numgames], count = 0;
    for (int i = 0; i < numgames; i++)
        if (HasData (games[i]))
            available[count++] = i;

    // With a single game there is nothing to choose.
    int game = (count == 1) ? available[0] : PickGame (available, count);
    if (game < 0)
        return 0;

    const char *program = argc > 0 ? argv[0] : "wolf4sdl";
    char *args[4] = {(char *) program, NULL, NULL, NULL};
    int numargs = 1;
    if (games[game].mission)
    {
        // Wolf4SDL's mission 0 only looks for *.sod; mission 1 is *.sd1.
        const char *mission = games[game].mission;
        if (mission[0] == '0' && !Exists ("/3ds/wolf4sdl/sod/vswap.sod"))
            mission = "1";
        args[numargs++] = (char *) "--mission";
        args[numargs++] = (char *) mission;
    }
    return games[game].main (numargs, args);
}
