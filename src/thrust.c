
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
#include "menu_state.h"
#include "demo_state.h"
#include "screen_state.h"
#include "menu_state.h"
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

typedef enum
{
    MAIN_STATE_MENU,
    MAIN_STATE_INSTRUCTIONS,
    MAIN_STATE_PLAY,
    MAIN_STATE_HIGH_SCORES,
    MAIN_STATE_ABOUT,
    MAIN_STATE_CONFIG,
    MAIN_STATE_DEMO,
    MAIN_STATE_QUIT
} main_state_t;

static main_state_t handle_menu_choice(options choice);

int main(int argc, char** argv)
{
    int optc;
    int enable_smooth = 0;
    main_state_t current_state = MAIN_STATE_MENU;

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
    menu_state_init();
    screen_state_init();
    demo_state_init();

    const uint32_t frame_interval = 20;
    uint32_t frame_target = SDL_GetTicks() + frame_interval;

    while (current_state != MAIN_STATE_QUIT)
    {
        input_frame_tick();
        switch (current_state)
        {
        case MAIN_STATE_MENU:
            current_state = handle_menu_choice(menu_state_run());
            break;
        case MAIN_STATE_INSTRUCTIONS:
            screen_state_run(INST);
            input_flush_events();
            current_state = MAIN_STATE_MENU;
            break;
        case MAIN_STATE_PLAY:
            if (game(0) != 0)
                current_state = MAIN_STATE_QUIT;
            else
            {
                if (ahighscore(game_last_score()))
                    newhighscore();
                input_flush_events();
                current_state = MAIN_STATE_MENU;
            }
            break;
        case MAIN_STATE_HIGH_SCORES:
            screen_state_run(HI);
            input_flush_events();
            current_state = MAIN_STATE_MENU;
            break;
        case MAIN_STATE_ABOUT:
            screen_state_run(ABOUT);
            input_flush_events();
            current_state = MAIN_STATE_MENU;
            break;
        case MAIN_STATE_CONFIG:
            screen_state_run(CONF);
            input_flush_events();
            current_state = MAIN_STATE_MENU;
            break;
        case MAIN_STATE_DEMO:
            demo_state_run();
            input_flush_events();
            current_state = MAIN_STATE_MENU;
            break;
        case MAIN_STATE_QUIT:
        default:
            current_state = MAIN_STATE_QUIT;
            break;
        }

        uint32_t now = SDL_GetTicks();
        if (frame_target > now)
            SDL_Delay(frame_target - now);
        frame_target = now + frame_interval;
    }

    restoremem();
    restorehardware();

    return (0);
}

static main_state_t handle_menu_choice(options choice)
{
    switch (choice)
    {
    case INST:
        return MAIN_STATE_INSTRUCTIONS;
    case PLAY:
        return MAIN_STATE_PLAY;
    case HI:
        return MAIN_STATE_HIGH_SCORES;
    case ABOUT:
        return MAIN_STATE_ABOUT;
    case CONF:
        return MAIN_STATE_CONFIG;
    case DEMO:
        return MAIN_STATE_DEMO;
    case END:
        return MAIN_STATE_QUIT;
    default:
        return MAIN_STATE_MENU;
    }
}
