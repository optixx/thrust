#include <SDL.h>

#include "game.h"
#include "hiscore.h"
#include "input.h"
#include "menu_state.h"
#include "main_loop.h"
#include "demo_state.h"
#include "screen_state.h"
#include "pref.h"
#include "thrust.h"

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

static void advance_frame(uint32_t* frame_target)
{
    uint32_t now = SDL_GetTicks();
    if (*frame_target > now)
        SDL_Delay(*frame_target - now);
    *frame_target = now + 20;
}

int
run_main_loop(void)
{
    main_state_t current_state = MAIN_STATE_MENU;
    const uint32_t frame_interval = 20;
    uint32_t frame_target = SDL_GetTicks() + frame_interval;

    menu_state_init();
    screen_state_init();
    demo_state_init();

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

        advance_frame(&frame_target);
    }

    return 0;
}
