/* Derived from the original `thrust.c` gameplay helper block. */

#include <SDL.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "conf.h"
#include "font5x5.h"
#include "graphics.h"
#include "helpers.h"
#include "init.h"
#include "input.h"
#include "hud.h"
#include "level.h"
#include "soundIt.h"
#include "things.h"
#include "thrust.h"
#include "game.h"

#define checkfork(b, a)                                                                            \
    case b - 1:                                                                                    \
        if (easyrider == a || easyrider == a + 1)                                                  \
            easyrider = a + 1;                                                                     \
        else if (easyrider != 9)                                                                   \
            easyrider = -1;                                                                        \
        break

typedef enum
{
    GAME_RESULT_CONTINUE,
    GAME_RESULT_QUIT
} game_result;

static char** const levels[LEVELS] = {level1, level2, level3, level4, level5, level6};

static int prepare_level(int level, int round, int* lastlevel, int* gravitymsg, int* visibilitymsg,
                         int* restartx, int* restarty, int* loadedrestart, uint32_t lives,
                         uint32_t fuel, int score);
static game_result
run_level_loop(int demo, int round, int* level, uint32_t* lives, int* localscore, uint32_t* fuel,
               int* teleport, int* easyrider, int* restartx, int* restarty, int* loadedrestart,
               uint8_t* actionbits_out);
static void handle_post_level(int demo, int* level, int round, uint32_t* lives, int* localscore,
                              uint32_t fuel, int actionbits, int* teleport, int* easyrider);
static void handle_pause_menu(int* easyrider_ptr);
static void handle_escape_menu(uint32_t* endlevel_ptr, int* level_ptr);

int game(int demo)
{
    uint8_t actionbits = 0;
    uint32_t lives;
    uint32_t fuel;
    game_result result;
    int localscore;
    int level;
    int round;
    int lastlevel;
    int easyrider = 0;
    int restartx = 0;
    int restarty = 0;
    int loadedrestart = 0;
    int gravitymsg = 0;
    int visibilitymsg = 0;
    int teleport = 0;

    if (demo)
        nextmove(1);

    lives = 3;
    localscore = 0;
    score = 0;
    round = 0;
    level = 0;
    lastlevel = -1;
    shield = 0;
    fuel = 1000;

    while (level < LEVELS && lives > 0 && fuel)
    {
        if (prepare_level(level, round, &lastlevel, &gravitymsg, &visibilitymsg, &restartx,
                          &restarty, &loadedrestart, lives, fuel, score) != 0)
        {
            return (1);
        }

        result = run_level_loop(demo, round, &level, &lives, &localscore, &fuel, &teleport, &easyrider,
                                &restartx, &restarty, &loadedrestart, &actionbits);
        if (result == GAME_RESULT_QUIT)
            break;

        handle_post_level(demo, &level, round, &lives, &localscore, fuel, actionbits, &teleport,
                          &easyrider);
    }

    if (!demo)
    {
        chflag = 1;
        gamestatusframe();
        gamestatus(lives, fuel, score);

        gcenter(73, "Game Over");

        displayscreen();
        fade_in();
        SDL_Delay(2000);
        fade_out();
    }

    return (0);
}

static int
prepare_level(int level, int round, int* lastlevel, int* gravitymsg, int* visibilitymsg, int* restartx,
              int* restarty, int* loadedrestart, uint32_t lives, uint32_t fuel, int score)
{
    char textstr[40];

    srandom(time(NULL));
    if (level != *lastlevel || !powerplant)
    {
        if (level == 0 && *lastlevel > 0)
        {
            *gravitymsg = 1;
            if (round == 2)
                *visibilitymsg = 1;
        }
        if (!readbana(levels[level]))
        {
            printf("Illegal definition of level %d.\n", level + 1);
            return (-1);
        }
        *restartx = (lenx + restartpoints[0].x - (154 >> 3)) % lenx;
        *restarty = restartpoints[0].y - (82 >> 3) - 4 * (round & 1);
        *loadedrestart = 0;
        initgame(round, 1, *restartx, *restarty);
    }
    else
    {
        loaded = *loadedrestart;
        initgame(round, 0, *restartx, *restarty);
    }

    initscreen(round);
    putscr(pixx % PBILDX, pixy % PBILDY, 1);
    *lastlevel = level;

    gamestatusframe();
    gamestatus(lives, fuel, score);
    sprintf(textstr, "Mission %d", level + 1 + round * 6);
    gcenter(70, textstr);
    if (*gravitymsg)
    {
        gcenter(60, (round & 1) ? "Reversed gravity" : "Normal gravity");
        if (*visibilitymsg)
            gcenter(52, (round & 2) ? "Invisible ground" : "Visible ground");
    }
    displayscreen();
    syncscreen();
    fade_in();
    SDL_Delay(1000 + 2000 * (*gravitymsg));
    *gravitymsg = 0;
    syncscreen();
    putscr(pixx % PBILDX, pixy % PBILDY, 1);
    drawteleport(1);
    displayscreen();

    return (0);
}

static game_result
run_level_loop(int demo, int round, int* level, uint32_t* lives, int* localscore, uint32_t* fuel,
               int* teleport, int* easyrider, int* restartx, int* restarty, int* loadedrestart,
               uint8_t* actionbits_out)
{
    uint8_t actionbits = 0;
    uint32_t endlevel = 0;
    uint32_t dying = 0;
    uint32_t alive = 1;
    int l = 0;
    double acircum;
    double ax;
    double ay;
    restartpoint* restartxy;
    char textstr[40];

    while (!endlevel)
    {
        actionbits = demo ? nextmove(0) : whatkeys();

        if (alive && (actionbits & right_bit))
        {
            decr_wrap(&kdir, 0, 96);
            dir = kdir / 3;
        }
        if (alive && (actionbits & left_bit))
        {
            incr_wrap(&kdir, 96, 0);
            dir = kdir / 3;
        }
        if (alive && (actionbits & fire_bit))
        {
            if (!shoot)
            {
                shoot = 1;
                newbullet((uint32_t)(x + ((160 + shipdx) << 3) + 74 * cos(dir * M_PI / 16)),
                          (uint32_t)(y + ((88 + shipdy) << 3) - 74 * sin(dir * M_PI / 16)),
                          (int)(speedx / 256.0 + 32 * cos(dir * M_PI / 16)),
                          (int)(speedy / 256.0 + 32 * sin(dir * M_PI / 16)), kdir / 6, 1);
            }
            else if (repetetive || *easyrider)
            {
                shoot = 0;
            }
        }
        else
        {
            shoot = 0;
        }
        refueling = 0;
        if (alive && (actionbits & pickup_bit))
        {
            if (*fuel > 0)
            {
                if (shield == 0 && round & 2)
                {
                    bin_colors[65 * 3 + 0] = GAMMA(colorr);
                    bin_colors[65 * 3 + 1] = GAMMA(colorg);
                    bin_colors[65 * 3 + 2] = GAMMA(colorb);
                    fadepalette(0, 255, bin_colors, 64, 0);
                }
                if (shield++ == 3)
                {
#if !(defined(DEBUG) || defined(DEBUG2))
                    if (!*easyrider)
                        (*fuel)--;
#endif
                    shield = 1;
                }
            }
            else
            {
                shield = 0;
            }
            l = closestfuel((pixx + shipdx + 160) % lenx3, (pixy + shipdy + 88) % leny3);
            if (l >= 0)
            {
                if (resonablefuel((pixx + shipdx + 160) % lenx3, (pixy + shipdy + 88) % leny3, l))
                {
#ifndef DEBUG
                    if (!*easyrider)
                        (*fuel) += 6;
#endif
                    refueling = 1;
                    things[l].alive--;
                    if (things[l].alive == 1)
                        things[l].score = 300;
                }
            }
            if (!loaded)
            {
                if (inloadcontact((pixx + shipdx + 160) % lenx3, (pixy + shipdy + 88) % leny3))
                {
                    loadcontact = 1;
                    *(bana + lenx * loadby + loadbx) = 32;
                    drawload(0);
                }
            }
        }
        else
        {
            if (shield != 0 && round & 2)
            {
                bin_colors[65 * 3 + 0] = 0;
                bin_colors[65 * 3 + 1] = 0;
                bin_colors[65 * 3 + 2] = 0;
                fadepalette(0, 255, bin_colors, 64, 0);
            }
            shield = 0;
            if (alive && loadcontact)
            {
                *(bana + lenx * loadby + loadbx) = 109;
                drawload(1);
                loadcontact = 0;
            }
        }
        if (alive && (actionbits & thrust_bit))
        {
            if (*fuel > 0)
            {
                if (Thrust_Is_On == 0)
                {
                    Thrust_Is_On = 1;
                    Snd_effect(SND_THRUST, CHAN_1);
                }
#if !(defined(DEBUG) || defined(DEBUG2))
                if (!*easyrider)
                    (*fuel)--;
#endif
                oldabs = speedx * (long)speedx + speedy * (long)speedy;

                if (loaded)
                { /* Ship and blob */
                    acircum = sin(dir * M_PI / 16 - alpha);
                    deltaalpha += SPEED / 2 * acircum * M_PI / 262144;
                    ax = SPEED / 2 * cos(dir * M_PI / 16) / (1 + REL_MASS);
                    ay = SPEED / 2 * sin(dir * M_PI / 16) / (1 + REL_MASS);
                }
                else
                { /* Ship, no blob */
                    ax = SPEED / 2 * cos(dir * M_PI / 16);
                    ay = SPEED / 2 * sin(dir * M_PI / 16);
                }

                speedx += (int)ax;
                speedy += (int)ay;
                absspeed = speedx * (long)speedx + speedy * (long)speedy;
                if (absspeed > 1000000000L && absspeed > oldabs)
                {
                    speedx -= (int)ax;
                    speedy -= (int)ay;
                }
            }
            else
            {
                if (Thrust_Is_On == 1)
                {
                    Snd_effect(SND_ZERO, CHAN_1);
                    Thrust_Is_On = 0;
                }
            }
        }
        else
        {
            if (Thrust_Is_On == 1)
            {
                Snd_effect(SND_ZERO, CHAN_1);
                Thrust_Is_On = 0;
            }
        }
        if (actionbits & quit_bit)
        {
#ifdef DEBUG2
            printf("Endlevel: User pressed quit-key\n");
#endif
            endlevel = 1;
        }

        if (actionbits & pause_bit)
            handle_pause_menu(easyrider);
        if (actionbits & escape_bit)
            handle_escape_menu(&endlevel, level);

        if (loaded)
        {
            if (loadpointshift)
            {
                speedx += shipdx * 12;
                speedy += shipdy * 12;
            }
            alpha += deltaalpha;
            if (alpha > 2 * M_PI)
                alpha -= 2 * M_PI;
            if (alpha < 0)
                alpha += 2 * M_PI;
            loadpointshift = 0;
            if (++loadpoint > 126)
                loadpoint = 126;
            else
                loadpointshift = 1;
            shipdx = (int)(cos(alpha) * loadpoint / 5.90625);
            shipdy = (int)(-sin(alpha) * loadpoint / 5.90625);
            if (loadpointshift)
            {
                speedx -= shipdx * 12;
                speedy -= shipdy * 12;
            }
            deltaalpha -= deltaalpha / 1024;
        }
        else
        {
            shipdx = shipdy = 0;
        }
        /* Gravity and Aerodynamics */
        if (speedx > 0)
            speedx = speedx - (speedx >> 9) - 1;
        else if (speedx < 0)
            speedx = speedx - (speedx >> 9) + 1;
        if (alive)
        {
            if (gravity >= 0)
                speedy -= (SPEED * gravity + 1) >> 8;
            else
                speedy -= (SPEED * gravity >> 8) + 1;
            if (speedy > 0)
                speedy--;
            else if (speedy < 0)
                speedy++;
            /* Move the Ship */
            speedx = (speedx < 16384) ? speedx : 16384;
            speedx = (speedx > -16384) ? speedx : -16384;
            speedy = (speedy < 16384) ? speedy : 16384;
            speedy = (speedy > -16384) ? speedy : -16384;
            if (speedx >= 0)
                vx = (speedx + 1) >> 8;
            else
                vx = (speedx >> 8) + 1;
            if (speedy >= 0)
                vy = (speedy + 1) >> 8;
            else
                vy = (speedy >> 8) + 1;
            x = (x + vx + (lenx << 6)) % (lenx << 6);
            y = (y - vy + (leny << 6)) % (leny << 6);
        }

        /* Bunkerfire */
        if (!ppblip)
            bunkerfirebullets();
        movebullets();
        movefragments();
        drawfuel(*fuel);

        /* Move the Power Plant blip */
        ppcount = (ppcount + 1) & 15;
        if (!ppcount && powerplant && ppblip)
            ppblip--;

        if (!powerplant)
        {
            countdown--;
            if (countdown < 0)
            {
#ifndef DEBUG
                if (alive && !*easyrider)
                {
                    dying = 1;
#ifdef DEBUG2
                    printf("Dying: Power Plant countdown.\n");
#endif
                }
#endif
            }
            else
            {
                chflag = 1;
                sprintf(textstr, "%d  ", (countdown + 99) / 100);
                printgs(105, 12, textstr);
                printgs(205, 12, textstr);
                chflag = 0;
            }
        }

        /* Precalculate some values */
        pixx = x >> 3;
        pixy = y >> 3;
        bildx = (pixx + PBILDX - 4) % PBILDX + 4;
        bildy = pixy % PBILDY;
        pblockx = pixx >> 3;
        pblocky = pixy >> 3;
        bblockx = bildx >> 3;
        bblocky = bildy >> 3;

        if (pblocky > (int)leny - 3)
        {
            endlevel = 1;
#ifdef DEBUG2
            printf("Endlevel: Finished level.\n");
#endif
            *teleport = 1;
            y = 0;
            pixy = 0;
            pblocky = 0;
        }

        /* Check if at a restart barrier. If so, update the restart point. */
        restartxy = atbarrier((pblockx + ((154 + shipdx) >> 3)) % lenx, pblocky + ((82 + shipdy) >> 3));
        if (restartxy)
        {
            *restartx = (lenx + restartxy->x - (154 >> 3)) % lenx;
            *restarty = restartxy->y - (82 >> 3);
            *loadedrestart = loaded;
        }

        /* Scroll the screen */
        updateborder(pblockx, pblocky, bblockx, bblocky, vx, vy);

        drawpowerplantblip();
        drawbullets();
        if (alive)
            crash = drawshuttle();
        drawfragments();
        if (alive && refueling)
            drawfuellines();
        /* Check if end of life. */
#ifndef DEBUG
        if (!*easyrider)
            if (alive && crash)
            {
                (*lives)--;
                dying = 1;
#ifdef DEBUG2
                printf("Dying: Crashing.\n");
#endif
            }
#endif
        /* Wait for the screen retrace and then dump the graphics to it. */
        /* Screendump */

        syncscreen();
        putscr(bildx, bildy, 0);
        displayscreen();

        /* Remove moveable objects from screen in reverse order. */
        if (alive && refueling)
            undrawfuellines();
        undrawfragments();
        if (alive)
            undrawshuttle();
        undrawbullets();

        /* Remove objects */
        if (!*easyrider)
            *localscore += killdyingthings();
        else
            killdyingthings();
        if (dying)
        {
            alive = 0;
            dying = 0;
            Snd_effect(SND_ZERO, CHAN_1);
            Thrust_Is_On = 0;
            Snd_effect(SND_BOOM2, CHAN_2);
            explodeship();
        }
        if (!alive && !livefragments())
        {
            Snd_effect(SND_ZERO, CHAN_1);
            Thrust_Is_On = 0;
            endlevel = 1;
#ifdef DEBUG2
            printf("Endlevel: Shit crashed.\n");
#endif
        }
        animatesliders();
        if (*localscore > score)
        {
            chflag = 1;
            if (*localscore / 10000 > score / 10000)
                (*lives)++;
            score = *localscore;
            gamestatus(*lives, *fuel, score);
            chflag = 0;
        }
    }

    *actionbits_out = actionbits;
    return (actionbits & escape_bit) ? GAME_RESULT_QUIT : GAME_RESULT_CONTINUE;
}

static void
handle_post_level(int demo, int* level, int round, uint32_t* lives, int* localscore, uint32_t fuel,
                  int actionbits, int* teleport, int* easyrider)
{
    char textstr[40];

    if (*teleport)
    {
        Snd_effect(SND_ZERO, CHAN_1);
        bin_colors[65 * 3 + 0] = GAMMA(colorr);
        bin_colors[65 * 3 + 1] = GAMMA(colorg);
        bin_colors[65 * 3 + 2] = GAMMA(colorb);
        fadepalette(0, 255, bin_colors, 64, 1);
        drawteleport(0);
    }

    if (!(actionbits & (quit_bit | escape_bit)))
        SDL_Delay(1000);
    fade_out();

    if (!demo && !(actionbits & (quit_bit | escape_bit)))
    {
        if (*teleport || !powerplant)
        {
            chflag = 1;
            gamestatusframe();
            gamestatus(*lives, fuel, score);

            if (!powerplant)
            {
                sprintf(textstr, "Planet destroyed");
                gcenter(61, textstr);
            }

            if (*teleport && loaded)
                sprintf(textstr, "Mission %d complete", *level + 1 + round * 6);
            else if (powerplant)
                sprintf(textstr, "Mission incomplete");
            else
                sprintf(textstr, "Misson  %d  failed", *level + 1 + round * 6);
            gcenter(73 - 6 * (*teleport && loaded && powerplant), textstr);

            if ((*teleport && loaded) || !powerplant)
            {
                if (*teleport && loaded)
                    sprintf(textstr, "Bonus %d", 4000 + 400 * (*level) - 2000 * powerplant);
                else
                    sprintf(textstr, "No bonus");
                gcenter(85 - 6 * (!!powerplant), textstr);
            }

            displayscreen();
            fade_in();
            SDL_Delay(2000);
            if (!*easyrider && *teleport && loaded)
                *localscore += 4000 + 400 * (*level) - 2000 * powerplant;
            if ((*teleport && loaded) || !powerplant)
            {
                if (++(*level) == LEVELS)
                {
                    *level = 0;
                    round = (round + 1) % 4;
                }
            }
            if (*localscore / 10000 > score / 10000)
                (*lives)++;
            score = *localscore;
            gamestatus(*lives, fuel, score);
            chflag = 0;
            displayscreen();
            SDL_Delay(2000);
            fade_out();
        }
    }
    *teleport = 0;

    if (demo)
        *level = LEVELS;
}

static void
handle_pause_menu(int* easyrider_ptr)
{
    int ch;
    options end = NOTHING;
    int easyrider = *easyrider_ptr;

    Thrust_Is_On = 0;
    Snd_effect(SND_ZERO, CHAN_1);
    pause_message();
    flushkeyboard();

    while (end == NOTHING)
    {
        ch = wait_for_key();
        switch (islower(ch) ? ch : tolower(ch))
        {
        case 'p':
            if (easyrider != 9)
                easyrider = 0;
            break;
        case 't':
            checkfork('m', 0);
            checkfork('b', 1);
            checkfork('z', 2);
            checkfork('h', 3);
            checkfork('s', 4);
            checkfork('p', 5);
            checkfork('v', 6);
            checkfork('o', 7);
            checkfork('e', 8);
            break;
        case 'c':
        case 'q':
        case 27:
            end = PLAY;
            break;
        }
    }

    if (easyrider != 9)
        easyrider = 0;

    *easyrider_ptr = easyrider;
}

static void
handle_escape_menu(uint32_t* endlevel_ptr, int* level_ptr)
{
    int ch;
    options end = NOTHING;

    Thrust_Is_On = 0;
    Snd_effect(SND_ZERO, CHAN_1);
    escape_message();
    flushkeyboard();

    while (end == NOTHING)
    {
        ch = wait_for_key();
        switch (islower(ch) ? ch : tolower(ch))
        {
        case 'y':
            end = END;
            *endlevel_ptr = 1;
            *level_ptr = LEVELS;
            break;
        case 'n':
            end = PLAY;
            break;
        }
    }
}
