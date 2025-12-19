
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef GR_DRV_H
#define GR_DRV_H

#include "thrust_t.h"

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

#endif
