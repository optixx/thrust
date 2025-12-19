
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "fast_gr.h"

#define maxlenx ((word)BBILDX*12) /*max 48 skärmar a 41*23 tecken a */
#define maxleny ((word)BBILDY*4) /* 8*8 pixels a 8 bitar, dvs. 3274752 bytes.*/

void writeblock(word bx, word by, byte block);
void drawteleline(int round, int x1, int y1, int x2, int y2, int j, int k);
void drawteleport(int tohere);
void swap(int *pa, int *pb);
void drawlinev  (int x1, int y1, int x2, int y2, byte color, byte *storage);
void undrawlinev(int x1, int y1, int x2, int y2,             byte *storage);
void drawlineh  (int x1, int y1, int x2, int y2, byte color, byte *storage);
void undrawlineh(int x1, int y1, int x2, int y2,             byte *storage);
void drawline   (int x1, int y1, int x2, int y2, byte color, byte *storage);
void undrawline (int x1, int y1, int x2, int y2,             byte *storage);
void drawbullets(void);
void undrawbullets(void);
void drawfragments(void);
void undrawfragments(void);
void drawpowerplantblip(void);
void drawload(int flag);
word drawshuttle(void);
void undrawshuttle(void);
void drawfuellines(void);
void undrawfuellines(void);
void clearscr(void);
void putarea(byte *source,
	     int x, int y, int width, int height, int bytesperline,
	     int destx, int desty);
void putpixel(int x, int y, byte color);
void syncscreen(void);
void displayscreen(void);
void fade_in(void);
void fade_out(void);
void fadepalette(int first, int last,
	 byte *RGBtable,
	 int fade, int flag);
void graphics_preinit(void);
int graphicsinit(int argc, char **argv);
int graphicsclose(void);
char *graphicsname(void);

#endif /* GRAPHICS_H */
