#include <SDL.h>
#include <ctype.h>

#include "font5x5.h"
#include "graphics.h"
#include "helpers.h"
#include "input.h"
#include "menu.h"
#include "menu_state.h"
#include "pref.h"
#include "thrust.h"

void
menu_state_init(void)
{
    input_flush_events();
}

options
menu_state_run(void)
{
    int i;
    options end = NOTHING;
    int ch;
    static char* menuchoices[NOTHING] = {"I", "C", "H", "P", "A", "Q"};
    static char* menuoptions[NOTHING] = {"Instructions", "Configuration", "Highscores",
                                         "Play Game",    "About",         "Quit"};
    int count = 0;

    clearscr();
    putarea(title_pixels, 0, 0, title_cols, title_rows, title_cols, (PUSEX - title_cols) >> 1,
            (PUSEY + 24 - title_rows) >> 1);
    for (i = 0; i < NOTHING; i++)
    {
        chcolor = TEXTCOLOR;
        printgs(20, 134 + i * 11, menuoptions[i]);
        chcolor = HIGHLIGHT;
        printgs(20, 134 + i * 11, menuchoices[i]);
    }
    printgs(60, 35, "GRATTIS CALLE!");
    chcolor = TEXTCOLOR;

    fade_in();

    while (end == NOTHING)
    {
        input_frame_tick();
        const SDL_Keycode key = input_consume_key();
        const input_actions_t* actions = input_actions();

        if (actions->escape)
        {
            end = END;
            break;
        }

        ch = (int)key;
        switch (islower(ch) ? ch : tolower(ch))
        {
        case 0:
            break;
        case 'i':
            end = INST;
            break;
        case 'p':
            end = PLAY;
            break;
        case 'h':
            end = HI;
            break;
        case 'a':
            end = ABOUT;
            break;
        case 'c':
            end = CONF;
            break;
        case 'd':
            end = DEMO;
            break;
        case 'q':
        case 27:
            end = END;
            break;
        default:
            count = 0;
            break;
        }

        SDL_Delay(20);

        if (++count == 160 && end == NOTHING)
        {
            if (!world_nodemo())
                end = DEMO;
            count = 0;
        }
    }

    fade_out();

    return (end);
}
