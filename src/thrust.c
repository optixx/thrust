
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "thrust_t.h"
#include "thrust.h"
#include "hiscore.h"
#include "graphics.h"
#include "graphics.h"
#include "graphics.h"
#include "font5x5.h"
#include "things.h"
#include "conf.h"
#include "init.h"
#include "level.h"
#include "keyboard.h"
#include "options.h"

#ifdef HAVE_SOUND
# include "soundIt.h"

# define CHAN_1 0
# define CHAN_2 1
# define CHAN_3 2
# define CHAN_4 3
# define SND_BOOM   0
# define SND_BOOM2  1
# define SND_HARP   2
# define SND_THRUST 3
# define SND_ZERO   4
#endif

byte *bulletmap;
byte *blocks;
byte *ship;
byte *shieldship;
byte *bana;
byte *fuelmap;
byte *loadmap;
byte *shipstorage;
byte *bulletstorage;
byte *fragmentstorage;
byte *fuelstorage;
byte *loadstorage;
byte *wirestorage;

word lenx; /* x-size of level */
word leny; /* y-size of level */
word lenx3, leny3;
             /* Status of game. */
double alpha, deltaalpha;
word loaded, loadcontact, loadpointshift;
int loadpoint;
int countdown;
word crash, shoot, repetetive;
word refueling;
int speedx, speedy;
long absspeed, oldabs;
int kdir, dir;
int shipdx, shipdy;
int x, y;              /* Top left corner, 8 units per pixel. */
int pixx, pixy;        /* Top left corner in pixels.   */
int pblockx, pblocky;  /* Top left corner in blocks (8x8 pixels). */
int vx, vy;				/* Speed of the ship. */
int bildx, bildy;      /* Top left corner of backing store (in pixels). */
int bblockx, bblocky;  /* Top left corner of backing store (in blocks). */
int loadbx, loadby;    /* Position of the load (in blocks). */
int gravity;
int score;
byte shield;
byte colorr, colorg, colorb;
int nodemo=0;
int Thrust_Is_On=0;
int play_sound=1;
double gamma_correction=1.0;
int skip_frames=0;

#define checkfork(b, a) \
  case b-1: \
    if(easyrider==a || easyrider==a+1) \
      easyrider=a+1; \
    else if(easyrider!=9) \
      easyrider=-1; \
    break

int
insideblock(int blkx, int blky, int pblkx, int pblky,
            int sx, int sy)
{
  return((blkx>=pblkx-sx) && (blkx<pblkx+BBILDX) &&
         (blky>=pblky-sy) && (blky<pblky+BBILDY));
}

int
insidepixel(int x, int y, int pixx, int pixy, int sx, int sy)
{
  return((x>pixx-sx) && (x<pixx+PUSEX) &&
         (y>pixy-sy) && (y<pixy+PUSEY));
}

void
updateborder(int pblkx, int pblky, int bblkx, int bblky,
             int vx, int vy)
{
  word k;

  if(vy<=0) /* update bottom border */
    for(k=0; k<BBILDX; k++)
      putblock(bblkx+k, (bblky+BBILDY-1)%BBILDY, blocks+
               (*(bana+(pblkx+k)%lenx+((pblky+BBILDY-1)%leny)*lenx)<<6));
  else      /* update top border */
    for(k=0; k<BBILDX; k++)
      putblock(bblkx+k, bblky, blocks+
               (*(bana+(pblkx+k)%lenx+(pblky%leny)*lenx)<<6));
  if(vx>0)  /* update right border */
    for(k=0; k<BBILDY; k++)
      putblock(bblkx+BBILDX-1, (bblky+k)%BBILDY, blocks+
               (*(bana+(pblkx+BBILDX-1)%lenx+((pblky+k)%leny)*lenx)<<6));
  else      /* update left border */
    for(k=0; k<BBILDY; k++)
      putblock(bblkx, (bblky+k)%BBILDY, blocks+
               (*(bana+pblkx%lenx+((pblky+k)%leny)*lenx)<<6));
}

void
pause_message(void)
{
  char *str[] = {
    "Game paused.",
    "Press 'C' to continue..." };

  chflag=1;
  gcenter(80, str[0]);
  gcenter(91, str[1]);
  chflag=0;
  displayscreen();
}

void
escape_message(void)
{
  char *str[] = {
    "Are you sure you want to quit (Y/N)?" };

  chflag=1;
  gcenter(85, str[0]);
  chflag=0;
  displayscreen();
}

byte
whatkeys(void)
{
  byte keys;

/* Use this to create a demo.
 * Don't forget to uncomment the fputc statement below.
 */
  /*
  static FILE *f=NULL;
  if(f == NULL)
    f=fopen("foobarfoo", "w");
  */

  keys = getkeys();

  /*
  fputc((keys&escape_bit)?quit_bit:keys, f);
  */
  return(keys);
}

byte
nextmove(int reset)
{
  static byte *p;
  byte retbits=0;

  if(reset) {
    keywaiting();
    p=&bin_demomove[0];
  }
  else if(keywaiting()) {
    retbits=quit_bit;
    flushkeyboard();
  }
  else {
    retbits=*(p++);
    retbits&=~thrust_bit;
    retbits|=*(p-!!(random()%20)) & thrust_bit;
  }

  return(retbits);
}

void
gamestatusframe(void)
{
  int i, maxx;

  for(i=3; i<317; i++)
    putpixel(i, 3, 0x88);
  for(i=3; i<11; i++) {
    putpixel(  3, i, 0x88);
    putpixel(  4, i, 0x88);
    putpixel(315, i, 0x88);
    putpixel(316, i, 0x88);
  }
  for(i=3; i<11; i++) {
    putpixel(    i, i+8, 0x88);
    putpixel(  1+i, i+8, 0x88);
    putpixel(318-i, i+8, 0x88);
    putpixel(319-i, i+8, 0x88);
  }
  for(i=9; i<311; i++)
    putpixel(i, 18, 0x88);
  maxx = 80-gstrlen("FUEL")-2;
  for(i=6; i<maxx; i++) {
    putpixel(i, 6, 65);
    putpixel(i, 8, 65);
  }
  maxx = 160-gstrlen("LIVES")/2-2;
  for(i=82; i<maxx; i++) {
    putpixel(i, 6, 65);
    putpixel(i, 8, 65);
  }
  maxx = 280-gstrlen("SCORE")-2;
  for(i=160+gstrlen("LIVES")/2+2; i<maxx; i++) {
    putpixel(i, 6, 65);
    putpixel(i, 8, 65);
  }
  for(i=282; i<313; i++) {
    putpixel(i, 6, 65);
    putpixel(i, 8, 65);
  }
  chcolor = 0x89;
  printgs(80-gstrlen("FUEL"), 5, "FUEL");
  gcenter(5, "LIVES");
  printgs(280-gstrlen("SCORE"), 5, "SCORE");
  chcolor = TEXTCOLOR;
}

void
gamestatus(int lives, int fuel, int score)
{
  static char textstr[40];

  chcolor = 0x88;
  drawfuel(fuel);
  sprintf(textstr, "%d", lives);
  gcenter(12, textstr);
  sprintf(textstr, "%d", score);
  printgs(280-gstrlen(textstr), 12, textstr);
  chcolor = TEXTCOLOR;
}

int
game(int demo)
{
  byte actionbits=0;
  double ax, ay, acircum;
  word lives, endlevel;
  word dying;
  word alive;
  word fuel;
  int l;
  int level;
  int round;
  static char **levels[LEVELS]
    = { level1, level2, level3, level4, level5, level6 };
  static char textstr[40];
  int localscore;
  options end;
  int ch;
  int lastlevel;
  int easyrider=0;
  int restartx=0, restarty=0;
  int loadedrestart=0;
  restartpoint *restartxy;
  int gravitymsg;
  int visibilitymsg;
  int teleport;

  if(demo)
    nextmove(1);

  lives=3; /* 3 */
  localscore=0;
  score=0;
  round=0; /* 0 */
  level=0; /* 0 */
  lastlevel=-1;
  shield=0;
  fuel=1000; /* 1000 */
  gravitymsg=0;
  visibilitymsg=0;
  teleport=0;

  while(level<LEVELS && lives>0 && fuel) {
#ifdef DEBUG2
    printf("Newlevel: %d\n", level);
#endif
    endlevel = 0;
    dying    = 0;
    alive    = 1;

    srandom(time(NULL));
    if(level!=lastlevel || !powerplant) {
      if(level==0 && lastlevel>0) {
        gravitymsg = 1;
        if(round == 2)
          visibilitymsg = 1;
      }
      if(!readbana(levels[level])) {
        printf("Illegal definition of level %d.\n", level+1);
        return(1);
      }
      restartx = (lenx+restartpoints[0].x-(154>>3))%lenx;
      restarty = restartpoints[0].y-(82>>3)-4*(round&1);
      loadedrestart = 0;
      initgame(round, 1, restartx, restarty);
    }
    else {
      loaded = loadedrestart;
      initgame(round, 0, restartx, restarty);
    }

    initscreen(round);
    putscr(pixx%PBILDX, pixy%PBILDY, 1);
    lastlevel = level;

    gamestatusframe();
    gamestatus(lives, fuel, score);
    sprintf(textstr, "Mission %d",  level+1 + round*6);
    gcenter(70, textstr);
    if(gravitymsg) {
      gcenter(60, (round&1) ? "Reversed gravity" : "Normal gravity");
      if(visibilitymsg)
        gcenter(52, (round&2) ? "Invisible ground" : "Visible ground");
    }
    displayscreen();
    syncscreen();
    fade_in();
    usleep(1000000UL + 2000000UL*gravitymsg);
    gravitymsg=0;
    syncscreen();
    putscr(pixx%PBILDX, pixy%PBILDY, 1);
    drawteleport(1);
    displayscreen();
    while(!endlevel) {
      actionbits=demo ? nextmove(0) : whatkeys();

      if(alive && (actionbits&right_bit)) {
        decr(kdir, 0, 96);
        dir=kdir/3;
      }
      if(alive && (actionbits&left_bit)) {
        incr(kdir, 96, 0);
        dir=kdir/3;
      }
      if(alive && (actionbits&fire_bit)) {
        if(!shoot) {
          shoot=1;
          newbullet((word)(x+((160+shipdx)<<3)+74*cos(dir * M_PI/16)),
        	    (word)(y+(( 88+shipdy)<<3)-74*sin(dir * M_PI/16)),
        	    (int)(speedx/256.0+32*cos(dir * M_PI/16)),
        	    (int)(speedy/256.0+32*sin(dir * M_PI/16)),
        	    kdir/6, 1);
        }
        else if(repetetive || easyrider)
          shoot=0;
      }
      else
        shoot=0;
      refueling=0;
      if(alive && (actionbits&pickup_bit)) {
        if(fuel>0) {
          if(shield==0 && round&2) {
            bin_colors[65*3+0]=GAMMA(colorr);
            bin_colors[65*3+1]=GAMMA(colorg);
            bin_colors[65*3+2]=GAMMA(colorb);
            fadepalette(0, 255, bin_colors, 64, 0);
          }
          if(shield++==3) {
#if !(defined(DEBUG) || defined(DEBUG2))
            if(!easyrider)
              fuel--;
#endif
            shield=1;
          }
        }
        else
          shield=0;
        l=closestfuel((pixx+shipdx+160)%lenx3,
        	      (pixy+shipdy+88)%leny3);
        if(l>=0)
          if(resonablefuel((pixx+shipdx+160)%lenx3,
        		   (pixy+shipdy+88)%leny3, l)) {
#ifndef DEBUG
            if(!easyrider)
              fuel+=6;
#endif
            refueling=1;
            things[l].alive--;
            if(things[l].alive==1)
              things[l].score=300;
          }
        if(!loaded)
          if(inloadcontact((pixx+shipdx+160)%lenx3,
        		   (pixy+shipdy+88)%leny3)) {
            loadcontact=1;
            *(bana+lenx*loadby+loadbx)=32;
            drawload(0);
          }
      }
      else {
        if(shield!=0 && round&2) {
          bin_colors[65*3+0]=0;
          bin_colors[65*3+1]=0;
          bin_colors[65*3+2]=0;
          fadepalette(0, 255, bin_colors, 64, 0);
        }
        shield=0;
        if(alive && loadcontact) {
          *(bana+lenx*loadby+loadbx)=109;
          drawload(1);
          loadcontact=0;
        }
      }
      if(alive && (actionbits&thrust_bit)) {
        if(fuel>0) {
#ifdef HAVE_SOUND
          if(play_sound)
            if(Thrust_Is_On==0) {
              Thrust_Is_On=1;
              Snd_effect(SND_THRUST, CHAN_1);
            }
#endif
#if !(defined(DEBUG) || defined(DEBUG2))
          if(!easyrider)
            fuel--;
#endif
          oldabs=speedx*(long)speedx+speedy*(long)speedy;

          if(loaded) { /* Ship and blob */
            acircum  = sin(dir * M_PI/16 - alpha);
            deltaalpha += SPEED/2 * acircum * M_PI/262144;
            ax=SPEED/2 * cos(dir * M_PI/16) / (1 + REL_MASS);
            ay=SPEED/2 * sin(dir * M_PI/16) / (1 + REL_MASS);
          }
          else {       /* Ship, no blob */
            ax=SPEED/2 * cos(dir * M_PI/16);
            ay=SPEED/2 * sin(dir * M_PI/16);
          }

          speedx+=(int)ax;
          speedy+=(int)ay;
          absspeed=speedx*(long)speedx+speedy*(long)speedy;
          if(absspeed>1000000000L && absspeed>oldabs) {
            speedx-=(int)ax;
            speedy-=(int)ay;
          }
        }
        else {
#ifdef HAVE_SOUND
          if(play_sound)
            if(Thrust_Is_On == 1) {
              Snd_effect(SND_ZERO, CHAN_1);
              Thrust_Is_On = 0;
            }
#endif
        }
      }
      else {
#ifdef HAVE_SOUND
        if(play_sound)
          if(Thrust_Is_On == 1) {
            Snd_effect(SND_ZERO, CHAN_1);
            Thrust_Is_On = 0;
          }
#endif
      }
      if(actionbits&quit_bit) {
#ifdef DEBUG2
        printf("Endlevel: User pressed quit-key\n");
#endif
        endlevel=1;
      }

      if(actionbits&pause_bit) {
#ifdef HAVE_SOUND
        if(play_sound) {
          Thrust_Is_On=0;
          Snd_effect(SND_ZERO, CHAN_1);
        }
#endif
        pause_message();
        end=NOTHING;
        flushkeyboard();
        while(end==NOTHING) {
          ch=wait_for_key();
          switch(islower(ch) ? ch : tolower(ch)) {
          case 'p':
            if(easyrider!=9)
              easyrider=0;
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
            end=PLAY;
            break;
          }
        }
        if(easyrider!=9)
          easyrider=0;
      }
      if(actionbits&escape_bit) {
#ifdef HAVE_SOUND
        if(play_sound) {
          Thrust_Is_On=0;
          Snd_effect(SND_ZERO, CHAN_1);
        }
#endif
        escape_message();
        end=NOTHING;
        flushkeyboard();
        while(end==NOTHING) {
          ch=wait_for_key();
          switch(islower(ch) ? ch : tolower(ch)) {
          case 'y':
            end=END;
#ifdef DEBUG2
            printf("Endlevel: User answered yes efter ESC.\n");
#endif
            endlevel=1;
            level=LEVELS;
            break;
          case 'n':
            end=PLAY;
            break;
          }
        }
      }


      if(loaded) {
        if(loadpointshift) {
          speedx+=shipdx*12;
          speedy+=shipdy*12;
        }
        alpha+=deltaalpha;
        if(alpha>2*M_PI)
          alpha-=2*M_PI;
        if(alpha<0)
          alpha+=2*M_PI;
        loadpointshift=0;
        if(++loadpoint>126)
          loadpoint=126;
        else
          loadpointshift=1;
        shipdx=(int)( cos(alpha)*loadpoint/5.90625);
        shipdy=(int)(-sin(alpha)*loadpoint/5.90625);
        if(loadpointshift) {
          speedx-=shipdx*12;
          speedy-=shipdy*12;
        }
        deltaalpha-=deltaalpha/1024;
      }
      else
        shipdx=shipdy=0;
      /* Gravity and Aerodynamics */
      if(speedx>0)
        speedx=speedx-(speedx>>9)-1;
      else if(speedx<0)
        speedx=speedx-(speedx>>9)+1;
      if(alive) {
        if(gravity>=0)
          speedy-=(SPEED*gravity+1)>>8;
        else
          speedy-=(SPEED*gravity>>8)+1;
        if(speedy>0)
          speedy--;
        else if(speedy<0)
          speedy++;
        /* Move the Ship */
        speedx=min(speedx, 16384);
        speedx=max(speedx, -16384);
        speedy=min(speedy, 16384);
        speedy=max(speedy, -16384);
        if(speedx>=0)
          vx=(speedx+1)>>8;
        else
          vx=(speedx>>8)+1;
        if(speedy>=0)
          vy=(speedy+1)>>8;
        else
          vy=(speedy>>8)+1;
        x=(x+vx+(lenx<<6))%(lenx<<6);
        y=(y-vy+(leny<<6))%(leny<<6);
      }

      /* Bunkerfire */
      if(!ppblip)
        bunkerfirebullets();
      movebullets();
      movefragments();
      drawfuel(fuel);

      /* Move the Power Plant blip */
      ppcount=(ppcount+1)&15;
      if(!ppcount && powerplant && ppblip)
        ppblip--;

      if(!powerplant) {
        countdown--;
        if(countdown<0) {
#ifndef DEBUG
          if(alive && !easyrider) {
            dying=1;
#ifdef DEBUG2
            printf("Dying: Power Plant countdown.\n");
#endif
          }
#endif
        }
        else {
          chflag=1;
          sprintf(textstr, "%d  ", (countdown+99)/100);
          printgs(105, 12, textstr);
          printgs(205, 12, textstr);
          chflag=0;
        }
      }

      /* Precalculate some values */
      pixx=x>>3;
      pixy=y>>3;
      bildx=(pixx+PBILDX-4)%PBILDX+4;
      bildy=pixy%PBILDY;
      pblockx=pixx>>3;
      pblocky=pixy>>3;
      bblockx=bildx>>3;
      bblocky=bildy>>3;

      if(pblocky>(int)leny-3) {
        endlevel=1;
#ifdef DEBUG2
        printf("Endlevel: Finished level.\n");
#endif
        teleport=1;
        y=0;
        pixy=0;
        pblocky=0;
      }

      /* Check if at a restart barrier. If so, update the restart point. */
      restartxy=atbarrier((pblockx+((154+shipdx)>>3))%lenx,
        		  pblocky+((82+shipdy)>>3));
      if(restartxy) {
        restartx=(lenx+restartxy->x-(154>>3))%lenx;
        restarty=restartxy->y-(82>>3);
        loadedrestart=loaded;
      }

      /* Scroll the screen */
      updateborder(pblockx, pblocky, bblockx, bblocky, vx, vy);

      drawpowerplantblip();
      drawbullets();
      if(alive)
        crash=drawshuttle();
      drawfragments();
      if(alive && refueling)
        drawfuellines();
      /* Check if end of life. */
#ifndef DEBUG
      if(!easyrider)
        if(alive && crash) {
          lives--;
          dying=1;
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
      if(alive && refueling)
        undrawfuellines();
      undrawfragments();
      if(alive)
        undrawshuttle();
      undrawbullets();

      /* Remove objects */
      if(!easyrider)
        localscore+=killdyingthings();
      else
        killdyingthings();
      if(dying) {
        alive=0;
        dying=0;
#ifdef HAVE_SOUND
        if(play_sound) {
          Snd_effect(SND_ZERO, CHAN_1);
          Thrust_Is_On=0;
          Snd_effect(SND_BOOM2, CHAN_2);
        }
#endif
        explodeship();
      }
      if(!alive && !livefragments()) {
#ifdef HAVE_SOUND
        if(play_sound) {
          Snd_effect(SND_ZERO, CHAN_1);
          Thrust_Is_On=0;
        }
#endif
        endlevel=1;
#ifdef DEBUG2
        printf("Endlevel: Shit crashed.\n");
#endif
      }
      animatesliders();
      if(localscore>score) {
        chflag=1;
        if(localscore/10000 > score/10000)
          lives++;
        score=localscore;
        gamestatus(lives, fuel, score);
        chflag=0;
      }
    }
    if(teleport) {
#ifdef HAVE_SOUND
      if(play_sound)
        Snd_effect(SND_ZERO, CHAN_1);
#endif
      bin_colors[65*3+0]=GAMMA(colorr);
      bin_colors[65*3+1]=GAMMA(colorg);
      bin_colors[65*3+2]=GAMMA(colorb);
      fadepalette(0, 255, bin_colors, 64, 1);
      drawteleport(0);
    }

    if(!(actionbits&(quit_bit|escape_bit)))
      usleep(1000000UL);
    fade_out();

    if(!demo && !(actionbits&(quit_bit|escape_bit))) {
      if(teleport || !powerplant) {
        chflag=1;
        gamestatusframe();
        gamestatus(lives, fuel, score);

        if(!powerplant) {
          sprintf(textstr, "Planet destroyed");
          gcenter(61, textstr);
        }

        if(teleport && loaded)
          sprintf(textstr, "Mission %d complete", level+1 + round*6);
        else if(powerplant)
          sprintf(textstr, "Mission incomplete");
        else
          sprintf(textstr, "Misson  %d  failed", level+1 + round*6);
        gcenter(73-6*(teleport && loaded && powerplant), textstr);

        if((teleport && loaded) || !powerplant) {
          if(teleport && loaded)
            sprintf(textstr, "Bonus %d", 4000+400*level-2000*powerplant);
          else
            sprintf(textstr, "No bonus");
          gcenter(85-6*(!!powerplant), textstr);
        }

        displayscreen();
        fade_in();
        usleep(2000000UL);
        if(!easyrider && teleport && loaded)
          localscore+=4000+400*level-2000*powerplant;
        if((teleport && loaded) || !powerplant) {
          if(++level==LEVELS) {
            level=0;
            round=(round+1)%4;
          }
        }
        if(localscore/10000 > score/10000)
          lives++;
        score=localscore;
        gamestatus(lives, fuel, score);
        chflag=0;
        displayscreen();
        usleep(2000000UL);
        fade_out();
      }
    }
    teleport=0;

    if(demo)
      level=LEVELS;
  }

  if(!demo) {
    chflag=1;
    gamestatusframe();
    gamestatus(lives, fuel, score);

    gcenter(73, "Game Over");

    displayscreen();
    fade_in();
    usleep(2000000UL);
    fade_out();
  }

  return(0);
}

void
pressanykey(void)
{
  wait_for_key();
}

int
instructions(void)
{
  int i;
  static char *keys[] = {
    "Esc", "P", "C" };
  static char *func[] = {
    "Turn left", "Turn right", "Thrust", "Fire",
    "Pick up & Shield", "Quit Game (Q=Esc)", "Pause", "Continue" };
  static char *story[] = {
    "The resistance is about to launch a major offensive against",
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
    NULL };
  static char *score[] = {
    "Destroying a limpet gun:",
    "Destroying a fuel cell:",
    "Picking up a fuel cell:",
    "Bonus for destroying planet:",
    "",
    "A spare ship is allocated for every:"
  };
  static char *scores[] = {
    "750",
    "150",
    "300",
    "2000 + Mission Bonus",
    "",
    "10000"
  };

  chcolor = HIGHLIGHT;
  gcenter(20, "THE SILLY STORY!");
  chcolor = TEXTCOLOR;
  for(i=0; story[i]; i++)
    gcenter(35 + i*8, story[i]);

  gcenter(180, "Press any key for the next page.");

  fade_in();
  pressanykey();
  fade_out();

  chcolor = HIGHLIGHT;
  gcenter(40, "SCORING");
  chcolor = TEXTCOLOR;
  for(i=0; i<6; i++) {
    chcolor = TEXTCOLOR;
    printgs(200-gstrlen(score[i]), 65 + 8*i, score[i]);
    chcolor = HIGHLIGHT;
    printgs(205, 65 + 8*i, scores[i]);
  }

  chcolor = TEXTCOLOR;
  gcenter(130, "Press any key for the next page.");

  fade_in();
  pressanykey();
  fade_out();

  gcenter(50, "The following keys are used:");
  for(i=0; i<8; i++) {
    chcolor = HIGHLIGHT;
    if(i<5)
      printgs(140-gstrlen(keystring(scancode[i])),
              63+i*8+2*(i>4),
              keystring(scancode[i]));
    else
      printgs(140-gstrlen(keys[i-5]), 63+i*8+2*(i>4), keys[i-5]);
    chcolor = TEXTCOLOR;
    printgs(145, 63+i*8+2*(i>4), func[i]);
  }
  gcenter(150, "Press any key for the main menu.");

  fade_in();
  pressanykey();
  fade_out();

  return(0);
}

int
about(void)
{
  int i;
  static char *str[] = {
    "Thrust version " VERSION,
    "",
    "Written by",
    "",
    "Peter Ekberg",
    "peda@lysator.liu.se",
    "",
    "Thanks to the authors",
    "of the original",
    "for the C64.",
    NULL
  };

  for(i=0; str[i]; i++) {
    if(i==5)
      chcolor = HIGHLIGHT;
    else
      chcolor = TEXTCOLOR;
    gcenter(40+9*i, str[i]);
  }
  gcenter(145, "Press any key for the main menu.");

  fade_in();
  pressanykey();
  fade_out();

  return(0);
}

char *
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

  if(readgs(130, 97, name, 39, 80, 0)==-1)
    strcpy(name, standardname());

  fade_out();

  return(name);
}

int
showhighscores(void)
{
  char str[100];
  byte tmp=chcolor;
  int i;
  int scorew, namew;
  int len;

  gcenter(50, "The current highscores are");

  scorew=namew=0;
  for(i=0; i<HIGHSCORES; i++) {
    sprintf(str, "%d", highscorelist[i].score);
    len=gstrlen(str);
    if(len>scorew)
      scorew=len;
    len=gstrlen(highscorelist[i].name);
    if(len>namew)
      namew=len;
  }

  for(i=0; i<HIGHSCORES; i++) {
    sprintf(str, "%d", highscorelist[i].score);
    chcolor = SCORETEXT;
    printgs(155+(scorew-namew)/2-gstrlen(str), 70+11*i, str);
    chcolor = SCORENAME;
    printgs(165+(scorew-namew)/2, 70+11*i, highscorelist[i].name);
  }

  chcolor=tmp;
  gcenter(145, "Press any key for the main menu.");

  fade_in();
  pressanykey();
  fade_out();

  return(0);
}

void
newhighscore(void)
{
  char *name;

  name = enterhighscorename();
  inserthighscore(name, score);
  writehighscores();
  showhighscores();
}

options
menu(void)
{
  int i;
  options end=NOTHING;
  int ch;
  static char *menuchoises[NOTHING]= { "I", "C", "H", "P", "A", "Q" };
  static char *menuoptions[NOTHING]= {
    "Instructions", "Configuration", "Highscores",
    "Play Game", "About", "Quit" };
  int count=0;

  clearscr();
  putarea(title_pixels, 0, 0, title_cols, title_rows, title_cols,
          (PUSEX-title_cols)>>1, (PUSEY+24-title_rows)>>1);
  for(i=0; i<NOTHING; i++) {
    chcolor = TEXTCOLOR;
    printgs(20, 134+i*11, menuoptions[i]);
    chcolor = HIGHLIGHT;
    printgs(20, 134+i*11, menuchoises[i]);
  }
  printgs(60, 35, "GRATTIS CALLE!");
  chcolor = TEXTCOLOR;

  fade_in();

  while(end==NOTHING) {
    ch=getkey();
    switch(islower(ch) ? ch : tolower(ch)) {
    case 0:
      break;
    case 'i':
      end=INST;
      break;
    case 'p':
      end=PLAY;
      break;
    case 'h':
      end=HI;
      break;
    case 'a':
      end=ABOUT;
      break;
    case 'c':
      end=CONF;
      break;
    case 'd':
      end=DEMO;
      break;
    case 'q':
    case 27:
      end=END;
      break;
    default:
      count=0;
      break;
    }
    usleep(50000UL);

    if(++count==160 && end==NOTHING) {
      if(!nodemo)
        end=DEMO;
      count=0;
    }
  }

  fade_out();

  return(end);
}

int
main(int argc, char **argv)
{
  int end=0;
  int optc;

  window_zoom = 1;

  do {
    static struct option longopts[] = {
      OPTS,
      { 0, 0, 0, 0 }
    };

    optc=getopt_long_only(argc, argv, OPTC SDL_OPTC, longopts, (int *) 0);
    switch(optc) {
    case 'e':      /* --nosoundeffects */
      play_sound=0;
      break;
    case 'd':      /* --nodemo */
      nodemo=1;
      break;
    case 'c':      /* --gamma */
      {
        char *end;
        double tmp;

        if(*optarg) {
          tmp = strtod(optarg, &end);
          if(*end || tmp==HUGE_VAL || tmp<=0.0)
            tmp = 0.0;
        }
        else
          tmp = 0.0;

        if(tmp == 0.0)
          printf("Illegal gamma correction value: \"%s\"\n", optarg);
        else
          gamma_correction = tmp;
      }
      break;
    case 'j':      /* --step */
      skip_frames = 1;
      break;
    case 'z':      /* --zoom */
      {
        int z = atoi(optarg);
        if(z < 1 || z > 6) {
          printf("Zoom must be between 1 and 6.\n");
          exit(1);
        }
        window_zoom = z;
      }
      break;
  case 'h':      /* --help */
    printf("Thrust: version " VERSION " -- the Game\n");
    printf("Using %s to drive the graphics and\n"
           "      %s to drive the keyboard.\n\n",
           graphicsname(),
           keyname());
    printf("usage: thrust [OPTION]...\n\n"
           "  -v, --version\n"
           "  -h, --help\n"
           "  -d, --nodemo           Do not run the demo.\n"
           "  -e, --nosoundeffects   Do not play sound effects.\n"
           "  -c, --gamma=Value      Gamma correction of colors.\n"
           "  -j, --step             Only draw every third frame (faster).\n"
           "  -z, --zoom=1..6        Scale the window by an integer factor.\n");
    printf("\n");
    exit(1);
    case 'v':      /* --version */
      printf("Thrust: version " VERSION "\n");
      exit(0);
    case EOF:
      if(optind == argc)
        break;
    default:
      fprintf(stderr, "Thrust: bad usage (see 'thrust -h')\n");
      exit(1);
    }
  } while(optc != EOF);

  graphics_preinit();
  inithardware(argc, argv);

  if(!initmem()) {
    restorehardware();
    return(1);
  }
  inithighscorelist();
  initkeys();

  sleep(1);

  while(!end) {
    switch(menu()) {
    case INST:
      instructions();
      break;
    case PLAY:
      if(!(end=game(0)))
        if(ahighscore(score))
          newhighscore();
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
    case DEMO:
      game(1);
      break;
    case END:
      end=1;
      break;
    default:
      break;
    }
  }

  restoremem();
  restorehardware();

  return(0);
}
