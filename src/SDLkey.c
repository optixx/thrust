// SDL2 input for thrust
// output handled in SDL.c

#include <string.h>

#include <SDL.h>
#include "thrust.h"
#include "keyboard.h"

// left, right, thrust, fire, pick (, quit, pause, continue)
int scancode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static struct {
  byte keybits;
  SDL_Keycode last_key;
  int quit_requested;
} input_state;

static void pump_events(void);

// key driver name
char *
keyname(void)
{
  static char name[] = "SDL2";
  return name;
}

int
keyinit(void)
{
  flushkeyboard();
  return 0;
}

int
keyclose(void)
{
  return 0;
}

char *
keystring(int key)
{
  const char *name = SDL_GetScancodeName((SDL_Scancode)key);
  return (char *)(name && *name ? name : "unknown");
}

int
keycode(char *keyname)
{
  SDL_Scancode sc = SDL_GetScancodeFromName(keyname);
  if(sc == SDL_SCANCODE_UNKNOWN)
    return 0;
  return (int)sc;
}

int
getkey(void)
{
  int key = 0;

  pump_events();
  if(input_state.last_key) {
    key = (int)input_state.last_key;
    input_state.last_key = 0;
  } else if(input_state.quit_requested) {
    key = SDLK_ESCAPE;
  }

  return key;
}

static void
update_keybits(void)
{
  const Uint8 *state;
  byte keybits = 0;

  state = SDL_GetKeyboardState(NULL);

  if(state[SDL_SCANCODE_P])
    keybits |= pause_bit;
  if(state[SDL_SCANCODE_Q] || state[SDL_SCANCODE_ESCAPE])
    keybits |= escape_bit;

  if(state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
    keybits |= left_bit;
  if(state[SDL_SCANCODE_S] || state[SDL_SCANCODE_RIGHT])
    keybits |= right_bit;
  if(state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL] || state[SDL_SCANCODE_UP])
    keybits |= thrust_bit;
  if(state[SDL_SCANCODE_RETURN])
    keybits |= fire_bit;
  if(state[SDL_SCANCODE_SPACE])
    keybits |= pickup_bit;

  if(input_state.quit_requested)
    keybits |= escape_bit;

  input_state.keybits = keybits;
}

static void
pump_events(void)
{
  SDL_Event ev;

  SDL_PumpEvents();
  while(SDL_PollEvent(&ev)) {
    if(ev.type == SDL_KEYDOWN) {
      input_state.last_key = ev.key.keysym.sym;
    } else if(ev.type == SDL_QUIT) {
      input_state.quit_requested = 1;
    }
  }
  update_keybits();
}

byte
getkeys(void)
{
  pump_events();
  return input_state.keybits;
}

int
getonemultiplekey(void)
{
  return wait_for_key();
}

void
flushkeyboard(void)
{
  SDL_PumpEvents();
  SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
  input_state.keybits = 0;
  input_state.last_key = 0;
  input_state.quit_requested = 0;
}

int
keywaiting(void)
{
  pump_events();
  return (input_state.last_key != 0) || input_state.quit_requested;
}

int
wait_for_key(void)
{
  SDL_Event ev;

  for(;;) {
    int key = getkey();
    if(key)
      return key;

    if(SDL_WaitEventTimeout(&ev, 50)) {
      if(ev.type == SDL_KEYDOWN) {
        input_state.last_key = ev.key.keysym.sym;
        update_keybits();
        key = (int)input_state.last_key;
        input_state.last_key = 0;
        return key;
      } else if(ev.type == SDL_QUIT) {
        input_state.quit_requested = 1;
        return SDLK_ESCAPE;
      } else {
        update_keybits();
      }
    } else {
      pump_events();
    }
  }
}
