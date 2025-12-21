
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef THRUST_H
#define THRUST_H

/* DEBUG  will make you undestructible.
   DEBUG2 gives you some trace output. */

/*#define DEBUG*/
/*#define DEBUG2*/

/* The speed of the game. Gravity and thrust is affected.
   Don't try too large values. 256 is what it used to
   be in the good old days. */

#define SPEED 180

/* The relative mass of the blob. I.e relative to the ship.
   Higher values makes it harder to lift.
   Try negative values at your own risk. */

#define REL_MASS 1.1

#include "world.h"
#include "helpers.h"

/* How many levels and what they are named. */

#define LEVELS (6)
extern char* level1[];
extern char* level2[];
extern char* level3[];
extern char* level4[];
extern char* level5[];
extern char* level6[];

extern uint8_t sound_boom[];
extern uint8_t sound_boom2[];
extern uint8_t sound_harp[];
extern uint8_t sound_thrust[];
extern uint8_t sound_zero[];
extern unsigned int sound_boom_len;
extern unsigned int sound_boom2_len;
extern unsigned int sound_harp_len;
extern unsigned int sound_thrust_len;
extern unsigned int sound_zero_len;

typedef enum { INST, PLAY, HI, ABOUT, END, CONF, NOTHING, DEMO } options;

extern unsigned int title_nr_colors;
extern unsigned int title_cols, title_rows;
extern uint8_t title_colors[];
extern uint8_t title_pixels[];
extern uint8_t bin_blks[];
extern uint8_t bin_ship[];
extern uint8_t bin_shld[];
extern uint8_t bin_colors[];
extern int window_zoom;
extern uint8_t bin_bullet[];
extern uint8_t bin_demomove[];

#define GAMMA(x) ((int)(255 * pow((x) / 255.0, 1 / world_state()->gamma_correction)))
#define TEXTCOLOR (32)
#define HIGHLIGHT (33)
#define FUELCOLOR (34)
#define SCORETEXT (35)
#define SCORENAME (36)
#endif /* THRUST_H */
