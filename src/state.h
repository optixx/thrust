#ifndef STATE_H
#define STATE_H

#include <stdint.h>

#include "thrust.h"

typedef struct
{
    int thrust_on;
} sound_state_t;

typedef struct
{
    int level;
    int round;
    int lastlevel;
    int restartx;
    int restarty;
    int loadedrestart;
    uint32_t lives;
    uint32_t fuel;
    int localscore;
    int total_score;
    uint8_t shield;
    int gravity;
    int gravitymsg;
    int visibilitymsg;
    int teleport;
    int easyrider;
    sound_state_t sound;
} game_state_t;

void state_init(game_state_t* state);
int state_can_continue(const game_state_t* state);
void state_set_current(game_state_t* state);
game_state_t* state_current(void);
int sound_thrust_is_on(const game_state_t* state);
void sound_set_thrust(game_state_t* state, int on);

uint32_t state_fuel(const game_state_t* state);
int state_has_fuel(const game_state_t* state);
void state_adjust_fuel(game_state_t* state, int delta);
void state_gain_fuel(game_state_t* state, uint32_t amount);

uint8_t state_shield(const game_state_t* state);
int state_pickup_shield(game_state_t* state);
void state_reset_shield(game_state_t* state);

void state_add_local_score(game_state_t* state, int amount);
int state_update_total_score(game_state_t* state);

#endif /* STATE_H */
