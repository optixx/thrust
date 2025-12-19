
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef THINGS_H
#define THINGS_H

#include "thrust_t.h"

struct bulletdef {
  int life,x,y,vx,vy,dir,owner;
};
typedef struct bulletdef bullet;

struct fragmentdef {
  int life,x,y,vx,vy;
};
typedef struct fragmentdef fragment;

struct thingdef	{
  int alive,px,py,x,y,type,score;
  void *data;
};
typedef struct thingdef thing;
extern uint32_t nrthings;

struct sliderdef {
  int type,x1,y1,x2,y2,dir,active,stage,count,match;
  struct sliderdef *next;
};
typedef struct sliderdef slider;
extern uint32_t nrsliders;

struct buttondatadef {
  int major;
  int tag;
  slider *sliders;
};
typedef struct buttondatadef buttondata;

struct restartpointdef {
  uint32_t x, y;
};
typedef struct restartpointdef restartpoint;
extern uint32_t nrrestartpoints;

struct barrierdef {
  uint32_t x, y;
  restartpoint *restart;
};
typedef struct barrierdef restartbarrier;
extern uint32_t nrbarriers;


#define maxbullets (64)
extern bullet bullets[maxbullets];
#define maxfragments (512)
extern fragment fragments[maxfragments];
#define maxthings (32)
extern thing things[maxthings];
#define maxsliders (32)
extern slider sliders[maxsliders];
#define maxbarriers (512)
extern restartbarrier barriers[maxbarriers];
#define maxrestartpoints (16)
extern restartpoint restartpoints[maxrestartpoints];

extern uint32_t powerplant;
extern uint32_t ppx, ppy, ppcount;	/* Power Plant variables */
extern uint32_t ppblip;

void newslider(int x, int y, int type);
int majorbutton(int button);
void newthing(int x, int y, int px, int py, int type, void *data);
void animatesliders(void);
void startupsliders(int button);
restartpoint *atbarrier(uint32_t bx, uint32_t by);
void deletething(thing *tp);
void newbullet(uint32_t x, uint32_t y, int vx, int vy, uint32_t dir, int owner);
void movebullets(void);
uint32_t crashtype(uint32_t type);
int inloadcontact(int x, int y);
int resonablefuel(int x, int y, int l);
int closestfuel(int x, int y);
int closestbutton(int x, int y);
void hit(uint32_t x, uint32_t y, uint32_t crash, uint32_t owner);
void bunkerfirebullet(thing *b);
void bunkerfirebullets(void);
int killdyingthings(void);
void killallthings(void);
void newfragment(uint32_t x, uint32_t y);
void explodething(thing *thingptr);
void explodeship(void);
void movefragments(void);
uint32_t livefragments(void);

#endif /* THINGS_H */
