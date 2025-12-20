
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "helpers.h"

#define BBILDX (41)
#define BBILDY (23)
#define PBILDX (BBILDX * 8)
#define PBILDY (BBILDY * 8)
#define PUSEX (PBILDX - 8)
#define PUSEY (PBILDY - 8)

#define maxlenx ((uint32_t)BBILDX * 12) /*max 48 skärmar a 41*23 tecken a */
#define maxleny ((uint32_t)BBILDY * 4)  /* 8*8 pixels a 8 bitar, dvs. 3274752 bytes.*/

extern uint8_t* bild;

void writeblock(uint32_t bx, uint32_t by, uint8_t block);
void drawteleline(int round, int x1, int y1, int x2, int y2, int j, int k);
void drawteleport(int tohere);
void swap(int* pa, int* pb);
void drawlinev(int x1, int y1, int x2, int y2, uint8_t color, uint8_t* storage);
void undrawlinev(int x1, int y1, int x2, int y2, uint8_t* storage);
void drawlineh(int x1, int y1, int x2, int y2, uint8_t color, uint8_t* storage);
void undrawlineh(int x1, int y1, int x2, int y2, uint8_t* storage);
void drawline(int x1, int y1, int x2, int y2, uint8_t color, uint8_t* storage);
void undrawline(int x1, int y1, int x2, int y2, uint8_t* storage);
void drawbullets(void);
void undrawbullets(void);
void drawfragments(void);
void undrawfragments(void);
void drawpowerplantblip(void);
void drawload(int flag);
uint32_t drawshuttle(void);
void undrawshuttle(void);
void drawfuellines(void);
void undrawfuellines(void);
void putscr(int x, int y, int force_draw);
void putblock(int x, int y, uint8_t* source);
int insideblock(int blkx, int blky, int pblkx, int pblky, int sx, int sy);
int insidepixel(int x, int y, int pixx, int pixy, int sx, int sy);
void updateborder(int pblkx, int pblky, int bblkx, int bblky, int vx, int vy);
void drawfuel(int fuel);
void drawship(uint32_t bx, uint32_t by, uint8_t* ship, uint8_t* storage);
void undrawship(uint32_t bx, uint32_t by, uint8_t* storage);
void drawsquare(uint32_t bx, uint32_t by, uint8_t* object, uint8_t* storage, uint8_t deltax,
                uint8_t deltay);
void undrawsquare(uint32_t bx, uint32_t by, uint8_t* storage, uint8_t deltax, uint8_t deltay);
uint32_t testcrash(uint8_t* object, uint8_t* storage, uint32_t len, uint8_t shield);
void clearscr(void);
void putarea(uint8_t* source, int x, int y, int width, int height, int bytesperline, int destx,
             int desty);
void putpixel(int x, int y, uint8_t color);
void syncscreen(void);
void displayscreen(void);
void fade_in(void);
void fade_out(void);
void fadepalette(int first, int last, uint8_t* RGBtable, int fade, int flag);
void graphics_preinit(void);
int graphicsinit(int zoom);
int graphicsclose(void);
char* graphicsname(void);
void graphics_set_smooth(int enable);

#endif /* GRAPHICS_H */
