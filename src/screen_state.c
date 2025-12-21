#include <SDL.h>
#include <ctype.h>

#include "conf.h"
#include "input.h"
#include "menu.h"
#include "pref.h"
#include "screen_state.h"

void
screen_state_init(void)
{
    input_flush_events();
}

void
screen_state_run(options screen)
{
    switch (screen)
    {
    case INST:
        instructions();
        break;
    case HI:
        showhighscores();
        break;
    case ABOUT:
        about();
        break;
    case CONF:
        conf();
        break;
    default:
        break;
    }
}
