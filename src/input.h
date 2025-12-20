#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include <stdint.h>

enum {
    pause_bit = 1 << 0,
    escape_bit = 1 << 1,
    right_bit = 1 << 2,
    left_bit = 1 << 3,
    fire_bit = 1 << 4,
    pickup_bit = 1 << 5,
    thrust_bit = 1 << 6,
    quit_bit = 1 << 7
};

extern int scancode[8];

const char* input_driver_name(void);
int input_init(void);
int input_shutdown(void);
uint8_t input_get_action_bits(void);
int input_has_pending_events(void);
SDL_Keycode input_wait_key(void);
void input_flush_events(void);
const char* input_keyname(SDL_Keycode key);
SDL_Keycode input_keycode(const char* name);

/* legacy wrappers retained for compatibility */
char* keyname(void);
int keyinit(void);
int keyclose(void);
char* keystring(int key);
int keycode(char* keyname);
int getkey(void);
int keywaiting(void);
int wait_for_key(void);
uint8_t getkeys(void);
uint8_t whatkeys(void);
void flushkeyboard(void);

#endif /* INPUT_H */
