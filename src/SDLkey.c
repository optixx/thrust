// SDL2 input for thrust
// output handled in SDL.c

#include <string.h>

#include <SDL.h>
#include "thrust.h"
#include "keyboard.h"

// left, right, thrust, fire, pick (, quit, pause, continue)
int scancode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static int keyz[SDL_NUM_SCANCODES];

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
  SDL_Event ev;

  while(SDL_WaitEvent(&ev)) {
    if(ev.type == SDL_KEYDOWN) {
      return (int)ev.key.keysym.sym;
    } else if(ev.type == SDL_QUIT) {
      return SDLK_ESCAPE;
    }
  }
  return 0;
}

byte
getkeys(void)
{
  const Uint8 *state;
  byte keybits = 0;

  SDL_PumpEvents();
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

  return keybits;
}

void
singlekey(void)
{
}

void
multiplekeys(void)
{
}

int
getonemultiplekey(void)
{
  return getkey();
}

void
flushkeyboard(void)
{
  SDL_PumpEvents();
  SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
  memset(keyz, 0, sizeof keyz);
}

int
keywaiting(void)
{
  SDL_Event ev;

  SDL_PumpEvents();
  if(SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, SDL_KEYDOWN, SDL_KEYDOWN) > 0)
    return 1;
  return 0;
}
