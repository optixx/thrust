#include <stdint.h>
#include <stddef.h>

#include "state.h"

static game_state_t* current_state = NULL;

void state_init(game_state_t* state)
{
    state->level = 0;
    state->round = 0;
    state->lastlevel = -1;
    state->restartx = 0;
    state->restarty = 0;
    state->loadedrestart = 0;
    state->lives = 3;
    state->fuel = 1000;
    state->localscore = 0;
    state->total_score = 0;
    state->shield = 0;
    state->gravity = 0;
    state->gravitymsg = 0;
    state->visibilitymsg = 0;
    state->teleport = 0;
    state->easyrider = 0;
    state->sound.thrust_on = 0;
}

int state_can_continue(const game_state_t* state)
{
    return state->level < LEVELS && state->lives > 0 && state->fuel;
}

void state_set_current(game_state_t* state)
{
    current_state = state;
}

game_state_t* state_current(void)
{
    return current_state;
}

int sound_thrust_is_on(const game_state_t* state)
{
    return state && state->sound.thrust_on;
}

void sound_set_thrust(game_state_t* state, int on)
{
    if (state)
        state->sound.thrust_on = on;
}

uint32_t state_fuel(const game_state_t* state)
{
    return state ? state->fuel : 0;
}

int state_has_fuel(const game_state_t* state)
{
    return state && state->fuel > 0;
}

void state_adjust_fuel(game_state_t* state, int delta)
{
    if (!state)
        return;
    if (delta < 0)
    {
        uint32_t amount = (uint32_t)(-delta);
        if (state->fuel <= amount)
            state->fuel = 0;
        else
            state->fuel -= amount;
    }
    else
    {
        state->fuel += (uint32_t)delta;
    }
}

void state_gain_fuel(game_state_t* state, uint32_t amount)
{
    if (!state)
        return;
    state->fuel += amount;
}

uint8_t state_shield(const game_state_t* state)
{
    return state ? state->shield : 0;
}

int state_pickup_shield(game_state_t* state)
{
    if (!state)
        return 0;
    int prev = state->shield;
    state->shield++;
    if (prev == 3)
    {
        state->shield = 1;
        return 1;
    }
    return 0;
}

void state_reset_shield(game_state_t* state)
{
    if (!state)
        return;
    state->shield = 0;
}

void state_add_local_score(game_state_t* state, int amount)
{
    if (!state)
        return;
    state->localscore += amount;
}

int state_update_total_score(game_state_t* state)
{
    if (!state || state->localscore <= state->total_score)
        return 0;
    if (state->localscore / 10000 > state->total_score / 10000)
        state->lives++;
    state->total_score = state->localscore;
    return 1;
}
