
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "thrust_t.h"

#define BBILDX  (41)
#define BBILDY  (23)
#define PBILDX  (BBILDX*8)
#define PBILDY  (BBILDY*8)
#define PUSEX   (PBILDX-8)
#define PUSEY   (PBILDY-8)

#define maxlenx ((word)BBILDX*12) /*max 48 skärmar a 41*23 tecken a */
#define maxleny ((word)BBILDY*4) /* 8*8 pixels a 8 bitar, dvs. 3274752 bytes.*/

extern byte *bild;

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
void putscr(int x, int y, int force_draw);
void putblock(int x, int y, byte *source);
void drawfuel(int fuel);
void drawship(word bx, word by, byte *ship, byte *storage);
void undrawship(word bx, word by, byte *storage);
void drawsquare(word bx, word by,
		byte *object, byte *storage,
		byte deltax, byte deltay);
void undrawsquare(word bx, word by,
		  byte *storage,
		  byte deltax, byte deltay);
word testcrash(byte *object, byte *storage, word len, byte shield);
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
