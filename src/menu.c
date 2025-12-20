#include <SDL.h>
#include <ctype.h>
#include <stdio.h>

#include "font5x5.h"
#include "graphics.h"
#include "helpers.h"
#include "input.h"
#include "pref.h"
#include "thrust.h"
#include "menu.h"

void instructions(void)
{
    int i;
    static char* keys[] = {"Esc", "P", "C"};
    static char* func[] = {"Turn left",        "Turn right",        "Thrust", "Fire",
                           "Pick up & Shield", "Quit Game (Q=Esc)", "Pause",  "Continue"};
    static char* story[] = {"The resistance is about to launch a major offensive against",
                            "the Intergalactic Empire. In preparation for this, they have",
                            "captured several battle-grade starships, but they lack the",
                            "essential power sources for these formidable craft; Klystron",
                            "Pods. You have been commissioned by resistance to steal these",
                            "pods from the Empire's storage planets. Each planet is",
                            "defended by a battery of 'Limpet' guns, powered by a nuclear",
                            "power plant. By firing shots at the power plant, the guns can",
                            "be temporarily disabled; the more shots fired at the nuclear",
                            "reactor, the longer the guns will take to recharge.",
                            "BUT BEWARE!! If you fire too many shots at the reactor, it",
                            "will become critical, giving you just ten seconds to clear",
                            "the plantet before it is destroyed. If you have not already",
                            "retrieved the pod stored at the planet, then you will have",
                            "failed the mission. If you have retrieved the pod, and you",
                            "manage to send the reactor into its critical phase, and",
                            "leave the planet safely, you will receive a hefty bonus.",
                            NULL};
    static char* score[] = {"Destroying a limpet gun:",
                            "Destroying a fuel cell:",
                            "Picking up a fuel cell:",
                            "Bonus for destroying planet:",
                            "",
                            "A spare ship is allocated for every:"};
    static char* scores[] = {"750", "150", "300", "2000 + Mission Bonus", "", "10000"};

    chcolor = HIGHLIGHT;
    gcenter(20, "THE SILLY STORY!");
    chcolor = TEXTCOLOR;
    for (i = 0; story[i]; i++)
        gcenter(35 + i * 8, story[i]);

    gcenter(180, "Press any key for the next page.");

    fade_in();
    pressanykey();
    fade_out();

    chcolor = HIGHLIGHT;
    gcenter(40, "SCORING");
    chcolor = TEXTCOLOR;
    for (i = 0; i < 6; i++)
    {
        chcolor = TEXTCOLOR;
        printgs(200 - gstrlen(score[i]), 65 + 8 * i, score[i]);
        chcolor = HIGHLIGHT;
        printgs(205, 65 + 8 * i, scores[i]);
    }

    chcolor = TEXTCOLOR;
    gcenter(130, "Press any key for the next page.");

    fade_in();
    pressanykey();
    fade_out();

    gcenter(50, "The following keys are used:");
    for (i = 0; i < 8; i++)
    {
        chcolor = HIGHLIGHT;
        if (i < 5)
            printgs(140 - gstrlen(keystring(scancode[i])), 63 + i * 8 + 2 * (i > 4),
                    keystring(scancode[i]));
        else
            printgs(140 - gstrlen(keys[i - 5]), 63 + i * 8 + 2 * (i > 4), keys[i - 5]);
        chcolor = TEXTCOLOR;
        printgs(145, 63 + i * 8 + 2 * (i > 4), func[i]);
    }
    gcenter(150, "Press any key for the main menu.");

    fade_in();
    pressanykey();
    fade_out();
}

void about(void)
{
    int i;
    static char* str[] = {"Thrust version " VERSION,
                          "",
                          "Written by",
                          "",
                          "Peter Ekberg",
                          "peda@lysator.liu.se",
                          "",
                          "Thanks to the authors",
                          "of the original",
                          "for the C64.",
                          NULL};

    for (i = 0; str[i]; i++)
    {
        if (i == 5)
            chcolor = HIGHLIGHT;
        else
            chcolor = TEXTCOLOR;
        gcenter(40 + 9 * i, str[i]);
    }
    gcenter(145, "Press any key for the main menu.");

    fade_in();
    pressanykey();
    fade_out();
}

options
menu(void)
{
    int i;
    options end = NOTHING;
    int ch;
    static char* menuchoises[NOTHING] = {"I", "C", "H", "P", "A", "Q"};
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
        printgs(20, 134 + i * 11, menuchoises[i]);
    }
    printgs(60, 35, "GRATTIS CALLE!");
    chcolor = TEXTCOLOR;

    fade_in();

    while (end == NOTHING)
    {
        ch = getkey();
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
        SDL_Delay(50);

        if (++count == 160 && end == NOTHING)
        {
            if (!nodemo)
                end = DEMO;
            count = 0;
        }
    }

    fade_out();

    return (end);
}
