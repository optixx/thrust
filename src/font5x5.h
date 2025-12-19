
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef FONT5X5_H
#define FONT5X5_H

#include "thrust_t.h"

extern uint8_t font[256][6];

#define gcenter(y,s) (printgs(160-(gstrlen(s)>>1), (y), (s)))

extern uint8_t chcolor, chpaper, chflag;

void printgc(int x, int y, uint8_t ch);
int printgs(int x, int y, char *string);
int readgs(int x, int y, char string[], int maxc, int maxp, char flag);
int gstrlen(char *string);

#endif /* FONT5X5_H */
