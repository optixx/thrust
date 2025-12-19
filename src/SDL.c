// SDL2 output for thrust
// input handled in SDLkey.c

#include <assert.h>
#include <stdlib.h>
#include <getopt.h>

#include <SDL.h>

#include "thrust.h"
#include "graphics.h"
#include "options.h"

static const int X = 320;
static const int Y = 200;
static int double_size = 0;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static Uint8 *framebuffer = NULL;   /* indexed pixels */
static Uint32 *rgba_buffer = NULL;  /* converted for texture upload */
static SDL_Color palette[256];

static void
update_palette_from_table(int first, int last, byte *RGBtable)
{
  int n = last - first + 1;
  int i;

  for(i = 0; i < n; i++) {
    palette[first + i].r = RGBtable[3 * i + 0];
    palette[first + i].g = RGBtable[3 * i + 1];
    palette[first + i].b = RGBtable[3 * i + 2];
    palette[first + i].a = 0xff;
  }
}

static void
ensure_buffers(void)
{
  if(framebuffer == NULL)
    framebuffer = (Uint8 *)calloc(X * Y, 1);
  if(rgba_buffer == NULL)
    rgba_buffer = (Uint32 *)calloc(X * Y, sizeof(Uint32));
}

char *
graphicsname(void)
{
  static char name[] = "SDL2";
  return name;
}

void
graphics_preinit(void)
{
  /* Nothing needed for SDL2 */
}

int
graphicsinit(int argc, char **argv)
{
  int optc;

  optind = 0; /* reset getopt parser */
  do {
    static struct option longopts[] = {
      OPTS,
      SDL_OPTS,
      { 0, 0, 0, 0 }
    };

    optc = getopt_long_only(argc, argv, OPTC SDL_OPTC, longopts, (int *) 0);
    if(optc == '2')
      double_size = 1;
  } while(optc != EOF);

  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return -1;
  }

  window = SDL_CreateWindow("thrust",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            X * (double_size ? 2 : 1),
                            Y * (double_size ? 2 : 1),
                            SDL_WINDOW_SHOWN);
  if(!window) {
    fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
    return -1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
  if(!renderer) {
    fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
    return -1;
  }
  SDL_RenderSetLogicalSize(renderer, X, Y);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, X, Y);
  if(!texture) {
    fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
    return -1;
  }

  ensure_buffers();
  update_palette_from_table(0, 255, bin_colors);

  return 0;
}

int
graphicsclose(void)
{
  if(texture)
    SDL_DestroyTexture(texture);
  if(renderer)
    SDL_DestroyRenderer(renderer);
  if(window)
    SDL_DestroyWindow(window);
  texture = NULL;
  renderer = NULL;
  window = NULL;

  free(framebuffer);
  free(rgba_buffer);
  framebuffer = NULL;
  rgba_buffer = NULL;

  SDL_Quit();
  return 0;
}

void
clearscr(void)
{
  ensure_buffers();
  memset(framebuffer, 0, X * Y);
  displayscreen();
}

static void
blit_indexed_to_texture(void)
{
  int i;
  size_t total = (size_t)X * Y;

  for(i = 0; i < (int)total; i++) {
    SDL_Color c = palette[framebuffer[i]];
    rgba_buffer[i] = ((Uint32)c.a << 24) | ((Uint32)c.r << 16) |
                     ((Uint32)c.g << 8) | (Uint32)c.b;
  }

  SDL_UpdateTexture(texture, NULL, rgba_buffer, X * (int)sizeof(Uint32));
}

void
displayscreen(void)
{
  ensure_buffers();
  blit_indexed_to_texture();
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void
syncscreen(void)
{
  static Uint32 last = 0;
  Uint32 now = SDL_GetTicks();

  if(last != 0) {
    Uint32 elapsed = now - last;
    if(elapsed < 20)
      SDL_Delay(20 - elapsed);
  }
  last = SDL_GetTicks();

  displayscreen();
}

static void
_putpixel(int x, int y, byte color)
{
  if(x < 0 || x >= X || y < 0 || y >= Y)
    return;
  framebuffer[y * X + x] = color;
}

void
putpixel(int x, int y, byte color)
{
  _putpixel(x, y, color);
}

void
putarea(byte *source, int x, int y, int width, int height, int bytesperline, int destx, int desty)
{
  int j, i;

  for(j = 0; j < height; j++) {
    int dy = desty + j;
    if(dy < 0 || dy >= Y)
      continue;
    for(i = 0; i < width; i++) {
      int dx = destx + i;
      if(dx < 0 || dx >= X)
        continue;
      _putpixel(dx, dy, source[bytesperline * (y + j) + x + i]);
    }
  }
}

void
fadepalette(int first, int last, byte *RGBtable, int fade, int flag)
{
  (void)fade; /* fade ignored */
  update_palette_from_table(first, last, RGBtable + first * 3);
  if(flag)
    displayscreen();
}

void
fade_in(void)
{
  fadepalette(0, 255, bin_colors, -1, 1);
  displayscreen();
}

void
fade_out(void)
{
  fadepalette(0, 255, bin_colors, -1, 1);
  clearscr();
  SDL_Delay(500);
}
