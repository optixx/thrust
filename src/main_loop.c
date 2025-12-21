#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

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

typedef struct
{
    main_state_t current_state;
    uint32_t frame_interval;
    uint32_t frame_target;
} main_loop_context_t;

static main_loop_context_t g_main_loop_ctx;

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

static void advance_frame(main_loop_context_t* ctx)
{
    uint32_t now = SDL_GetTicks();
#ifndef __EMSCRIPTEN__
    if (ctx->frame_target > now)
        SDL_Delay(ctx->frame_target - now);
#endif
    ctx->frame_target = now + ctx->frame_interval;
}

static void handle_state_transition(main_loop_context_t* ctx)
{
    switch (ctx->current_state)
    {
    case MAIN_STATE_MENU:
        ctx->current_state = handle_menu_choice(menu_state_run());
        break;
    case MAIN_STATE_INSTRUCTIONS:
        screen_state_run(INST);
        input_flush_events();
        ctx->current_state = MAIN_STATE_MENU;
        break;
    case MAIN_STATE_PLAY:
        if (game(0) != 0)
            ctx->current_state = MAIN_STATE_QUIT;
        else
        {
            if (ahighscore(game_last_score()))
                newhighscore();
            input_flush_events();
            ctx->current_state = MAIN_STATE_MENU;
        }
        break;
    case MAIN_STATE_HIGH_SCORES:
        screen_state_run(HI);
        input_flush_events();
        ctx->current_state = MAIN_STATE_MENU;
        break;
    case MAIN_STATE_ABOUT:
        screen_state_run(ABOUT);
        input_flush_events();
        ctx->current_state = MAIN_STATE_MENU;
        break;
    case MAIN_STATE_CONFIG:
        screen_state_run(CONF);
        input_flush_events();
        ctx->current_state = MAIN_STATE_MENU;
        break;
    case MAIN_STATE_DEMO:
        demo_state_run();
        input_flush_events();
        ctx->current_state = MAIN_STATE_MENU;
        break;
    case MAIN_STATE_QUIT:
    default:
        ctx->current_state = MAIN_STATE_QUIT;
        break;
    }
}

static void main_loop_iteration(void* user_data)
{
    main_loop_context_t* ctx = user_data;

    if (ctx->current_state == MAIN_STATE_QUIT)
    {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    input_frame_tick();
    handle_state_transition(ctx);

    if (ctx->current_state == MAIN_STATE_QUIT)
    {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    advance_frame(ctx);
}

int
run_main_loop(void)
{
    menu_state_init();
    screen_state_init();
    demo_state_init();

    g_main_loop_ctx.current_state = MAIN_STATE_MENU;
    g_main_loop_ctx.frame_interval = 20;
    g_main_loop_ctx.frame_target = SDL_GetTicks() + g_main_loop_ctx.frame_interval;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop_iteration, &g_main_loop_ctx, 0, 1);
    return 0;
#else
    while (g_main_loop_ctx.current_state != MAIN_STATE_QUIT)
        main_loop_iteration(&g_main_loop_ctx);
    return 0;
#endif
}
