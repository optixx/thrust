
/* Written by Peter Ekberg, peda@lysator.liu.se */

#include <SDL.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "conf.h"
#include "font5x5.h"
#include "graphics.h"
#include "hiscore.h"
#include "init.h"
#include "input.h"
#include "level.h"
#include "things.h"
#include "thrust.h"
#include "helpers.h"
#include "menu.h"
#include "pref.h"
#include "game.h"

static const char thrust_opt_string[] = "vhdz:s";

static const struct option thrust_longopts[] = {
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {"nodemo", no_argument, 0, 'd'},
    {"zoom", required_argument, 0, 'z'},
    {"smooth", no_argument, 0, 's'},
    {0, 0, 0, 0}
};

#include "soundIt.h"

uint8_t* bulletmap;
uint8_t* blocks;
uint8_t* ship;
uint8_t* shieldship;
uint8_t* bana;
uint8_t* fuelmap;
uint8_t* loadmap;
uint8_t* shipstorage;
uint8_t* bulletstorage;
uint8_t* fragmentstorage;
uint8_t* fuelstorage;
uint8_t* loadstorage;
uint8_t* wirestorage;

uint32_t lenx; /* x-size of level */
uint32_t leny; /* y-size of level */
uint32_t lenx3, leny3;
/* Status of game. */
double alpha, deltaalpha;
uint32_t loaded, loadcontact, loadpointshift;
int loadpoint;
int countdown;
uint32_t crash, shoot, repetetive;
uint32_t refueling;
int speedx, speedy;
long absspeed, oldabs;
int kdir, dir;
int shipdx, shipdy;
int x, y;             /* Top left corner, 8 units per pixel. */
int pixx, pixy;       /* Top left corner in pixels.   */
int pblockx, pblocky; /* Top left corner in blocks (8x8 pixels). */
int vx, vy;           /* Speed of the ship. */
int bildx, bildy;     /* Top left corner of backing store (in pixels). */
int bblockx, bblocky; /* Top left corner of backing store (in blocks). */
int loadbx, loadby;   /* Position of the load (in blocks). */
int gravity;
int score;
uint8_t shield;
uint8_t colorr, colorg, colorb;
int nodemo = 0;
int Thrust_Is_On = 0;
double gamma_correction = 1.0;
int skip_frames = 0;

int main(int argc, char** argv)
{
    int end = 0;
    int optc;
    int enable_smooth = 0;

    window_zoom = 1;

    fprintf(stderr, "main start\n");

    do
    {
        optc = getopt_long_only(argc, argv, thrust_opt_string, thrust_longopts, NULL);
        switch (optc)
        {
        case 'd': /* --nodemo */
            nodemo = 1;
            break;
        case 'z': /* --zoom */
        {
            int z = atoi(optarg);
            if (z < 1 || z > 6)
            {
                printf("Zoom must be between 1 and 6.\n");
                exit(1);
            }
            window_zoom = z;
        }
        break;
        case 's':
            enable_smooth = 1;
            break;
        case 'h': /* --help */
            printf("Thrust: version " VERSION " -- the Game\n");
            printf("Using %s to drive the graphics and\n"
                   "      %s to drive the keyboard.\n\n",
                   graphicsname(), keyname());
            printf("usage: thrust [OPTION]...\n\n"
                   "  -v, --version\n"
                   "  -h, --help\n"
                   "  -d, --nodemo           Do not run the demo.\n"
                   "  -s, --smooth           Use linear filtering when scaling.\n"
                   "  -z, --zoom=1..6        Scale the window by an integer factor.\n");
            printf("\n");
            exit(1);
        case 'v': /* --version */
            printf("Thrust: version " VERSION "\n");
            exit(0);
        case EOF:
            if (optind == argc)
                break;
        default:
            fprintf(stderr, "Thrust: bad usage (see 'thrust -h')\n");
            exit(1);
        }
    } while (optc != EOF);

    fprintf(stderr, "main: zoom=%d smooth=%d nodemo=%d\n", window_zoom, enable_smooth, nodemo);
    graphics_set_smooth(enable_smooth);

    graphics_preinit();
    inithardware(argc, argv);

    if (!initmem())
    {
        restorehardware();
        return (1);
    }
    inithighscorelist();
    initkeys();

    SDL_Delay(1000);

    while (!end)
    {
        switch (menu())
        {
        case INST:
            instructions();
            break;
        case PLAY:
            if (!(end = game(0)))
                if (ahighscore(score))
                    newhighscore();
            break;
        case HI:
            showhighscores();
            break;
        case ABOUT:
            about();
            break;
        case CONF:
            conf();
            break;
        case DEMO:
            game(1);
            break;
        case END:
            end = 1;
            break;
        default:
            break;
        }
    }

    restoremem();
    restorehardware();

    return (0);
}
