#include <SDL.h>
#include <stdbool.h>
#include "input.h"

typedef struct
{
    uint8_t keybits;
    SDL_Keycode last_key;
    int quit_requested;
    input_actions_t actions;
} input_state_t;

int scancode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static input_state_t state;
static char driver_name[] = "SDL2";

static void update_keybits(void);
static void pump_events(void);
static void handle_key(SDL_Keycode key, int down);
static SDL_Keycode consume_last_key(void);

const char*
input_driver_name(void)
{
    return driver_name;
}

int
input_init(void)
{
    input_flush_events();
    return 0;
}

int
input_shutdown(void)
{
    return 0;
}

uint8_t
input_get_action_bits(void)
{
    input_frame_tick();
    return state.keybits;
}

int
input_has_pending_events(void)
{
    pump_events();
    return (state.last_key != 0) || state.quit_requested;
}

SDL_Keycode
input_wait_key(void)
{
    SDL_Event ev;
    for (;;)
    {
        if (SDL_WaitEvent(&ev) == 0)
            continue;

        if (ev.type == SDL_KEYDOWN)
        {
            if (ev.key.repeat)
                continue;
            state.last_key = ev.key.keysym.sym;
            handle_key(ev.key.keysym.sym, 1);
            update_keybits();
            return state.last_key;
        }
        if (ev.type == SDL_KEYUP)
        {
            handle_key(ev.key.keysym.sym, 0);
            update_keybits();
            continue;
        }
        if (ev.type == SDL_QUIT)
        {
            state.quit_requested = 1;
            update_keybits();
            return SDLK_ESCAPE;
        }
    }
}

void
input_flush_events(void)
{
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
    state.keybits = 0;
    state.last_key = 0;
    state.quit_requested = 0;
    state.actions = (input_actions_t){0};
}

const input_actions_t*
input_actions(void)
{
    return &state.actions;
}

void
input_frame_tick(void)
{
    pump_events();
}

uint8_t
input_cached_action_bits(void)
{
    return state.keybits;
}

const char*
input_keyname(SDL_Keycode key)
{
    const char* name = SDL_GetKeyName(key);
    return name ? name : "unknown";
}

SDL_Keycode
input_keycode(const char* name)
{
    if (!name)
        return SDLK_UNKNOWN;
    return SDL_GetKeyFromName(name);
}

char*
keyname(void)
{
    return driver_name;
}

int
keyinit(void)
{
    return input_init();
}

int
keyclose(void)
{
    return input_shutdown();
}

char*
keystring(int key)
{
    return (char*)input_keyname((SDL_Keycode)key);
}

int
keycode(char* keyname)
{
    return (int)input_keycode(keyname);
}

int
getkey(void)
{
    input_frame_tick();
    SDL_Keycode key = consume_last_key();
    if (key)
        return (int)key;
    if (state.quit_requested)
        return SDLK_ESCAPE;
    return 0;
}

int
keywaiting(void)
{
    return input_has_pending_events();
}

int
wait_for_key(void)
{
    return (int)input_wait_key();
}

uint8_t
getkeys(void)
{
    return input_cached_action_bits();
}

uint8_t
whatkeys(void)
{
    return getkeys();
}

void
flushkeyboard(void)
{
    input_flush_events();
}

static SDL_Keycode
consume_last_key(void)
{
    SDL_Keycode key = state.last_key;
    state.last_key = 0;
    return key;
}

SDL_Keycode
input_consume_key(void)
{
    return consume_last_key();
}

static void
handle_key(SDL_Keycode key, int down)
{
    switch (key)
    {
    case SDLK_p:
        state.actions.pause = down;
        break;
    case SDLK_q:
    case SDLK_ESCAPE:
        state.actions.escape = down;
        break;
    case SDLK_a:
    case SDLK_LEFT:
        state.actions.left = down;
        break;
    case SDLK_s:
    case SDLK_RIGHT:
        state.actions.right = down;
        break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
    case SDLK_UP:
        state.actions.thrust = down;
        break;
    case SDLK_RETURN:
        state.actions.fire = down;
        break;
    case SDLK_SPACE:
        state.actions.pickup = down;
        break;
    default:
        break;
    }
}

static void
update_keybits(void)
{
    uint8_t bits = 0;
    if (state.actions.pause)
        bits |= pause_bit;
    if (state.actions.escape)
        bits |= escape_bit;
    if (state.actions.left)
        bits |= left_bit;
    if (state.actions.right)
        bits |= right_bit;
    if (state.actions.thrust)
        bits |= thrust_bit;
    if (state.actions.fire)
        bits |= fire_bit;
    if (state.actions.pickup)
        bits |= pickup_bit;
    if (state.quit_requested)
        bits |= escape_bit;
    state.keybits = bits;
}

static void
pump_events(void)
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_KEYDOWN)
        {
            if (!ev.key.repeat)
                state.last_key = ev.key.keysym.sym;
            handle_key(ev.key.keysym.sym, 1);
        }
        else if (ev.type == SDL_KEYUP)
        {
            handle_key(ev.key.keysym.sym, 0);
        }
        else if (ev.type == SDL_QUIT)
        {
            state.quit_requested = 1;
        }
    }
    update_keybits();
}
