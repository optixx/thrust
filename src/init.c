
/* Written by Peter Ekberg, peda@lysator.liu.se */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <math.h>

#include "font5x5.h"
#include "graphics.h"
#include "init.h"
#include "input.h"
#include "things.h"
#include "thrust.h"
#include "helpers.h"
#include "state.h"
#include "assets.h"
#include "world.h"
#include "level.h"

#include "soundIt.h"
#define NUM_SAMPLES 5
#define SAMPLE_RATE 11025
#define NUM_CHANNELS 4
#define CHAN_1 0
#define CHAN_2 1
#define CHAN_3 2
#define CHAN_4 3
Sample snd[NUM_SAMPLES];

void turnship(void)
{
    uint32_t i, j, k;
    uint8_t* ship = assets_ship();
    uint8_t* shipstorage = assets_shipstorage();

    for (k = 0; k < 4; k++)
        for (i = 0; i < 16; i++)
            for (j = 0; j < 16; j++)
                *(ship + (5 + k) * 256 + i * 16 + j) =
                    *(ship + (3 - k) * 256 + (15 - j) * 16 + 15 - i);
    for (i = 0; i < 4; i++)
    {
        memcpy(shipstorage, ship + (i << 8), 256);
        memcpy(ship + (i << 8), ship + ((8 - i) << 8), 256);
        memcpy(ship + ((8 - i) << 8), shipstorage, 256);
    }
    for (k = 0; k < 8; k++)
        for (i = 0; i < 16; i++)
            for (j = 0; j < 16; j++)
                *(ship + (9 + k) * 256 + i * 16 + j) = *(ship + (7 - k) * 256 + i * 16 + 15 - j);
    for (k = 0; k < 15; k++)
        for (i = 0; i < 16; i++)
            for (j = 0; j < 16; j++)
                *(ship + (17 + k) * 256 + i * 16 + j) =
                    *(ship + (15 - k) * 256 + (15 - i) * 16 + j);
}

void makeshieldedship(void)
{
    uint32_t i, j, k;
    uint8_t* ship = assets_ship();
    uint8_t* shieldship = assets_shieldship();

    memcpy(shieldship, ship, 8192);
    for (i = 0; i < 32; i++)
        for (j = 0; j < 16; j++)
            for (k = 0; k < 16; k++)
                if (*(bin_shld + (j << 4) + k))
                    *(shieldship + (i << 8) + (j << 4) + k) = *(bin_shld + (j << 4) + k);
}

void makefuelmap(uint8_t* fuelmap)
{
    int i;

    memset(fuelmap, 0, 64 * 4);

    for (i = 0; i < 32; i++)
    {
        *(fuelmap + 4 * (i + 32) + (i > 6) + (i > 16) + (i > 26)) = 255;
        *(fuelmap + 4 * i + (i < 27) + (i < 17) + (i < 7)) = 255;
    }
}

int initmem(void)
{
    uint32_t i;

    printf("Allocating memory...");

    bild = (uint8_t*)malloc((long)PBILDX * PBILDY * 2 + 16);
    uint8_t* buf = (uint8_t*)malloc(maxlenx * maxleny);
    level_set_buffer(buf);

    if (!bild || !buf)
    {
        printf("failed!.\n");
        return (0);
    }

    if (!assets_allocate())
    {
        printf("failed!.\n");
        return (0);
    }

    printf("done.\n");

    uint8_t* ship = assets_ship();
    uint8_t* bulletmap = assets_bulletmap();
    uint8_t* fuelmap = assets_fuelmap();
    uint8_t* loadmap = assets_loadmap();
    uint8_t* blocks = assets_blocks();

    memcpy(ship, bin_ship, 256 * 5);
    for (i = 0; i < 16; i++)
        memcpy(bulletmap + ((20 - i) & 15) * 16, bin_bullet + i * 16, 16);

    for (i = 0; i < title_cols * title_rows; i++)
        *(title_pixels + i) += 192;

    memcpy(bin_colors + 192 * 3, title_colors, title_nr_colors * 3);

    for (i = 0; i < 3 * 256; i++)
        bin_colors[i] = GAMMA(bin_colors[i]);

    printf("Turning the ship...");
    turnship();
    printf("done.\n");
    printf("Building graphics...");
    makefuelmap(fuelmap);
    memcpy(loadmap, blocks + 64 * 109, 64);
    makeshieldedship();
    printf("done.\n");

    return (1);
}

void inithardware(int argc, char** argv)
{
    if (initsoundIt())
        printf("No sound.\n");

    if (graphicsinit(window_zoom))
        exit(-1);

    if (keyinit())
    {
        printf("Could not initialize the keyboard.\n");
        exit(-1);
    }
    printf("Keyboard initialized.\n");
}

void initscreen(int round)
{
    int i, j;

    if (round & 2)
    {
        bin_colors[65 * 3 + 0] = 0;
        bin_colors[65 * 3 + 1] = 0;
        bin_colors[65 * 3 + 2] = 0;
    }
    else
    {
        bin_colors[65 * 3 + 0] = GAMMA(world_state()->colorr);
        bin_colors[65 * 3 + 1] = GAMMA(world_state()->colorg);
        bin_colors[65 * 3 + 2] = GAMMA(world_state()->colorb);
    }

    for (j = world_state()->pblocky; j < BBILDY + world_state()->pblocky; j++)
        for (i = world_state()->pblockx; i < BBILDX + world_state()->pblockx; i++)
            writeblock(i % world_state()->lenx, j,
                       *(level_buffer() + i % world_state()->lenx + j * world_state()->lenx));
}

void initgame(int round, int reset, int xblock, int yblock)
{
    int i;
    world_state_t* world = world_state();

    world->crash = 0;
    world->shoot = 0;
#ifdef DEBUG
    world->repetetive = 1;
#else
    world->repetetive = 0;
#endif
    world->refueling = 0;
    world->speedx = 0;
    world->speedy = 0;
    world->absspeed = 0L;
    world->oldabs = 0L;
    world->vx = 0;
    world->vy = 0;
    int gravity_setting;
    if (round & 1)
    {
        world->kdir = 72;
        world->dir = 24;
        gravity_setting = -20;
        world->alpha = 3 * M_PI / 2;
        world->deltaalpha = 0;
    }
    else
    {
        world->kdir = 24;
        world->dir = 8;
        gravity_setting = 20;
        world->alpha = M_PI / 2;
        world->deltaalpha = 0;
    }
    game_state_t* state = state_current();
    if (state)
        state->gravity = gravity_setting;
    if (reset)
    {
        world_set_loaded(0);
        world_set_load_contact(0);
        world->loadpoint = 0;
        world->loadpointshift = 0;
        world->shipdx = 0;
        world->shipdy = 0;
    }
    else
    {
        world_set_load_contact(0);
        if (world_is_loaded())
        {
            world->loadpoint = 126;
            world->loadpointshift = 0;
            world->shipdx = (int)(cos(world->alpha) * world->loadpoint / 5.90625);
            world->shipdy = (int)(-sin(world->alpha) * world->loadpoint / 5.90625);
        }
        else
        {
            world->loadpoint = 0;
            world->loadpointshift = 0;
            *(level_buffer() + world->lenx * world->loadby + world->loadbx) = 109;
            world->shipdx = 0;
            world->shipdy = 0;
        }
    }

    world->pblockx = xblock;
    world->pblocky = yblock + 4 * (round & 1);
    if (world_is_loaded())
    {
        if (round & 1)
            world->pblocky -= 2;
        else
            world->pblocky += 2;
    }

    world->pixx = world->pblockx << 3;
    world->pixy = world->pblocky << 3;
    world->x = world->pixx << 3;
    world->y = world->pixy << 3;
    world->bildx = (world->pixx + PBILDX - 4) % PBILDX + 4;
    world->bildy = world->pixy % PBILDY;
    world->bblockx = world->bildx >> 3;
    world->bblocky = world->bildy >> 3;

    world->countdown = 0;

    for (i = 0; i < maxbullets; i++)
        bullets[i].life = 0;
    for (i = 0; i < maxfragments; i++)
        fragments[i].life = 0;

    chcolor = TEXTCOLOR;
    chpaper = 0;
    chflag = 0;
}

int initsoundIt(void)
{
    printf("Initializing soundIt library v" SOUNDIT_VERS "...");
    fflush(stdout);

    snd[0].data = sound_boom;
    snd[0].len = sound_boom_len;
    snd[0].loop = 0;
    snd[1].data = sound_boom2;
    snd[1].len = sound_boom2_len;
    snd[1].loop = 0;
    snd[2].data = sound_harp;
    snd[2].len = sound_harp_len;
    snd[2].loop = 0;
    snd[3].data = sound_thrust;
    snd[3].len = sound_thrust_len;
    snd[3].loop = 1;
    snd[4].data = sound_zero;
    snd[4].len = sound_zero_len;
    snd[4].loop = 0;

    if (Snd_init(NUM_SAMPLES, snd, SAMPLE_RATE, NUM_CHANNELS) == EXIT_FAILURE)
    {
        printf("No sound.\n");
        return (-1);
    }

    printf("done.\n");
    return (0);
}

void restorehardware(void)
{
    printf("Releasing keyboard...");
    keyclose();
    printf("done.\n");

    graphicsclose();

    Snd_restore();
}

void restoremem(void)
{
    printf("Freeing allocated memory...");
    assets_cleanup();
    free(bild);
    level_release_buffer();
    printf("done.\n");
}
