#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "font5x5.h"
#include "graphics.h"
#include "helpers.h"
#include "input.h"
#include "hud.h"
#include "thrust.h"
void pause_message(void)
{
    char* str[] = {"Game paused.", "Press 'C' to continue..."};

    chflag = 1;
    gcenter(80, str[0]);
    gcenter(91, str[1]);
    chflag = 0;
    displayscreen();
}

void escape_message(void)
{
    char* str[] = {"Are you sure you want to quit (Y/N)?"};

    chflag = 1;
    gcenter(85, str[0]);
    chflag = 0;
    displayscreen();
}

uint8_t nextmove(int reset)
{
    static uint8_t* p;
    uint8_t retbits = 0;

    if (reset)
    {
        keywaiting();
        p = &bin_demomove[0];
    }
    else if (keywaiting())
    {
        retbits = quit_bit;
        flushkeyboard();
    }
    else
    {
        retbits = *(p++);
        retbits &= ~thrust_bit;
        retbits |= *(p - !!(random() % 20)) & thrust_bit;
    }

    return (retbits);
}

void gamestatusframe(void)
{
    int i, maxx;

    for (i = 3; i < 317; i++)
        putpixel(i, 3, 0x88);
    for (i = 3; i < 11; i++)
    {
        putpixel(3, i, 0x88);
        putpixel(4, i, 0x88);
        putpixel(315, i, 0x88);
        putpixel(316, i, 0x88);
    }
    for (i = 3; i < 11; i++)
    {
        putpixel(i, i + 8, 0x88);
        putpixel(1 + i, i + 8, 0x88);
        putpixel(318 - i, i + 8, 0x88);
        putpixel(319 - i, i + 8, 0x88);
    }
    for (i = 9; i < 311; i++)
        putpixel(i, 18, 0x88);
    maxx = 80 - gstrlen("FUEL") - 2;
    for (i = 6; i < maxx; i++)
    {
        putpixel(i, 6, 65);
        putpixel(i, 8, 65);
    }
    maxx = 160 - gstrlen("LIVES") / 2 - 2;
    for (i = 82; i < maxx; i++)
    {
        putpixel(i, 6, 65);
        putpixel(i, 8, 65);
    }
    maxx = 280 - gstrlen("SCORE") - 2;
    for (i = 160 + gstrlen("LIVES") / 2 + 2; i < maxx; i++)
    {
        putpixel(i, 6, 65);
        putpixel(i, 8, 65);
    }
    for (i = 282; i < 313; i++)
    {
        putpixel(i, 6, 65);
        putpixel(i, 8, 65);
    }
    chcolor = 0x89;
    printgs(80 - gstrlen("FUEL"), 5, "FUEL");
    gcenter(5, "LIVES");
    printgs(280 - gstrlen("SCORE"), 5, "SCORE");
    chcolor = TEXTCOLOR;
}

void gamestatus(int lives, int fuel, int score)
{
    static char textstr[40];

    chcolor = 0x88;
    drawfuel(fuel);
    sprintf(textstr, "%d", lives);
    gcenter(12, textstr);
    sprintf(textstr, "%d", score);
    printgs(280 - gstrlen(textstr), 12, textstr);
    chcolor = TEXTCOLOR;
}
