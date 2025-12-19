// sdl output for thrust
// (adapted from tank [ https://github.com/himdel/tank/ ])
// input in sdlkey.c

// Written by Martin Hradil, himdel@seznam.cz

#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include <getopt.h>

#include <SDL/SDL.h>
#include "thrust.h"
#include "gr_drv.h"
#include "options.h"

static const int X = 320;
static const int Y = 200;
static int double_size = 0;

static SDL_Surface *scr = NULL;


// used for output-specific options
char *
graphicsname()
{
	static char name[] = "SDL";
	return name;
}

// run before init, empty
void
graphics_preinit()
{
	;
}

// parse options, init graphics
int
graphicsinit(int argc, char **argv)
{
	int optc;

	optind = 0;		// reset getopt parser
	do {
		static struct option longopts[] = {
			OPTS,
			SDL_OPTS,
			{ 0, 0, 0, 0 }
		};

		optc = getopt_long_only(argc, argv, OPTC SDL_OPTC, longopts, (int *) 0);
		if (optc == '2')
			double_size = 1;
	} while (optc != EOF);

	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return -1;
	}

	SDL_WM_SetCaption("thrust", "thrust");

	scr = SDL_SetVideoMode(X * (double_size ? 2 : 1), Y * (double_size ? 2 : 1), 8, SDL_SWSURFACE);
	assert(scr);

	return 0;
}

// close graphics
int
graphicsclose()
{
	SDL_Quit();
	scr = NULL;

	return 0;
}

// clear screen
void
clearscr()
{
	if (SDL_MUSTLOCK(scr))
		SDL_LockSurface(scr);

	for (int x = 0; x < X; x++)
		for (int y = 0; y < Y; y++)
			putpixel(x, y, 0);

	if (SDL_MUSTLOCK(scr))
		SDL_UnlockSurface(scr);

	// displayscreen does the actual repaint
	displayscreen();
}

// flip buffer
void
displayscreen()
{
	SDL_UpdateRect(scr, 0, 0, 0, 0);
}

// displayscreen and wait
void
syncscreen()
{
	struct timeval tmp;
	static int old = -1;

	if (old == -1) {
		gettimeofday(&tmp, NULL);
		old = tmp.tv_usec;
	}

	displayscreen();

	gettimeofday(&tmp, NULL);
	int new = tmp.tv_usec;
	int diff = (old - new + 1000000) % 1000000;

	if (diff > 20000)
		usleep(diff % 20000);

	gettimeofday(&tmp, NULL);
	old = tmp.tv_usec;
}

// actually paints a pixel, no locking
static void
_putpixel(int x, int y, byte color)
{
	if (!double_size) {
		// single pixel
		*((Uint8 *) scr->pixels + (scr->pitch * y) + x) = color;
	} else {
		// double size
		*((Uint8 *) scr->pixels + (scr->pitch * (2 * y)) + (2 * x)) = color;
		*((Uint8 *) scr->pixels + (scr->pitch * (2 * y)) + (2 * x + 1)) = color;
		*((Uint8 *) scr->pixels + (scr->pitch * (2 * y + 1)) + (2 * x)) = color;
		*((Uint8 *) scr->pixels + (scr->pitch * (2 * y + 1)) + (2 * x + 1)) = color;
	}
}

// paint a pixel
void
putpixel(int x, int y, byte color)
{
	if (SDL_MUSTLOCK(scr))
		SDL_LockSurface(scr);

	_putpixel(x, y, color);

	if (SDL_MUSTLOCK(scr))
		SDL_UnlockSurface(scr);
}

// copy an area
void
putarea(byte *source, int x, int y, int width, int height, int bytesperline, int destx, int desty)
{
	if (SDL_MUSTLOCK(scr))
		SDL_LockSurface(scr);

	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
			_putpixel(destx + i, desty + j, source[bytesperline * (y + j) + x + i]);

	if (SDL_MUSTLOCK(scr))
		SDL_UnlockSurface(scr);
}

// set/fade pallete
// fade ignored for now
void
fadepalette(int first, int last, byte *RGBtable, int fade, int flag)
{
	int n = last - first + 1;
	SDL_Color *col = calloc(n, sizeof(SDL_Color));

	for (int foo = 0; foo < n; foo++) {
		col[foo].r = RGBtable[3 * foo + 0];
		col[foo].g = RGBtable[3 * foo + 1];
		col[foo].b = RGBtable[3 * foo + 2];
	}

	SDL_SetColors(scr, col, first, n);
	free(col);

	if (flag)
		displayscreen();
}

// fade in gradually
void
fade_in()
{
	fadepalette(0, 255, bin_colors, -1, 1);
//	for (int i = 1; i <= 64; i++)
//		fadepalette(0, 255, bin_colors, i, 1);

	displayscreen();
}

// fade out gradually
void
fade_out()
{
	fadepalette(0, 255, bin_colors, -1, 1);
//	for (int i = 64; i; i--)
//		fadepalette(0, 255, bin_colors, i, 1);

	clearscr();
	usleep(500000L);
}
