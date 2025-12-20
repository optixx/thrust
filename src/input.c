#include <SDL.h>
#include "input.h"

typedef struct
{
    uint8_t keybits;
    SDL_Keycode last_key;
    int quit_requested;
} input_state_t;

int scancode[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static input_state_t state;
static char driver_name[] = "SDL2";

static void update_keybits(void);
static void pump_events(void);
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
    pump_events();
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
    SDL_Keycode key;

    for (;;)
    {
        pump_events();
        key = consume_last_key();
        if (key)
            return key;
        if (state.quit_requested)
            return SDLK_ESCAPE;

        if (SDL_WaitEventTimeout(&ev, 50))
        {
            if (ev.type == SDL_KEYDOWN)
            {
                state.last_key = ev.key.keysym.sym;
                update_keybits();
                key = consume_last_key();
                if (key)
                    return key;
            }
            else if (ev.type == SDL_QUIT)
            {
                state.quit_requested = 1;
                return SDLK_ESCAPE;
            }
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
    pump_events();
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
    return input_get_action_bits();
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

static void
update_keybits(void)
{
    const Uint8* keyboard = SDL_GetKeyboardState(NULL);
    uint8_t bits = 0;

    if (keyboard[SDL_SCANCODE_P])
        bits |= pause_bit;
    if (keyboard[SDL_SCANCODE_Q] || keyboard[SDL_SCANCODE_ESCAPE])
        bits |= escape_bit;
    if (keyboard[SDL_SCANCODE_A] || keyboard[SDL_SCANCODE_LEFT])
        bits |= left_bit;
    if (keyboard[SDL_SCANCODE_S] || keyboard[SDL_SCANCODE_RIGHT])
        bits |= right_bit;
    if (keyboard[SDL_SCANCODE_LCTRL] || keyboard[SDL_SCANCODE_RCTRL] || keyboard[SDL_SCANCODE_UP])
        bits |= thrust_bit;
    if (keyboard[SDL_SCANCODE_RETURN])
        bits |= fire_bit;
    if (keyboard[SDL_SCANCODE_SPACE])
        bits |= pickup_bit;
    if (state.quit_requested)
        bits |= escape_bit;

    state.keybits = bits;
}

static void
pump_events(void)
{
    SDL_Event ev;

    SDL_PumpEvents();
    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_KEYDOWN)
        {
            state.last_key = ev.key.keysym.sym;
        }
        else if (ev.type == SDL_QUIT)
        {
            state.quit_requested = 1;
        }
    }
    update_keybits();
}
