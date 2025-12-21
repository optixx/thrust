
/* Written by Peter Ekberg, peda@lysator.liu.se */

#include <SDL.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font5x5.h"
#include "graphics.h"
#include "things.h"
#include "thrust.h"
#include "helpers.h"
#include "assets.h"
#include "world.h"
#include "state.h"
#include "level.h"

int insideblock(int blkx, int blky, int pblkx, int pblky, int sx, int sy)
{
    return ((blkx >= pblkx - sx) && (blkx < pblkx + BBILDX) && (blky >= pblky - sy) &&
            (blky < pblky + BBILDY));
}

int insidepixel(int x, int y, int pixx, int pixy, int sx, int sy)
{
    return ((x > pixx - sx) && (x < pixx + PUSEX) && (y > pixy - sy) && (y < pixy + PUSEY));
}

void updateborder(int pblkx, int pblky, int bblkx, int bblky, int vx, int vy)
{
    uint32_t k;
    uint8_t* blocks = assets_blocks();

    if (vy <= 0) /* update bottom border */
        for (k = 0; k < BBILDX; k++)
            putblock(bblkx + k, (bblky + BBILDY - 1) % BBILDY,
                     blocks + (*(level_buffer() + (pblkx + k) % world_state()->lenx + ((pblky + BBILDY - 1) % world_state()->leny) * world_state()->lenx)
                               << 6));
    else /* update top border */
        for (k = 0; k < BBILDX; k++)
            putblock(bblkx + k, bblky,
                     blocks + (*(level_buffer() + (pblkx + k) % world_state()->lenx + (pblky % world_state()->leny) * world_state()->lenx) << 6));
    if (vx > 0) /* update right border */
        for (k = 0; k < BBILDY; k++)
            putblock(bblkx + BBILDX - 1, (bblky + k) % BBILDY,
                     blocks + (*(level_buffer() + (pblkx + BBILDX - 1) % world_state()->lenx + ((pblky + k) % world_state()->leny) * world_state()->lenx)
                               << 6));
    else /* update left border */
        for (k = 0; k < BBILDY; k++)
            putblock(bblkx, (bblky + k) % BBILDY,
                     blocks + (*(level_buffer() + pblkx % world_state()->lenx + ((pblky + k) % world_state()->leny) * world_state()->lenx) << 6));
}

uint8_t* bild;
static uint8_t fuelblink;
static int skip_frames = 0;


void putscr(int x, int y, int force_draw)
{
    static int counter = 0;

    counter++;
    if (!skip_frames || force_draw || (skip_frames && counter == 3))
    {
        counter = 0;
        if (PUSEY <= PBILDY - y)
            putarea(bild, x, y, PUSEX, PUSEY, PBILDX << 1, 0, 24);
        else
        {
            putarea(bild, x, y, PUSEX, PBILDY - y, PBILDX << 1, 0, 24);
            putarea(bild, x, 0, PUSEX, PUSEY - PBILDY + y, PBILDX << 1, 0, PBILDY + 24 - y);
        }
    }
}

void putblock(int x, int y, uint8_t* source)
{
    int i;
    uint8_t *dest1, *dest2;

    dest1 = bild + ((y << 3) * (PBILDX << 1)) + (x << 3);
    dest2 = dest1 + ((x >= BBILDX) ? -(PBILDX) : (PBILDX));

    for (i = 0; i < 8; i++)
    {
        memcpy(dest1, source, 8);
        memcpy(dest2, source, 8);
        source += 8;
        dest1 += PBILDX << 1;
        dest2 += PBILDX << 1;
    }
}

void drawfuel(int fuel)
{
    char str[16];
    uint8_t tmpcol, tmppap, tmpflg;

    tmpcol = chcolor;
    tmppap = chpaper;
    tmpflg = chflag;
    chpaper = 0;
    fuelblink = (fuelblink - 1) & 31;
    if (((fuel + 9) / 10) > 10 || fuelblink & 16)
        chcolor = 0x88;
    else
        chcolor = FUELCOLOR;
    chflag = 1;
    sprintf(str, "  %d", fuel);
    printgs(80 - gstrlen(str), 12, str);
    chflag = tmpflg;
    chpaper = tmppap;
    chcolor = tmpcol;
}

void drawship(uint32_t bx, uint32_t by, uint8_t* ship, uint8_t* storage)
{
    uint8_t *maxtmp, *tmp, pix;
    int i, j;

    maxtmp = bild + ((PBILDY - 1) * PBILDX << 1);
    tmp = bild + by * (PBILDX << 1) + bx;
    for (i = 0; i < 256; i += 16)
    {
        memcpy(storage + i, tmp, 16);
        for (j = 0; j < 16; j++)
        {
            pix = *(ship++);
            if (pix)
                *(tmp + j) = pix;
        }
        if (tmp >= maxtmp)
            tmp = bild + bx;
        else
            tmp += PBILDX << 1;
    }
}

void undrawship(uint32_t bx, uint32_t by, uint8_t* storage)
{
    uint8_t *maxtmp, *tmp;
    int i;

    maxtmp = bild + ((PBILDY - 1) * PBILDX << 1);
    tmp = bild + by * (PBILDX << 1) + bx;
    for (i = 0; i < 256; i += 16)
    {
        memcpy(tmp, storage + i, 16);
        if (tmp >= maxtmp)
            tmp = bild + bx;
        else
            tmp += PBILDX << 1;
    }
}

void drawsquare(uint32_t bx, uint32_t by, uint8_t* object, uint8_t* storage, uint8_t deltax,
                uint8_t deltay)
{
    uint8_t *maxtmp, *tmp, pix;
    uint32_t i, j;
    uint32_t deltaxy;

    deltaxy = (uint32_t)deltax * deltay;
    maxtmp = bild + ((PBILDY - 1) * PBILDX << 1);
    tmp = bild + by * (PBILDX << 1) + bx;
    for (i = 0; i < deltaxy; i += (int)deltax)
    {
        memcpy(storage + i, tmp, (int)deltax);
        for (j = 0; j < (int)deltax; j++)
        {
            pix = *(object++);
            if (pix)
                *(tmp + j) = pix;
        }
        if (tmp >= maxtmp)
            tmp = bild + bx;
        else
            tmp += PBILDX << 1;
    }
}

void undrawsquare(uint32_t bx, uint32_t by, uint8_t* storage, uint8_t deltax, uint8_t deltay)
{
    uint8_t *maxtmp, *tmp;
    uint32_t i;
    uint32_t deltaxy;

    deltaxy = (uint32_t)deltax * deltay;
    maxtmp = bild + ((PBILDY - 1) * PBILDX << 1);
    tmp = bild + by * (PBILDX << 1) + bx;
    for (i = 0; i < deltaxy; i += (int)deltax)
    {
        memcpy(tmp, storage + i, (int)deltax);
        if (tmp >= maxtmp)
            tmp = bild + bx;
        else
            tmp += PBILDX << 1;
    }
}

uint32_t testcrash(uint8_t* object, uint8_t* storage, uint32_t len, uint8_t shield)
{
    uint32_t i;
    uint8_t res = 0;

    for (i = 0; i < len; i++)
    {
        if (*(object++))
        {
            if (*storage > res && (!shield || (shield && *storage < 224)))
                res = *storage;
        }
        storage++;
    }
    return (((uint32_t)res) >> 5);
}

void writeblock(uint32_t bx, uint32_t by, uint8_t block)
{
    uint32_t tempx, tempy;
    uint8_t* blocks = assets_blocks();

    *(level_buffer() + bx + by * world_state()->lenx) = block;
    tempx = bx;
    tempy = by;
    if (world_state()->pblockx + BBILDX > (int)world_state()->lenx && tempx < BBILDX)
        tempx += world_state()->lenx;
    if (world_state()->pblocky + BBILDY > (int)world_state()->leny && tempy < BBILDY)
        tempy += world_state()->leny;
    if (insideblock(tempx, tempy, world_state()->pblockx, world_state()->pblocky, 0, 0))
        putblock(world_state()->bblockx - world_state()->pblockx + tempx, tempy % BBILDY, blocks + (block << 6));
}

#define NR_TP (6)
#define SZ_TP (3)
#define DIFF_TP (8)
#define SHIFT_TP (10)

void drawteleline(int round, int x1, int y1, int x2, int y2, int j, int k)
{
    int l;
    static uint8_t telemem[2 * 5 * 4 * NR_TP * SZ_TP];
    static uint8_t* tm;

    switch (round)
    {
    case 0:
        tm = &telemem[0];
        break;
    case 1:
        for (l = -2; l <= 2; l++)
        {
            putpixel(x1 + (j + 2) * DIFF_TP + k - SHIFT_TP, 24 + y1 + l, 65);
            putpixel(x1 - (j + 2) * DIFF_TP - k + SHIFT_TP, 24 + y1 + l, 65);
            putpixel(x1 + l, 24 + y1 + (j + 2) * DIFF_TP + k - SHIFT_TP, 65);
            putpixel(x1 + l, 24 + y1 - (j + 2) * DIFF_TP - k + SHIFT_TP, 65);
            if (world_state()->loaded)
            {
                putpixel(x2 + (j + 2) * DIFF_TP + k - SHIFT_TP, 24 + y2 + l, 65);
                putpixel(x2 - (j + 2) * DIFF_TP - k + SHIFT_TP, 24 + y2 + l, 65);
                putpixel(x2 + l, 24 + y2 + (j + 2) * DIFF_TP + k - SHIFT_TP, 65);
                putpixel(x2 + l, 24 + y2 - (j + 2) * DIFF_TP - k + SHIFT_TP, 65);
            }
        }
        break;
    case 2:
        for (l = -2; l <= 2; l++)
        {
            *(tm++) = *(bild + (world_state()->bildx + x1 + (j + 2) * DIFF_TP + k - SHIFT_TP) +
                        ((world_state()->bildy + y1 + l) % PBILDY) * PBILDX * 2);
            *(tm++) = *(bild + (world_state()->bildx + x1 - (j + 2) * DIFF_TP - k + SHIFT_TP) +
                        ((world_state()->bildy + y1 + l) % PBILDY) * PBILDX * 2);
            *(tm++) = *(bild + (world_state()->bildx + x1 + l) +
                        ((world_state()->bildy + y1 + (j + 2) * DIFF_TP + k - SHIFT_TP) % PBILDY) * PBILDX * 2);
            *(tm++) = *(bild + (world_state()->bildx + x1 + l) +
                        ((world_state()->bildy + y1 - (j + 2) * DIFF_TP - k + SHIFT_TP) % PBILDY) * PBILDX * 2);
            if (world_state()->loaded)
            {
                *(tm++) = *(bild + (world_state()->bildx + x2 + (j + 2) * DIFF_TP + k - SHIFT_TP) +
                            ((world_state()->bildy + y2 + l) % PBILDY) * PBILDX * 2);
                *(tm++) = *(bild + (world_state()->bildx + x2 - (j + 2) * DIFF_TP - k + SHIFT_TP) +
                            ((world_state()->bildy + y2 + l) % PBILDY) * PBILDX * 2);
                *(tm++) =
                    *(bild + (world_state()->bildx + x2 + l) +
                      ((world_state()->bildy + y2 + (j + 2) * DIFF_TP + k - SHIFT_TP) % PBILDY) * PBILDX * 2);
                *(tm++) =
                    *(bild + (world_state()->bildx + x2 + l) +
                      ((world_state()->bildy + y2 - (j + 2) * DIFF_TP - k + SHIFT_TP) % PBILDY) * PBILDX * 2);
            }
        }
        break;
    case 3:
        for (l = -2; l <= 2; l++)
        {
            putpixel(x1 + (j + 2) * DIFF_TP + k - SHIFT_TP, 24 + y1 + l, *(tm++));
            putpixel(x1 - (j + 2) * DIFF_TP - k + SHIFT_TP, 24 + y1 + l, *(tm++));
            putpixel(x1 + l, 24 + y1 + (j + 2) * DIFF_TP + k - SHIFT_TP, *(tm++));
            putpixel(x1 + l, 24 + y1 - (j + 2) * DIFF_TP - k + SHIFT_TP, *(tm++));
            if (world_state()->loaded)
            {
                putpixel(x2 + (j + 2) * DIFF_TP + k - SHIFT_TP, 24 + y2 + l, *(tm++));
                putpixel(x2 - (j + 2) * DIFF_TP - k + SHIFT_TP, 24 + y2 + l, *(tm++));
                putpixel(x2 + l, 24 + y2 + (j + 2) * DIFF_TP + k - SHIFT_TP, *(tm++));
                putpixel(x2 + l, 24 + y2 - (j + 2) * DIFF_TP - k + SHIFT_TP, *(tm++));
            }
        }
        break;
    }
}

void drawteleport(int tohere)
{
    int i, j, k;
    int x1, y1, x2, y2;

    x1 = x2 = 154 + 7 + world_state()->shipdx;
    y1 = y2 = 82 + 7 + world_state()->shipdy;
    if (world_state()->loaded)
    {
        x2 = 161 - (int)((252 - world_state()->loadpoint) * cos(world_state()->alpha) / 7.875);
        y2 = 89 + (int)((252 - world_state()->loadpoint) * sin(world_state()->alpha) / 7.875);
    }

    syncscreen();
    for (i = 0; i < NR_TP + SZ_TP - 1; i++)
    {
        for (k = min_int(SZ_TP - 1, i), j = max_int(i - (SZ_TP - 1), 0); j <= min_int(i, NR_TP - 1); k--, j++)
            drawteleline(1, x1, y1, x2, y2, j, k);
        SDL_Delay(50);
        displayscreen();
    }

    SDL_Delay(250);
    syncscreen();
    if (tohere)
        drawshuttle();
    putscr(world_state()->bildx, world_state()->bildy, 1);

    for (i = 0; i < NR_TP + SZ_TP - 1; i++)
        for (k = min_int(SZ_TP - 1, i), j = max_int(i - (SZ_TP - 1), 0); j <= min_int(i, NR_TP - 1); k--, j++)
            drawteleline(1, x1, y1, x2, y2, j, k);
    displayscreen();
    drawteleline(0, 0, 0, 0, 0, 0, 0);
    for (i = 0; i < NR_TP + SZ_TP - 1; i++)
    for (k = min_int(SZ_TP - 1, i), j = max_int(i - (SZ_TP - 1), 0); j <= min_int(i, NR_TP - 1); k--, j++)
            drawteleline(2, x1, y1, x2, y2, j, k);

    SDL_Delay(250);

    syncscreen();
    drawteleline(0, 0, 0, 0, 0, 0, 0);
    for (i = 0; i < NR_TP + SZ_TP - 1; i++)
    {
        for (k = min_int(SZ_TP - 1, i), j = max_int(i - (SZ_TP - 1), 0); j <= min_int(i, NR_TP - 1); k--, j++)
            drawteleline(3, x1, y1, x2, y2, j, k);
        SDL_Delay(50);
        displayscreen();
    }
    if (tohere)
        undrawshuttle();
}

void swap(int* pa, int* pb)
{
    int t;
    t = *pa;
    *pa = *pb;
    *pb = t;
}

void drawlinev(int x1, int y1, int x2, int y2, uint8_t color, uint8_t* storage)
{
    int d, dx, dy;
    int Ai, Bi, xi;
    uint8_t* ptr = bild;

    if (y1 > y2)
    {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    xi = (x2 > x1) ? 1 : -1;
    dx = abs(x2 - x1);
    dy = (y2 - y1);
    Ai = (dx - dy) << 1;
    Bi = (dx << 1);
    d = Bi - dy;

    ptr += y1 * (PBILDX << 1);
    *(storage++) = *(ptr + x1);
    *(ptr + x1) = color;
    for (y1++, ptr += PBILDX << 1; y1 <= y2; y1++, ptr += PBILDX << 1)
    {
        if (y1 == PBILDY)
            ptr = bild;
        if (d < 0)
            d += Bi;
        else
        {
            x1 += xi;
            d += Ai;
        }
        *(storage++) = *(ptr + x1);
        *(ptr + x1) = color;
    }
}

void undrawlinev(int x1, int y1, int x2, int y2, uint8_t* storage)
{
    int d, dx, dy;
    int Ai, Bi, xi;
    uint8_t* ptr = bild;

    if (y1 > y2)
    {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    xi = (x2 > x1) ? 1 : -1;
    dx = abs(x2 - x1);
    dy = (y2 - y1);
    Ai = (dx - dy) << 1;
    Bi = (dx << 1);
    d = Bi - dy;

    ptr += y1 * (PBILDX << 1);
    *(ptr + x1) = *(storage++);
    for (y1++, ptr += PBILDX << 1; y1 <= y2; y1++, ptr += PBILDX << 1)
    {
        if (y1 == PBILDY)
            ptr = bild;
        if (d < 0)
            d += Bi;
        else
        {
            x1 += xi;
            d += Ai;
        }
        *(ptr + x1) = *(storage++);
    }
}

void drawlineh(int x1, int y1, int x2, int y2, uint8_t color, uint8_t* storage)
{
    int d, dx, dy;
    int Ai, Bi, yi, i;
    uint8_t* ptr = bild;

    if (x1 > x2)
    {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    if (y2 > y1)
    {
        yi = PBILDX << 1;
        i = 1;
    }
    else
    {
        yi = -(PBILDX << 1);
        i = -1;
    }
    dx = x2 - x1;
    dy = abs(y2 - y1);
    Ai = (dy - dx) << 1;
    Bi = (dy << 1);
    d = Bi - dx;
    if (y1 >= PBILDY)
        y1 -= PBILDY;
    y2 = y1;
    y1 = y1 * PBILDX << 1;

    ptr += y1;
    *(storage++) = *(ptr + x1);
    *(ptr + x1) = color;
    for (x1++; x1 <= x2; x1++)
    {
        if (d < 0)
            d += Bi;
        else
        {
            ptr += yi;
            y2 += i;
            if (y2 == -1)
                ptr += PBILDY * PBILDX << 1;
            if (y2 == PBILDY)
                ptr = bild;
            d += Ai;
        }
        *(storage++) = *(ptr + x1);
        *(ptr + x1) = color;
    }
}

void undrawlineh(int x1, int y1, int x2, int y2, uint8_t* storage)
{
    int d, dx, dy;
    int Ai, Bi, yi, i;
    uint8_t* ptr = bild;

    if (x1 > x2)
    {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    if (y2 > y1)
    {
        yi = PBILDX << 1;
        i = 1;
    }
    else
    {
        yi = -(PBILDX << 1);
        i = -1;
    }
    dx = x2 - x1;
    dy = abs(y2 - y1);
    Ai = (dy - dx) << 1;
    Bi = (dy << 1);
    d = Bi - dx;
    if (y1 >= PBILDY)
        y1 -= PBILDY;
    y2 = y1;
    y1 = y1 * PBILDX << 1;

    ptr += y1;
    *(ptr + x1) = *(storage++);
    for (x1++; x1 <= x2; x1++)
    {
        if (d < 0)
            d += Bi;
        else
        {
            ptr += yi;
            y2 += i;
            if (y2 == -1)
                ptr += PBILDY * PBILDX << 1;
            if (y2 == PBILDY)
                ptr = bild;
            d += Ai;
        }
        *(ptr + x1) = *(storage++);
    }
}

void drawline(int x1, int y1, int x2, int y2, uint8_t color, uint8_t* storage)
{
    if (y1 > y2 + 64)
        y2 += PBILDY;
    if (y2 > y1 + 64)
        y1 += PBILDY;
    if (abs(x1 - x2) < abs(y1 - y2))
        drawlinev(x1, y1, x2, y2, color, storage);
    else
        drawlineh(x1, y1, x2, y2, color, storage);
}

void undrawline(int x1, int y1, int x2, int y2, uint8_t* storage)
{
    if (y1 > y2 + 64)
        y2 += PBILDY;
    if (y2 > y1 + 64)
        y1 += PBILDY;
    if (abs(x1 - x2) < abs(y1 - y2))
        undrawlinev(x1, y1, x2, y2, storage);
    else
        undrawlineh(x1, y1, x2, y2, storage);
}

void drawbullets(void)
{
    int l;
    bullet* bulletptr;
    uint32_t tempx, tempy;
    uint8_t target;
    uint8_t* bulletmap = assets_bulletmap();
    uint8_t* bulletstorage = assets_bulletstorage();

    for (l = 0, bulletptr = bullets; l < maxbullets; l++, bulletptr++)
        if ((*bulletptr).life)
        {
            tempx = (*bulletptr).x >> 3;
            tempy = (*bulletptr).y >> 3;
            if (world_state()->pixx + PUSEX > (int)world_state()->lenx3 && tempx < PUSEX)
                tempx += world_state()->lenx3;
            if (world_state()->pixy + PUSEY > (int)world_state()->leny3 && tempy < PUSEY)
                tempy += world_state()->leny3;
            if (insidepixel(tempx, tempy, world_state()->pixx, world_state()->pixy, 4, 4))
                drawsquare(world_state()->bildx + tempx - world_state()->pixx, tempy % PBILDY,
                           bulletmap + ((*bulletptr).dir << 4), bulletstorage + (l << 4), 4, 4);
            else
            {
                target = *(level_buffer() + (tempx >> 3) % world_state()->lenx + ((tempy >> 3) % world_state()->leny) * world_state()->lenx);
                if (target != 32)
                {
                    /* Add code to take care of offscreen hits */
                    if ((*bulletptr).owner)
                        switch (target)
                        {
                        case '`':
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                        case 'g':
                        case 'h':
                        case 'i':
                        case 'j':
                        case 'k':
                        case 'l':
                        case 'L':
                        case 'M':
                        case 'N':
                        case 'O':
                        case 'P':
                        case 'Q':
                        case 'R':
                        case 'S':
                        case 'T':
                        case 'U':
                        case 'V':
                        case 'W':
                        case 'X':
                        case 'Y':
                        case 'Z':
                        case ']':
                        case '\\':
                        case '^':
                        case '_':
                            hit((tempx + 3) % world_state()->lenx3, (tempy + 3) % world_state()->leny3, 4, (*bulletptr).owner);
                        }
                    (*bulletptr).life = 0;
                }
            }
        }
}

void undrawbullets(void)
{
    int l;
    bullet* bulletptr;
    uint32_t tempx, tempy;
    uint32_t crash;
    uint8_t* bulletmap = assets_bulletmap();
    uint8_t* bulletstorage = assets_bulletstorage();

    for (l = maxbullets - 1, bulletptr = bullets + maxbullets - 1; l >= 0; l--, bulletptr--)
        if ((*bulletptr).life)
        {
            tempx = (*bulletptr).x >> 3;
            tempy = (*bulletptr).y >> 3;
            if (world_state()->pixx + PUSEX > (int)world_state()->lenx3 && tempx < PUSEX)
                tempx += world_state()->lenx3;
            if (world_state()->pixy + PUSEY > (int)world_state()->leny3 && tempy < PUSEY)
                tempy += world_state()->leny3;
            if (insidepixel(tempx, tempy, world_state()->pixx, world_state()->pixy, 4, 4))
            {
                crash =
                    testcrash(bulletmap + ((*bulletptr).dir << 4), bulletstorage + (l << 4), 16, 0);
                if (crash)
                {
                    if (crash >= 4)
                        hit((tempx + 3) % world_state()->lenx3, (tempy + 3) % world_state()->leny3, crash, (*bulletptr).owner);
                    (*bulletptr).life = 0;
                }
                undrawsquare(world_state()->bildx + tempx - world_state()->pixx, tempy % PBILDY, bulletstorage + (l << 4), 4, 4);
            }
        }
}

void drawfragments(void)
{
    int l;
    fragment* fragmentptr;
    uint32_t tempx, tempy;
    static uint8_t fragmentmap[4] = {12, 12, 12, 12};
    uint8_t* fragmentstorage = assets_fragmentstorage();

    for (l = 0, fragmentptr = fragments; l < maxfragments; l++, fragmentptr++)
        if ((*fragmentptr).life)
        {
            tempx = (*fragmentptr).x >> 3;
            tempy = (*fragmentptr).y >> 3;
            if (world_state()->pixx + PUSEX > (int)world_state()->lenx3 && tempx < PUSEX)
                tempx += world_state()->lenx3;
            if (world_state()->pixy + PUSEY > (int)world_state()->leny3 && tempy < PUSEY)
                tempy += world_state()->leny3;
            if (insidepixel(tempx, tempy, world_state()->pixx, world_state()->pixy, 2, 2))
                drawsquare(world_state()->bildx + tempx - world_state()->pixx, tempy % PBILDY, fragmentmap,
                           fragmentstorage + (l << 2), 2, 2);
            else if (*(level_buffer() + (tempx >> 3) % world_state()->lenx + ((tempy >> 3) % world_state()->leny) * world_state()->lenx) != 32)
                (*fragmentptr).life = 0;
        }
}

void undrawfragments(void)
{
    int l;
    fragment* fragmentptr;
    uint32_t tempx, tempy;
    uint32_t crash;
    static uint8_t fragmentmap[4] = {12, 12, 12, 12};
    uint8_t* fragmentstorage = assets_fragmentstorage();

    for (l = maxfragments - 1, fragmentptr = fragments + maxfragments - 1; l >= 0;
         l--, fragmentptr--)
        if ((*fragmentptr).life)
        {
            tempx = (*fragmentptr).x >> 3;
            tempy = (*fragmentptr).y >> 3;
            if (world_state()->pixx + PUSEX > (int)world_state()->lenx3 && tempx < PUSEX)
                tempx += world_state()->lenx3;
            if (world_state()->pixy + PUSEY > (int)world_state()->leny3 && tempy < PUSEY)
                tempy += world_state()->leny3;
            if (insidepixel(tempx, tempy, world_state()->pixx, world_state()->pixy, 2, 2))
            {
                crash = testcrash(fragmentmap, fragmentstorage + (l << 2), 4, 0);
                if (crash)
                {
                    (*fragmentptr).life = 0;
                }
                undrawsquare(world_state()->bildx + tempx - world_state()->pixx, tempy % PBILDY, fragmentstorage + (l << 2), 2,
                             2);
            }
        }
}

void drawpowerplantblip(void)
{
    uint32_t tempx, tempy;
    uint8_t* blocks = assets_blocks();

    tempx = ppx;
    tempy = ppy;
    if (world_state()->pblockx + BBILDX > (int)world_state()->lenx && tempx < BBILDX)
        tempx += world_state()->lenx;
    if (world_state()->pblocky + BBILDY > (int)world_state()->leny && tempy < BBILDY)
        tempy += world_state()->leny;
    if (insideblock(tempx, tempy, world_state()->pblockx, world_state()->pblocky, 0, 0))
        putblock(world_state()->bblockx - world_state()->pblockx + tempx, tempy % BBILDY,
                 blocks + ((ppblip ? 32 : 222 - (ppcount & 0xc)) << 6));
}

void drawload(int flag)
{
    uint32_t tempx, tempy;
    uint8_t* blocks = assets_blocks();

    tempx = world_state()->loadbx;
    tempy = world_state()->loadby;
    if (world_state()->pblockx + BBILDX > (int)world_state()->lenx && tempx < BBILDX)
        tempx += world_state()->lenx;
    if (world_state()->pblocky + BBILDY > (int)world_state()->leny && tempy < BBILDY)
        tempy += world_state()->leny;
    putblock(world_state()->bblockx - world_state()->pblockx + tempx, tempy % BBILDY, blocks + ((flag ? 109 : 32) << 6));
}

uint32_t drawshuttle(void)
{
    uint32_t crash = 0, tmp;
#ifdef DEBUG2
    int debug2i;
#endif
    int x1, x2 = 0, y1, y2 = 0, lx, ly;
    static uint8_t wiremap[64] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                                  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                                  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
                                  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13};
    game_state_t* state = state_current();
    uint8_t shield_state = state ? state->shield : 0;
    uint8_t* ship = assets_ship();
    uint8_t* shieldship = assets_shieldship();
    uint8_t* shipstorage = assets_shipstorage();
    uint8_t* loadmap = assets_loadmap();
    uint8_t* loadstorage = assets_loadstorage();
    uint8_t* wirestorage = assets_wirestorage();

    if (world_state()->loaded || world_state()->loadcontact)
    {
        x1 = world_state()->bildx + 161 + world_state()->shipdx;
        y1 = (world_state()->bildy + 89 + world_state()->shipdy);
        if (world_state()->loaded)
        {
            x2 = world_state()->bildx + 161 - (int)((252 - world_state()->loadpoint) * cos(world_state()->alpha) / 7.875);
            y2 = world_state()->bildy + 89 + (int)((252 - world_state()->loadpoint) * sin(world_state()->alpha) / 7.875);
        }
        else
        {
            x2 = (world_state()->loadbx << 3) + 3;
            if (abs(x2 - x1) > PBILDX / 2)
                x2 += PBILDX;
            y2 = (world_state()->loadby << 3) + 3;
        }
        lx = abs(x1 - x2) % PBILDX;
        ly = abs(y1 - y2) % PBILDY;
        if (lx > 64)
            lx = abs(lx - PBILDX);
        if (ly > 64)
            ly = abs(ly - PBILDY);
        drawline(x1, y1 % PBILDY, x2, y2 % PBILDY, 11, wirestorage);
        tmp = testcrash(wiremap, wirestorage, max_int(lx, ly) + 1, shield_state);
#ifdef DEBUG2
        if (tmp)
        {
            printf("Crash: Wire destroyed. By %d. Wirelength %d.\n", tmp, max_int(lx, ly) + 1);
            printf("Wirestorage:");
            for (debug2i = 0; debug2i < max_int(lx, ly) + 1; debug2i++)
                printf(" %02x", *(wirestorage + debug2i));
            printf("\n");
            printf("Killer line: x1=%d, y1=%d, x2=%d, y2=%d\n", x1, y1 % PBILDY, x2, y2 % PBILDY);
            undrawline(x1 + 2, y1 % PBILDY, x2 + 2, y2 % PBILDY, wirestorage);
            SDL_Delay(10000);
        }
#endif
        crash = max_int(crash, tmp);
    }
    /* Draw the shuttle */
    drawship(world_state()->bildx + 154 + world_state()->shipdx, (world_state()->bildy + 82 + world_state()->shipdy) % PBILDY,
             (shield_state ? shieldship : ship) + (world_state()->dir << 8), shipstorage);
    tmp = testcrash(ship + (world_state()->dir << 8), shipstorage, 256, shield_state);
#ifdef DEBUG2
    if (tmp)
        printf("Crash: Ship destroyed. By %d.\n", tmp);
#endif
    crash = max_int(tmp, crash);
    if (world_state()->loaded || world_state()->loadcontact)
    {
        if (world_state()->loaded)
            drawsquare(x2 - 3, (y2 - 3) % PBILDY, loadmap, loadstorage, 8, 8);
        else if (world_state()->loadcontact)
        {
            x1 = world_state()->loadbx << 3;
            y1 = world_state()->loadby << 3;
            if (world_state()->pixx + PBILDX > (int)world_state()->lenx3 && x1 < PBILDX)
                x1 += world_state()->lenx3;
            if (world_state()->pixy + PBILDY > (int)world_state()->leny3 && y1 < PBILDY)
                y1 += world_state()->leny3;
            drawsquare(world_state()->bildx - world_state()->pixx + x1, y1 % PBILDY, loadmap, loadstorage, 8, 8);
        }
        tmp = testcrash(loadmap, loadstorage, 64, shield_state);
#ifdef DEBUG2
        if (tmp)
            printf("Crash: Load destroyed. By %d.\n", tmp);
#endif
        crash = max_int(crash, tmp);
    }
    return (crash);
}

void undrawshuttle(void)
{
    int x1, x2 = 0, y1, y2 = 0;
    uint8_t* loadstorage = assets_loadstorage();
    uint8_t* shipstorage = assets_shipstorage();
    uint8_t* wirestorage = assets_wirestorage();

    if (world_state()->loaded)
    {
        x2 = world_state()->bildx + 161 - (int)((252 - world_state()->loadpoint) * cos(world_state()->alpha) / 7.875);
        y2 = world_state()->bildy + 89 + (int)((252 - world_state()->loadpoint) * sin(world_state()->alpha) / 7.875);
        undrawsquare(x2 - 3, (y2 - 3) % PBILDY, loadstorage, 8, 8);
    }
    else if (world_state()->loadcontact)
    {
        x1 = world_state()->loadbx << 3;
        y1 = world_state()->loadby << 3;
        if (world_state()->pixx + PBILDX > (int)world_state()->lenx3 && x1 < PBILDX)
            x1 += world_state()->lenx3;
        if (world_state()->pixy + PBILDY > (int)world_state()->leny3 && y1 < PBILDY)
            y1 += world_state()->leny3;
        undrawsquare(world_state()->bildx - world_state()->pixx + x1, y1 % PBILDY, loadstorage, 8, 8);
    }
    undrawship(world_state()->bildx + 154 + world_state()->shipdx, (world_state()->bildy + 82 + world_state()->shipdy) % PBILDY, shipstorage);
    if (world_state()->loaded || world_state()->loadcontact)
    {
        x1 = world_state()->bildx + 161 + world_state()->shipdx;
        y1 = (world_state()->bildy + 89 + world_state()->shipdy);
        if (world_state()->loadcontact)
        {
            x2 = (world_state()->loadbx << 3) + 3;
            if (abs(x2 - x1) > PBILDX / 2)
                x2 += PBILDX;
            y2 = (world_state()->loadby << 3) + 3;
        }
        undrawline(x1, y1 % PBILDY, x2, y2 % PBILDY, wirestorage);
    }
}

void drawfuellines(void)
{
    uint8_t* fuelmap = assets_fuelmap();
    uint8_t* fuelstorage = assets_fuelstorage();

    drawsquare(world_state()->bildx + world_state()->shipdx + 151, (world_state()->bildy + world_state()->shipdy + 98) % PBILDY, fuelmap, fuelstorage, 4, 32);
    drawsquare(world_state()->bildx + world_state()->shipdx + 168, (world_state()->bildy + world_state()->shipdy + 98) % PBILDY, fuelmap + 128,
               fuelstorage + 128, 4, 32);
}

void undrawfuellines(void)
{
    uint8_t* fuelstorage = assets_fuelstorage();

    undrawsquare(world_state()->bildx + world_state()->shipdx + 151, (world_state()->bildy + world_state()->shipdy + 98) % PBILDY, fuelstorage, 4, 32);
    undrawsquare(world_state()->bildx + world_state()->shipdx + 168, (world_state()->bildy + world_state()->shipdy + 98) % PBILDY, fuelstorage + 128, 4, 32);
}
