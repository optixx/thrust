
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>

#include "thrust_t.h"
#include "thrust.h"
#include "fast_gr.h"
#include "font5x5.h"
#include "graphics.h"

byte *bild;

static byte fuelblink;

void
putscr(int x, int y, int force_draw)
{
  static int counter=0;

  counter++;
  if(!skip_frames || force_draw || (skip_frames && counter==3)) {
    counter=0;
    if(PUSEY<=PBILDY-y)
      putarea(bild, x, y, PUSEX, PUSEY, PBILDX<<1, 0, 24);
    else {
      putarea(bild, x, y, PUSEX, PBILDY-y, PBILDX<<1, 0, 24);
      putarea(bild, x, 0, PUSEX, PUSEY-PBILDY+y, PBILDX<<1, 0, PBILDY+24-y);
    }
  }
}

void
putblock(int x, int y, byte *source)
{
  int i;
  byte *dest1, *dest2;

  dest1=bild+((y<<3)*(PBILDX<<1))+(x<<3);
  dest2=dest1+((x>=BBILDX)?-(PBILDX):(PBILDX));
  
  for(i=0; i<8; i++) {
    memcpy(dest1, source, 8);
    memcpy(dest2, source, 8);
    source+=8;
    dest1+=PBILDX<<1;
    dest2+=PBILDX<<1;
  }
}

void
drawfuel(int fuel)
{
  char str[16];
  byte tmpcol, tmppap, tmpflg;

  tmpcol=chcolor;
  tmppap=chpaper;
  tmpflg=chflag;
  chpaper=0;
  fuelblink=(fuelblink-1)&31;
  if(((fuel+9)/10)>10 || fuelblink&16)
    chcolor=0x88;
  else
    chcolor=FUELCOLOR;
  chflag=1;
  sprintf(str, "  %d", fuel);
  printgs(80-gstrlen(str), 12, str);
  chflag=tmpflg;
  chpaper=tmppap;
  chcolor=tmpcol;
}

void
drawship(word bx, word by, byte *ship, byte *storage)
{
  byte *maxtmp, *tmp, pix;
  int i,j;

  maxtmp=bild+((PBILDY-1)*PBILDX<<1);
  tmp=bild+by*(PBILDX<<1)+bx;
  for(i=0; i<256; i+=16) {
    memcpy(storage+i, tmp, 16);
    for(j=0; j<16; j++) {
      pix=*(ship++);
      if(pix)
	*(tmp+j)=pix;
    }
    if(tmp>=maxtmp)
      tmp=bild+bx;
    else
      tmp+=PBILDX<<1;
  }
}

void
undrawship(word bx, word by, byte *storage)
{
  byte *maxtmp, *tmp;
  int i;

  maxtmp=bild+((PBILDY-1)*PBILDX<<1);
  tmp=bild+by*(PBILDX<<1)+bx;
  for(i=0; i<256; i+=16) {
    memcpy(tmp, storage+i, 16);
    if(tmp>=maxtmp)
      tmp=bild+bx;
    else
      tmp+=PBILDX<<1;
  }    
}

void
drawsquare(word bx, word by,
	   byte *object, byte *storage,
	   byte deltax, byte deltay)
{
  byte *maxtmp, *tmp, pix;
  word i,j;
  word deltaxy;

  deltaxy=(word)deltax*deltay;
  maxtmp=bild+((PBILDY-1)*PBILDX<<1);
  tmp=bild+by*(PBILDX<<1)+bx;
  for(i=0; i<deltaxy; i+=(int)deltax) {
    memcpy(storage+i, tmp, (int)deltax);
    for(j=0; j<(int)deltax; j++) {
      pix=*(object++);
      if(pix)
	*(tmp+j)=pix;
    }
    if(tmp>=maxtmp)
      tmp=bild+bx;
    else
      tmp+=PBILDX<<1;
  }
}

void
undrawsquare(word bx, word by,
	     byte *storage,
	     byte deltax, byte deltay)
{
  byte *maxtmp, *tmp;
  word i;
  word deltaxy;

  deltaxy=(word)deltax*deltay;
  maxtmp=bild+((PBILDY-1)*PBILDX<<1);
  tmp=bild+by*(PBILDX<<1)+bx;
  for(i=0; i<deltaxy; i+=(int)deltax) {
    memcpy(tmp, storage+i, (int)deltax);
    if(tmp>=maxtmp)
      tmp=bild+bx;
    else
      tmp+=PBILDX<<1;
  }
}

word
testcrash(byte *object, byte *storage, word len, byte shield)
{
  word i;
  byte res=0;

  for(i=0; i<len; i++) {
    if(*(object++)) {
      if(*storage>res && (!shield || (shield && *storage<224)))
	res=*storage;
    }
    storage++;
  }
  return(((word)res)>>5);
}
