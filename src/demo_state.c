#include <SDL.h>

#include "demo_state.h"
#include "game.h"
#include "input.h"

void
demo_state_init(void)
{
    input_flush_events();
}

void
demo_state_run(void)
{
    input_flush_events();
    game(1);
}
