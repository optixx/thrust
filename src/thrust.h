
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


#include "thrust_t.h"

/* How many levels and what they are named. */

#define LEVELS (6)
extern char *level1[];
extern char *level2[];
extern char *level3[];
extern char *level4[];
extern char *level5[];
extern char *level6[];


#ifdef HAVE_SOUND
extern uint8_t sound_boom[];
extern uint8_t sound_boom2[];
extern uint8_t sound_harp[];
extern uint8_t sound_thrust[];
extern uint8_t sound_zero[];
extern unsigned int  sound_boom_len;
extern unsigned int  sound_boom2_len;
extern unsigned int  sound_harp_len;
extern unsigned int  sound_thrust_len;
extern unsigned int  sound_zero_len;
#endif

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

extern uint8_t *bulletmap;
extern uint8_t *blocks;
extern uint8_t *ship;
extern uint8_t *shieldship;
extern uint8_t *bana;
extern uint8_t *fuelmap;
extern uint8_t *loadmap;
extern uint8_t *shipstorage;
extern uint8_t *bulletstorage;
extern uint8_t *fragmentstorage;
extern uint8_t *fuelstorage;
extern uint8_t *loadstorage;
extern uint8_t *wirestorage;

extern uint32_t lenx; /* Banans max i x-len, används senare till den aktuella */
extern uint32_t leny; /* Banans max i y-len, storleken på banan.Sätts i readbana */
extern uint32_t lenx3,leny3;

extern double alpha,deltaalpha;
extern uint32_t loaded,loadcontact,loadpointshift;
extern int loadpoint;
extern int countdown;
extern uint32_t crash,shoot,repetetive;
extern uint32_t refueling;
extern int speedx,speedy;
extern long absspeed,oldabs;
extern int kdir,dir;
extern int shipdx,shipdy;
extern int x,y;
extern int pixx,pixy;
extern int pblockx,pblocky;
extern int vx,vy;
extern int bildx,bildy;
extern int bblockx,bblocky;
extern int loadbx,loadby;
extern int gravity;
extern int score;
extern uint8_t shield;
extern uint8_t colorr, colorg, colorb;
extern int play_sound;
extern double gamma_correction;
extern int skip_frames;

#define GAMMA(x)  ((int)(255*pow((x)/255.0, 1/gamma_correction)))
#define TEXTCOLOR (32)
#define HIGHLIGHT (33)
#define FUELCOLOR (34)
#define SCORETEXT (35)
#define SCORENAME (36)

int insideblock(int blockx, int blocky,
		int pblockx, int pblocky, int sx, int sy);
int insidepixel(int x, int y, int pixx, int pixy, int sx, int sy);
void updateborder(int pblockx, int pblocky,
		  int bblockx, int bblocky, int vx, int vy);
void fade_in(void);
void fade_out(void);
void pause_message(void);
void escape_message(void);
uint8_t whatkeys(void);
uint8_t nextmove(int reset);
void gamestatusframe(void);
void gamestatus(int lives, int fuel, int score);
int game(int demo);
void pressanykey(void);
int instructions(void);
int about(void);
char *enterhighscorename(void);
int showhighscores(void);
void newhighscore(void);
options menu(void);
#endif /* THRUST_H */
