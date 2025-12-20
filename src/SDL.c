// SDL2 output for thrust
// input handled in input.c

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "graphics.h"
#include "thrust.h"

static const int X = 320;
static const int Y = 200;
int window_zoom = 1;
static int linear_filter = 0;
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static Uint8* framebuffer = NULL; /* indexed pixels */
static SDL_Color palette[256];
static Uint32 palette32[256];

static void update_palette_from_table(int first, int last, uint8_t* RGBtable)
{
    int n = last - first + 1;
    int i;

    for (i = 0; i < n; i++)
    {
        palette[first + i].r = RGBtable[3 * i + 0];
        palette[first + i].g = RGBtable[3 * i + 1];
        palette[first + i].b = RGBtable[3 * i + 2];
        palette[first + i].a = 0xff;
        palette32[first + i] = ((Uint32)palette[first + i].a << 24) |
                               ((Uint32)palette[first + i].r << 16) |
                               ((Uint32)palette[first + i].g << 8) | (Uint32)palette[first + i].b;
    }
}

static void ensure_buffers(void)
{
    if (framebuffer == NULL)
        framebuffer = (Uint8*)calloc(X * Y, 1);
}

char* graphicsname(void)
{
    static char name[] = "SDL2";
    return name;
}

void graphics_set_smooth(int enable)
{
    linear_filter = enable ? 1 : 0;
}

void graphics_preinit(void)
{
    /* Nothing needed for SDL2 */
}

int graphicsinit(int zoom)
{
    if (zoom < 1)
        zoom = 1;
    if (zoom > 6)
        zoom = 6;
    window_zoom = zoom;

    fprintf(stderr, "graphicsinit: zoom=%d smooth=%d\n", window_zoom, linear_filter);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("thrust", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              X * window_zoom, Y * window_zoom, SDL_WINDOW_SHOWN);
    fprintf(stderr, "SDL_CreateWindow returned %p\n", window);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return -1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, linear_filter ? "linear" : "nearest");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    fprintf(stderr, "SDL_CreateRenderer returned %p\n", renderer);
    if (!renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return -1;
    }
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer, X, Y);

    texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, X, Y);
    fprintf(stderr, "SDL_CreateTexture returned %p\n", texture);
    if (!texture)
    {
        fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        return -1;
    }

    ensure_buffers();
    update_palette_from_table(0, 255, bin_colors);

    return 0;
}

int graphicsclose(void)
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    texture = NULL;
    renderer = NULL;
    window = NULL;

    free(framebuffer);
    framebuffer = NULL;

    SDL_Quit();
    return 0;
}

void clearscr(void)
{
    ensure_buffers();
    memset(framebuffer, 0, X * Y);
    displayscreen();
}

static void blit_indexed_to_texture(void)
{
    void* pixels = NULL;
    int pitch = 0;
    int y;

    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0)
        return;

    Uint8* src = framebuffer;
    Uint8* dst = (Uint8*)pixels;
    for (y = 0; y < Y; y++)
    {
        Uint32* row = (Uint32*)(dst + y * pitch);
        int x;
        for (x = 0; x < X; x++)
            row[x] = palette32[*src++];
    }

    SDL_UnlockTexture(texture);
}

void displayscreen(void)
{
    ensure_buffers();
    blit_indexed_to_texture();
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void syncscreen(void)
{
    static Uint32 last = 0;
    Uint32 now = SDL_GetTicks();

    if (last != 0)
    {
        Uint32 elapsed = now - last;
        if (elapsed < 20)
            SDL_Delay(20 - elapsed);
    }
    last = SDL_GetTicks();

    displayscreen();
}

void putpixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= X || y < 0 || y >= Y)
        return;
    framebuffer[y * X + x] = color;
}

void putarea(uint8_t* source, int x, int y, int width, int height, int bytesperline, int destx,
             int desty)
{
    int j;
    int sx = x;
    int sy = y;
    int dx = destx;
    int dy = desty;
    int w = width;
    int h = height;

    /* Clip against destination */
    if (dx < 0)
    {
        sx -= dx;
        w += dx;
        dx = 0;
    }
    if (dy < 0)
    {
        sy -= dy;
        h += dy;
        dy = 0;
    }
    if (dx + w > X)
        w = X - dx;
    if (dy + h > Y)
        h = Y - dy;

    if (w <= 0 || h <= 0)
        return;

    for (j = 0; j < h; j++)
    {
        uint8_t* dstrow = framebuffer + (dy + j) * X + dx;
        uint8_t* srcrow = source + (sy + j) * bytesperline + sx;
        memcpy(dstrow, srcrow, (size_t)w);
    }
}

void fadepalette(int first, int last, uint8_t* RGBtable, int fade, int flag)
{
    (void)fade; /* fade ignored */
    update_palette_from_table(first, last, RGBtable + first * 3);
    if (flag)
        displayscreen();
}

void fade_in(void)
{
    fadepalette(0, 255, bin_colors, -1, 1);
    displayscreen();
}

void fade_out(void)
{
    fadepalette(0, 255, bin_colors, -1, 1);
    clearscr();
    SDL_Delay(500);
}
