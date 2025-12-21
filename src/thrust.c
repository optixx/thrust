
/* Written by Peter Ekberg, peda@lysator.liu.se */

#include <SDL.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "conf.h"
#include "graphics.h"
#include "hiscore.h"
#include "init.h"
#include "input.h"
#include "thrust.h"
#include "main_loop.h"

static const char thrust_opt_string[] = "vhdz:s";

static const struct option thrust_longopts[] = {
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {"nodemo", no_argument, 0, 'd'},
    {"zoom", required_argument, 0, 'z'},
    {"smooth", no_argument, 0, 's'},
    {0, 0, 0, 0}
};

int main(int argc, char** argv)
{
    int optc;
    int enable_smooth = 0;

    window_zoom = 1;
    world_init();

    fprintf(stderr, "main start\n");

    do
    {
        optc = getopt_long_only(argc, argv, thrust_opt_string, thrust_longopts, NULL);
        switch (optc)
        {
        case 'd': /* --nodemo */
            world_set_nodemo(1);
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

    fprintf(stderr,
            "main: zoom=%d smooth=%d nodemo=%d\n",
            window_zoom,
            enable_smooth,
            world_nodemo());
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
    run_main_loop();

    restoremem();
    restorehardware();

    return (0);
}
