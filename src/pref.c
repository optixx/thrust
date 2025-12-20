#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "font5x5.h"
#include "graphics.h"
#include "helpers.h"
#include "hiscore.h"
#include "input.h"
#include "thrust.h"
#include "pref.h"

void pressanykey(void)
{
    wait_for_key();
}

char*
enterhighscorename(void)
{
    static char name[40];
    char str[40];

    strcpy(name, standardname());
    sprintf(str, "You managed %d points!", score);
    gcenter(64, str);
    gcenter(75, "You made it into the highscore list!");
    gcenter(86, "Enter your name:");
    printgs(130, 97, name);
    fade_in();

    if (readgs(130, 97, name, 39, 80, 0) == -1)
        strcpy(name, standardname());

    fade_out();

    return (name);
}

void showhighscores(void)
{
    char str[100];
    uint8_t tmp = chcolor;
    int i;
    int scorew, namew;
    int len;

    gcenter(50, "The current highscores are");

    scorew = namew = 0;
    for (i = 0; i < HIGHSCORES; i++)
    {
        sprintf(str, "%d", highscorelist[i].score);
        len = gstrlen(str);
        if (len > scorew)
            scorew = len;
        len = gstrlen(highscorelist[i].name);
        if (len > namew)
            namew = len;
    }

    for (i = 0; i < HIGHSCORES; i++)
    {
        sprintf(str, "%d", highscorelist[i].score);
        chcolor = SCORETEXT;
        printgs(155 + (scorew - namew) / 2 - gstrlen(str), 70 + 11 * i, str);
        chcolor = SCORENAME;
        printgs(165 + (scorew - namew) / 2, 70 + 11 * i, highscorelist[i].name);
    }

    chcolor = tmp;
    gcenter(145, "Press any key for the main menu.");

    fade_in();
    pressanykey();
    fade_out();
}

void newhighscore(void)
{
    char* name;

    name = enterhighscorename();
    inserthighscore(name, score);
    writehighscores();
    showhighscores();
}
