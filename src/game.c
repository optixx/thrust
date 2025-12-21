/* Derived from the original `thrust.c` gameplay helper block. */

#include <SDL.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
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
#include "state.h"
#include "world.h"

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

static int game_last_score_value = 0;

static int prepare_level(game_state_t* state);
static game_result
run_level_loop(int demo, game_state_t* state, uint8_t* actionbits_out);
static void handle_post_level(int demo, game_state_t* state, int actionbits);
static void handle_pause_menu(int* easyrider_ptr);
static void handle_escape_menu(uint32_t* endlevel_ptr, int* level_ptr);

int game(int demo)
{
    uint8_t actionbits = 0;
    game_result result;
    game_state_t state;

    if (demo)
        nextmove(1);

    state_init(&state);
    state_set_current(&state);
    world_init();
    input_flush_events();
    game_last_score_value = state.total_score;

    while (state_can_continue(&state))
    {
        if (prepare_level(&state) != 0)
            return (1);

        result = run_level_loop(demo, &state, &actionbits);
        if (result == GAME_RESULT_QUIT)
            break;

        handle_post_level(demo, &state, actionbits);
    }

    if (!demo)
    {
        chflag = 1;
        gamestatusframe();
        gamestatus(state.lives, state.fuel, state.total_score);

        gcenter(73, "Game Over");

        displayscreen();
        fade_in();
        SDL_Delay(2000);
        fade_out();
    }

    return (0);
}

static int
prepare_level(game_state_t* state)
{
    char textstr[40];
    world_state_t* world = world_state();

    srandom(time(NULL));
    if (state->level != state->lastlevel || !powerplant)
    {
        if (state->level == 0 && state->lastlevel > 0)
        {
            state->gravitymsg = 1;
            if (state->round == 2)
                state->visibilitymsg = 1;
        }
        if (!readbana(levels[state->level]))
        {
            printf("Illegal definition of level %d.\n", state->level + 1);
            return (-1);
        }
        state->restartx = (world_state()->lenx + restartpoints[0].x - (154 >> 3)) % world_state()->lenx;
        state->restarty = restartpoints[0].y - (82 >> 3) - 4 * (state->round & 1);
        state->loadedrestart = 0;
        initgame(state->round, 1, state->restartx, state->restarty);
    }
    else
    {
        world_set_loaded(state->loadedrestart);
        initgame(state->round, 0, state->restartx, state->restarty);
    }

    initscreen(state->round);
    putscr(world->pixx % PBILDX, world->pixy % PBILDY, 1);
    state->lastlevel = state->level;

    gamestatusframe();
        gamestatus(state->lives, state->fuel, state->total_score);
    sprintf(textstr, "Mission %d", state->level + 1 + state->round * 6);
    gcenter(70, textstr);
    if (state->gravitymsg)
    {
        gcenter(60, (state->round & 1) ? "Reversed gravity" : "Normal gravity");
        if (state->visibilitymsg)
            gcenter(52, (state->round & 2) ? "Invisible ground" : "Visible ground");
    }
    displayscreen();
    syncscreen();
    fade_in();
    SDL_Delay(1000 + 2000 * (state->gravitymsg));
    state->gravitymsg = 0;
    syncscreen();
    putscr(world->pixx % PBILDX, world->pixy % PBILDY, 1);
    drawteleport(1);
    displayscreen();

    return (0);
}

static game_result
run_level_loop(int demo, game_state_t* state, uint8_t* actionbits_out)
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
    int* easyrider = &state->easyrider;
    int* teleport = &state->teleport;
    int* restartx = &state->restartx;
    int* restarty = &state->restarty;
    int* loadedrestart = &state->loadedrestart;
    world_state_t* world = world_state();

    while (!endlevel)
    {
        input_frame_tick();
        actionbits = demo ? nextmove(0) : whatkeys();

        if (alive && (actionbits & right_bit))
        {
            decr_wrap(&world->kdir, 0, 96);
            world->dir = world->kdir / 3;
        }
        if (alive && (actionbits & left_bit))
        {
            incr_wrap(&world->kdir, 96, 0);
            world->dir = world->kdir / 3;
        }
        if (alive && (actionbits & fire_bit))
        {
            if (!world->shoot)
            {
                world->shoot = 1;
                newbullet((uint32_t)(world->x + ((160 + world->shipdx) << 3) + 74 * cos(world->dir * M_PI / 16)),
                          (uint32_t)(world->y + ((88 + world->shipdy) << 3) - 74 * sin(world->dir * M_PI / 16)),
                          (int)(world->speedx / 256.0 + 32 * cos(world->dir * M_PI / 16)),
                          (int)(world->speedy / 256.0 + 32 * sin(world->dir * M_PI / 16)), world->kdir / 6, 1);
            }
            else if (world->repetetive || *easyrider)
            {
                world->shoot = 0;
            }
        }
        else
        {
            world->shoot = 0;
        }
        world->refueling = 0;
        if (alive && (actionbits & pickup_bit))
        {
            if (state_has_fuel(state))
            {
                if (state_shield(state) == 0 && (state->round & 2))
                {
                    bin_colors[65 * 3 + 0] = GAMMA(world_state()->colorr);
                    bin_colors[65 * 3 + 1] = GAMMA(world_state()->colorg);
                    bin_colors[65 * 3 + 2] = GAMMA(world_state()->colorb);
                    fadepalette(0, 255, bin_colors, 64, 0);
                }
                int shield_overflow = state_pickup_shield(state);
                if (shield_overflow)
                {
#if !(defined(DEBUG) || defined(DEBUG2))
                    if (!state->easyrider)
                        state_adjust_fuel(state, -1);
#endif
                }
            }
            else
            {
                state_reset_shield(state);
            }
            l = closestfuel((world->pixx + world->shipdx + 160) % world_state()->lenx3,
                            (world->pixy + world->shipdy + 88) % world_state()->leny3);
            if (l >= 0)
            {
                if (resonablefuel((world->pixx + world->shipdx + 160) % world_state()->lenx3,
                                  (world->pixy + world->shipdy + 88) % world_state()->leny3, l))
                {
#ifndef DEBUG
                    if (!state->easyrider)
                        state_gain_fuel(state, 6);
#endif
                    world->refueling = 1;
                    things[l].alive--;
                    if (things[l].alive == 1)
                        things[l].score = 300;
                }
            }
            if (!world_is_loaded())
            {
                if (inloadcontact((world->pixx + world->shipdx + 160) % world_state()->lenx3,
                                  (world->pixy + world->shipdy + 88) % world_state()->leny3))
                {
                    world_set_load_contact(1);
                    *(level_buffer() + world_state()->lenx * world->loadby + world->loadbx) = 32;
                    drawload(0);
                }
            }
        }
        else
        {
            if (state_shield(state) != 0 && (state->round & 2))
            {
                bin_colors[65 * 3 + 0] = 0;
                bin_colors[65 * 3 + 1] = 0;
                bin_colors[65 * 3 + 2] = 0;
                fadepalette(0, 255, bin_colors, 64, 0);
            }
            state_reset_shield(state);
            if (alive && world_load_contact())
            {
                *(level_buffer() + world_state()->lenx * world->loadby + world->loadbx) = 109;
                drawload(1);
                world_set_load_contact(0);
            }
        }
        if (alive && (actionbits & thrust_bit))
        {
            if (state_has_fuel(state))
            {
                if (!sound_thrust_is_on(state))
                {
                    sound_set_thrust(state, 1);
                    Snd_effect(SND_THRUST, CHAN_1);
                }
#if !(defined(DEBUG) || defined(DEBUG2))
                if (!state->easyrider)
                    state_adjust_fuel(state, -1);
#endif
                world->oldabs = world->speedx * (long)world->speedx + world->speedy * (long)world->speedy;

                if (world_is_loaded())
                { /* Ship and blob */
                    acircum = sin(world->dir * M_PI / 16 - world->alpha);
                    world->deltaalpha += SPEED / 2 * acircum * M_PI / 262144;
                    ax = SPEED / 2 * cos(world->dir * M_PI / 16) / (1 + REL_MASS);
                    ay = SPEED / 2 * sin(world->dir * M_PI / 16) / (1 + REL_MASS);
                }
                else
                { /* Ship, no blob */
                    ax = SPEED / 2 * cos(world->dir * M_PI / 16);
                    ay = SPEED / 2 * sin(world->dir * M_PI / 16);
                }

                world->speedx += (int)ax;
                world->speedy += (int)ay;
                world->absspeed = world->speedx * (long)world->speedx + world->speedy * (long)world->speedy;
                if (world->absspeed > 1000000000L && world->absspeed > world->oldabs)
                {
                    world->speedx -= (int)ax;
                    world->speedy -= (int)ay;
                }
            }
	            else
	            {
	                if (sound_thrust_is_on(state))
	                {
	                    Snd_effect(SND_ZERO, CHAN_1);
	                    sound_set_thrust(state, 0);
	                }
	            }
	        }
	        else
	        {
	            if (sound_thrust_is_on(state))
	            {
	                Snd_effect(SND_ZERO, CHAN_1);
	                sound_set_thrust(state, 0);
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
            handle_escape_menu(&endlevel, &state->level);

        if (world_is_loaded())
        {
            if (world->loadpointshift)
            {
                world->speedx += world->shipdx * 12;
                world->speedy += world->shipdy * 12;
            }
            world->alpha += world->deltaalpha;
            if (world->alpha > 2 * M_PI)
                world->alpha -= 2 * M_PI;
            if (world->alpha < 0)
                world->alpha += 2 * M_PI;
            world->loadpointshift = 0;
            if (++world->loadpoint > 126)
                world->loadpoint = 126;
            else
                world->loadpointshift = 1;
            world->shipdx = (int)(cos(world->alpha) * world->loadpoint / 5.90625);
            world->shipdy = (int)(-sin(world->alpha) * world->loadpoint / 5.90625);
            if (world->loadpointshift)
            {
                world->speedx -= world->shipdx * 12;
                world->speedy -= world->shipdy * 12;
            }
            world->deltaalpha -= world->deltaalpha / 1024;
        }
        else
        {
            world->shipdx = world->shipdy = 0;
        }
        /* Gravity and Aerodynamics */
        if (world->speedx > 0)
            world->speedx = world->speedx - (world->speedx >> 9) - 1;
        else if (world->speedx < 0)
            world->speedx = world->speedx - (world->speedx >> 9) + 1;
        if (alive)
        {
            if (state->gravity >= 0)
                world->speedy -= (SPEED * state->gravity + 1) >> 8;
            else
                world->speedy -= (SPEED * state->gravity >> 8) + 1;
            if (world->speedy > 0)
                world->speedy--;
            else if (world->speedy < 0)
                world->speedy++;
            /* Move the Ship */
            world->speedx = (world->speedx < 16384) ? world->speedx : 16384;
            world->speedx = (world->speedx > -16384) ? world->speedx : -16384;
            world->speedy = (world->speedy < 16384) ? world->speedy : 16384;
            world->speedy = (world->speedy > -16384) ? world->speedy : -16384;
            if (world->speedx >= 0)
                world->vx = (world->speedx + 1) >> 8;
            else
                world->vx = (world->speedx >> 8) + 1;
            if (world->speedy >= 0)
                world->vy = (world->speedy + 1) >> 8;
            else
                world->vy = (world->speedy >> 8) + 1;
            world->x = (world->x + world->vx + (world_state()->lenx << 6)) % (world_state()->lenx << 6);
            world->y = (world->y - world->vy + (world_state()->leny << 6)) % (world_state()->leny << 6);
        }

        /* Bunkerfire */
        if (!ppblip)
            bunkerfirebullets();
        movebullets();
        movefragments();
        drawfuel(state_fuel(state));

        /* Move the Power Plant blip */
        ppcount = (ppcount + 1) & 15;
        if (!ppcount && powerplant && ppblip)
            ppblip--;

        if (!powerplant)
        {
            world->countdown--;
            if (world->countdown < 0)
            {
#ifndef DEBUG
                if (alive && !*easyrider)
                {
                    dying = 1;
#ifdef DEBUG2
                    printf("Dying: Power Plant world->countdown.\n");
#endif
                }
#endif
            }
            else
            {
                chflag = 1;
                sprintf(textstr, "%d  ", (world->countdown + 99) / 100);
                printgs(105, 12, textstr);
                printgs(205, 12, textstr);
                chflag = 0;
            }
        }

        /* Precalculate some values */
        world_sync_blocks();

        if (world->pblocky > (int)world_state()->leny - 3)
        {
            endlevel = 1;
#ifdef DEBUG2
            printf("Endlevel: Finished level.\n");
#endif
            *teleport = 1;
            world->y = 0;
            world->pixy = 0;
            world->pblocky = 0;
        }

        /* Check if at a restart barrier. If so, update the restart point. */
        restartxy = atbarrier((world->pblockx + ((154 + world->shipdx) >> 3)) % world_state()->lenx, world->pblocky + ((82 + world->shipdy) >> 3));
        if (restartxy)
        {
            *restartx = (world_state()->lenx + restartxy->x - (154 >> 3)) % world_state()->lenx;
            *restarty = restartxy->y - (82 >> 3);
        }
        *loadedrestart = world_is_loaded();

        /* Scroll the screen */
        updateborder(world->pblockx, world->pblocky, world->bblockx, world->bblocky, world->vx, world->vy);

        drawpowerplantblip();
        drawbullets();
        if (alive)
            world->crash = drawshuttle();
        drawfragments();
        if (alive && world->refueling)
            drawfuellines();
        /* Check if end of life. */
#ifndef DEBUG
        if (!*easyrider)
            if (alive && world->crash)
            {
                state->lives--;
                dying = 1;
#ifdef DEBUG2
                printf("Dying: Crashing.\n");
#endif
            }
#endif
        /* Wait for the screen retrace and then dump the graphics to it. */
        /* Screendump */

        syncscreen();
        putscr(world->bildx, world->bildy, 0);
        displayscreen();

        /* Remove moveable objects from screen in reverse order. */
        if (alive && world->refueling)
            undrawfuellines();
        undrawfragments();
        if (alive)
            undrawshuttle();
        undrawbullets();

        /* Remove objects */
        if (!*easyrider)
            state_add_local_score(state, killdyingthings());
        else
            killdyingthings();
        if (dying)
            {
                alive = 0;
                dying = 0;
                Snd_effect(SND_ZERO, CHAN_1);
                sound_set_thrust(state, 0);
                Snd_effect(SND_BOOM2, CHAN_2);
                explodeship();
            }
        if (!alive && !livefragments())
        {
            Snd_effect(SND_ZERO, CHAN_1);
            sound_set_thrust(state, 0);
            endlevel = 1;
#ifdef DEBUG2
            printf("Endlevel: Shit crashed.\n");
#endif
        }
        animatesliders();
        if (state_update_total_score(state))
        {
            game_last_score_value = state->total_score;
            chflag = 1;
            gamestatus(state->lives, state->fuel, state->total_score);
            chflag = 0;
        }
    }

    *actionbits_out = actionbits;
    return (actionbits & escape_bit) ? GAME_RESULT_QUIT : GAME_RESULT_CONTINUE;
}

static void
handle_post_level(int demo, game_state_t* state, int actionbits)
{
    char textstr[40];

    game_last_score_value = state->total_score;

    if (state->teleport)
    {
        Snd_effect(SND_ZERO, CHAN_1);
        bin_colors[65 * 3 + 0] = GAMMA(world_state()->colorr);
        bin_colors[65 * 3 + 1] = GAMMA(world_state()->colorg);
        bin_colors[65 * 3 + 2] = GAMMA(world_state()->colorb);
        fadepalette(0, 255, bin_colors, 64, 1);
        drawteleport(0);
    }

    if (!(actionbits & (quit_bit | escape_bit)))
        SDL_Delay(1000);
    fade_out();

    if (!demo && !(actionbits & (quit_bit | escape_bit)))
    {
        if (state->teleport || !powerplant)
        {
            chflag = 1;
            gamestatusframe();
            gamestatus(state->lives, state->fuel, state->total_score);

            if (!powerplant)
            {
                sprintf(textstr, "Planet destroyed");
                gcenter(61, textstr);
            }

            if (state->teleport && world_is_loaded())
                sprintf(textstr, "Mission %d complete", state->level + 1 + state->round * 6);
            else if (powerplant)
                sprintf(textstr, "Mission incomplete");
            else
                sprintf(textstr, "Misson  %d  failed", state->level + 1 + state->round * 6);
            gcenter(73 - 6 * (state->teleport && world_is_loaded() && powerplant), textstr);

            if ((state->teleport && world_is_loaded()) || !powerplant)
            {
                if (state->teleport && world_is_loaded())
                    sprintf(textstr, "Bonus %d", 4000 + 400 * (state->level) - 2000 * powerplant);
                else
                    sprintf(textstr, "No bonus");
                gcenter(85 - 6 * (!!powerplant), textstr);
            }

            displayscreen();
            fade_in();
            SDL_Delay(2000);
            if (!state->easyrider && state->teleport && world_is_loaded())
                state_add_local_score(state, 4000 + 400 * (state->level) - 2000 * powerplant);
            if ((state->teleport && world_is_loaded()) || !powerplant)
            {
                if (++(state->level) == LEVELS)
                {
                    state->level = 0;
                    state->round = (state->round + 1) % 4;
                }
            }
            if (state_update_total_score(state))
            {
                game_last_score_value = state->total_score;
                gamestatus(state->lives, state->fuel, state->total_score);
            }
            chflag = 0;
            displayscreen();
            SDL_Delay(2000);
            fade_out();
        }
    }
    state->teleport = 0;

    if (demo)
        state->level = LEVELS;
}

static void
handle_pause_menu(int* easyrider_ptr)
{
    int ch;
    options end = NOTHING;
    int easyrider = *easyrider_ptr;

    sound_set_thrust(state_current(), 0);
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

    sound_set_thrust(state_current(), 0);
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

int game_last_score(void)
{
    return game_last_score_value;
}
