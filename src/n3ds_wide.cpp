// [3DS] The top screen at 800x240 (see n3ds_wide.h).

#include <string.h>
#include <3ds.h>
#include "wl_def.h"
#include "n3ds_wide.h"

boolean widescreen800 = true;
bool n3ds_wide_view_ready;

static const int topwidth = 800;
static const int topheight = 240;

bool N3DS_Wide (void)
{
    // Chosen once: the buffers and the video mode are made for it.
    static int wide = -1;

    if (wide < 0)
    {
        u8 model = CFG_MODEL_3DS;

        if (R_SUCCEEDED (cfguInit ()))
        {
            CFGU_GetSystemModel (&model);
            cfguExit ();
        }

        wide = widescreen800 && model != CFG_MODEL_2DS;
    }

    return wide != 0;
}

// Two views, one drawn while core 2 shows the other.
static byte *views[2];
static int drawnview;

byte *N3DS_WideView (void)
{
    if (!views[0])
    {
        views[0] = (byte *) calloc (topwidth * topheight, 1);
        views[1] = (byte *) calloc (topwidth * topheight, 1);
        CHECKMALLOCRESULT (views[0]);
        CHECKMALLOCRESULT (views[1]);
    }

    return views[drawnview];
}

// ==========================================================================
// Video mode: SDL draws the bottom screen; the game, both screens' rows in
// one 400x480 surface as before.

static SDL_Surface *bottomscreen;

SDL_Surface *N3DS_WideSetVideoMode (void)
{
    bottomscreen = SDL_SetVideoMode (screenWidth, 240, 32, SDL_BOTTOMSCR | SDL_DOUBLEBUF);

    if (!bottomscreen)
        return NULL;

    const SDL_PixelFormat *f = bottomscreen->format;
    SDL_Surface *both = SDL_CreateRGBSurface (SDL_SWSURFACE, screenWidth, 480, 32,
        f->Rmask, f->Gmask, f->Bmask, f->Amask);

    // The top screen ours: ToRGB565 (half the bytes to write) and 800 wide.
    gfxSetScreenFormat (GFX_TOP, GSP_RGB565_OES);
    gfxSetWide (true);
    return both;
}

// ==========================================================================
// The top screen, made on core 2 (the New 3DS's third core, with no time
// limit: on core 1 a frame of 800x240 took 18 ms in Blake Stone, 6 there).

static Thread worker;
static LightEvent workgo, workdone;
static bool workbusy;

static struct
{
    u32 *rows;                  // the top 400x240 of the screen surface
    int pitch;                  // in pixels
    int rshift, gshift, bshift;
    bool view_ready;
    const byte *view;
    int vx0, vx1, vy0, vy1;     // the view's rectangle, 800 across
    u16 colors[256];            // the view's palette, RGB565
} job;

static u32 *rowscopy;

// [test] Pacing variants, each for 600 frames: 1 the vertical blank waited
// on the main thread; 2 waited by core 2 after showing the frame; 3 none;
// 4 as 1 with the bottom screen sent every other frame.
int n3ds_pace_variant = 1;
int n3ds_flip_ms, n3ds_top_ms, n3ds_wait_ms;
static u64 fliptks, toptks, waittks;
static int timedframes;
static const u64 tickspms = 268111;

static u64 parttks[WT_COUNT];
static u64 logstart, lastframe, framemax;
static int logframes, slow17, slow34;

void N3DS_WideTime (int part, unsigned long long ticks)
{
    parttks[part] += ticks;
}

static void WriteLog (void)
{
    static bool started;
    FILE *f = fopen ("/3ds/wolf4sdl/wide-log.txt", started ? "a" : "w");

    if (f)
    {
        if (!started)
            fprintf (f, "variant frames fps | ms per frame: draw wideend hud flip wait vblank"
                        " top(core2) | worst ms, frames over 17 ms, over 34 ms\n");
        started = true;

        const double n = logframes, ms = tickspms;
        fprintf (f, "%d %d %.1f |", n3ds_pace_variant, logframes,
                 n * 1000.0 * ms / (double) (lastframe - logstart));
        static const int order[] = { WT_DRAW, WT_WIDEEND, WT_HUD, WT_FLIP, WT_WAIT, WT_VBLANK, WT_TOP };
        for (int i : order)
            fprintf (f, " %.1f", parttks[i] / n / ms);
        fprintf (f, " | %.1f %d %d\n", framemax / ms, slow17, slow34);
        fclose (f);
    }

    memset (parttks, 0, sizeof (parttks));
    framemax = 0;
    logframes = slow17 = slow34 = 0;
}

void N3DS_WideFrameDone (void)
{
    const u64 now = svcGetSystemTick ();

    if (lastframe)
    {
        const u64 t = now - lastframe;
        if (t > framemax)
            framemax = t;
        slow17 += t > 17 * tickspms;
        slow34 += t > 34 * tickspms;
        logframes++;
    }
    else
    {
        logstart = now;
    }
    lastframe = now;

    // The next variant every 600 frames, after logging this one (the
    // writing's own time falls outside both).
    if (logframes == 600)
    {
        WriteLog ();
        n3ds_pace_variant = n3ds_pace_variant % 4 + 1;
        lastframe = logstart = svcGetSystemTick ();
    }
}

static inline u16 ToRGB565 (unsigned r, unsigned g, unsigned b)
{
    return (u16) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static void MakeTop (void)
{
    const u64 start = svcGetSystemTick ();
    u16 *fb = (u16 *) gfxGetFramebuffer (GFX_TOP, GFX_LEFT, NULL, NULL);
    const int block = 8;

    for (int bx = 0; bx < topwidth; bx += block)
    {
        for (int by = 0; by < topheight; by += block)
        {
            for (int x = bx; x < bx + block; ++x)
            {
                u16 *dst = &fb[x * topheight + (topheight - 1 - by)];
                const bool inview = job.view_ready && x >= job.vx0 && x < job.vx1;

                for (int y = by; y < by + block; ++y)
                {
                    if (inview && y >= job.vy0 && y < job.vy1)
                    {
                        *dst-- = job.colors[job.view[y * topwidth + x]];
                    }
                    else
                    {
                        const u32 p = job.rows[y * job.pitch + (x >> 1)];
                        *dst-- = ToRGB565 ((p >> job.rshift) & 0xFF, (p >> job.gshift) & 0xFF,
                                         (p >> job.bshift) & 0xFF);
                    }
                }
            }
        }
    }

    GSPGPU_FlushDataCache (fb, topwidth * topheight * 2);
    gfxScreenSwapBuffers (GFX_TOP, false);
    toptks += svcGetSystemTick () - start;
    parttks[WT_TOP] += svcGetSystemTick () - start;

    if (n3ds_pace_variant == 2)
        gspWaitForVBlank ();
}

static void WorkerMain (void *)
{
    for (;;)
    {
        LightEvent_Wait (&workgo);
        MakeTop ();
        LightEvent_Signal (&workdone);
    }
}

static void WaitTop (void)
{
    if (workbusy)
    {
        LightEvent_Wait (&workdone);
        workbusy = false;
    }
}

void N3DS_WidePresent (SDL_Surface *screen, SDL_Surface *source, bool view_ready)
{
    static bool tried;

    if (!tried)
    {
        tried = true;
        LightEvent_Init (&workgo, RESET_ONESHOT);
        LightEvent_Init (&workdone, RESET_ONESHOT);
        worker = threadCreate (WorkerMain, NULL, 32 * 1024, 0x18, 2, true);

        if (!worker)
        {
            if (R_FAILED (APT_SetAppCpuTimeLimit (80)))
                APT_SetAppCpuTimeLimit (30);
            worker = threadCreate (WorkerMain, NULL, 32 * 1024, 0x18, 1, true);
        }
    }

    // The bottom screen: its rows into SDL's surface.
    static unsigned frame;
    frame++;
    const u64 flipstart = svcGetSystemTick ();

    if (bottomscreen && (n3ds_pace_variant != 4 || (frame & 1)))
    {
        const int bytes = screen->w * 4;

        for (int y = 0; y < 240; y++)
            memcpy ((byte *) bottomscreen->pixels + y * bottomscreen->pitch,
                    (byte *) screen->pixels + (240 + y) * screen->pitch, bytes);
        SDL_Flip (bottomscreen);
    }

    if (aptShouldClose ())
        return;

    const u64 waitstart = svcGetSystemTick ();
    fliptks += waitstart - flipstart;
    parttks[WT_FLIP] += waitstart - flipstart;

    // Core 2 done with the last frame: its copies are free.
    WaitTop ();
    waittks += svcGetSystemTick () - waitstart;
    parttks[WT_WAIT] += svcGetSystemTick () - waitstart;

    if (++timedframes == 60)
    {
        n3ds_flip_ms = (int) (fliptks / 60 / tickspms);
        n3ds_top_ms = (int) (toptks / 60 / tickspms);
        n3ds_wait_ms = (int) (waittks / 60 / tickspms);
        fliptks = toptks = waittks = 0;
        timedframes = 0;
    }

    const SDL_PixelFormat *f = screen->format;
    job.pitch = screen->pitch / 4;
    job.rshift = f->Rshift;
    job.gshift = f->Gshift;
    job.bshift = f->Bshift;
    job.view_ready = view_ready;
    job.vx0 = viewscreenx * 2;
    job.vx1 = (viewscreenx + viewwidth) * 2;
    job.vy0 = viewscreeny;
    job.vy1 = viewscreeny + viewheight;

    // What the view does not cover, from the screen surface (copied: the
    // game draws the next frame into it meanwhile).
    const bool fullview = view_ready && job.vx0 <= 0 && job.vx1 >= topwidth
                          && job.vy0 <= 0 && job.vy1 >= topheight;

    if (!fullview)
    {
        if (!rowscopy)
            rowscopy = (u32 *) malloc (screen->pitch * topheight);
        if (rowscopy)
            memcpy (rowscopy, screen->pixels, screen->pitch * topheight);
        job.rows = rowscopy ? rowscopy : (u32 *) screen->pixels;
    }

    if (view_ready)
    {
        const SDL_Color *c = source->format->palette->colors;

        for (int i = 0; i < 256; i++)
            job.colors[i] = ToRGB565 (c[i].r, c[i].g, c[i].b);

        // This view goes to core 2; the next frame draws into the other.
        job.view = views[drawnview];
        drawnview ^= 1;
    }

    if (worker)
    {
        workbusy = true;
        LightEvent_Signal (&workgo);
    }
    else
    {
        MakeTop ();
    }
}
