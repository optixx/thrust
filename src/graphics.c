
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

    if (vy <= 0) /* update bottom border */
        for (k = 0; k < BBILDX; k++)
            putblock(bblkx + k, (bblky + BBILDY - 1) % BBILDY,
                     blocks + (*(bana + (pblkx + k) % lenx + ((pblky + BBILDY - 1) % leny) * lenx)
                               << 6));
    else /* update top border */
        for (k = 0; k < BBILDX; k++)
            putblock(bblkx + k, bblky,
                     blocks + (*(bana + (pblkx + k) % lenx + (pblky % leny) * lenx) << 6));
    if (vx > 0) /* update right border */
        for (k = 0; k < BBILDY; k++)
            putblock(bblkx + BBILDX - 1, (bblky + k) % BBILDY,
                     blocks + (*(bana + (pblkx + BBILDX - 1) % lenx + ((pblky + k) % leny) * lenx)
                               << 6));
    else /* update left border */
        for (k = 0; k < BBILDY; k++)
            putblock(bblkx, (bblky + k) % BBILDY,
                     blocks + (*(bana + pblkx % lenx + ((pblky + k) % leny) * lenx) << 6));
}

uint8_t* bild;
static uint8_t fuelblink;


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

    *(bana + bx + by * lenx) = block;
    tempx = bx;
    tempy = by;
    if (pblockx + BBILDX > (int)lenx && tempx < BBILDX)
        tempx += lenx;
    if (pblocky + BBILDY > (int)leny && tempy < BBILDY)
        tempy += leny;
    if (insideblock(tempx, tempy, pblockx, pblocky, 0, 0))
        putblock(bblockx - pblockx + tempx, tempy % BBILDY, blocks + (block << 6));
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
            if (loaded)
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
            *(tm++) = *(bild + (bildx + x1 + (j + 2) * DIFF_TP + k - SHIFT_TP) +
                        ((bildy + y1 + l) % PBILDY) * PBILDX * 2);
            *(tm++) = *(bild + (bildx + x1 - (j + 2) * DIFF_TP - k + SHIFT_TP) +
                        ((bildy + y1 + l) % PBILDY) * PBILDX * 2);
            *(tm++) = *(bild + (bildx + x1 + l) +
                        ((bildy + y1 + (j + 2) * DIFF_TP + k - SHIFT_TP) % PBILDY) * PBILDX * 2);
            *(tm++) = *(bild + (bildx + x1 + l) +
                        ((bildy + y1 - (j + 2) * DIFF_TP - k + SHIFT_TP) % PBILDY) * PBILDX * 2);
            if (loaded)
            {
                *(tm++) = *(bild + (bildx + x2 + (j + 2) * DIFF_TP + k - SHIFT_TP) +
                            ((bildy + y2 + l) % PBILDY) * PBILDX * 2);
                *(tm++) = *(bild + (bildx + x2 - (j + 2) * DIFF_TP - k + SHIFT_TP) +
                            ((bildy + y2 + l) % PBILDY) * PBILDX * 2);
                *(tm++) =
                    *(bild + (bildx + x2 + l) +
                      ((bildy + y2 + (j + 2) * DIFF_TP + k - SHIFT_TP) % PBILDY) * PBILDX * 2);
                *(tm++) =
                    *(bild + (bildx + x2 + l) +
                      ((bildy + y2 - (j + 2) * DIFF_TP - k + SHIFT_TP) % PBILDY) * PBILDX * 2);
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
            if (loaded)
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

    x1 = x2 = 154 + 7 + shipdx;
    y1 = y2 = 82 + 7 + shipdy;
    if (loaded)
    {
        x2 = 161 - (int)((252 - loadpoint) * cos(alpha) / 7.875);
        y2 = 89 + (int)((252 - loadpoint) * sin(alpha) / 7.875);
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
    putscr(bildx, bildy, 1);

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

    for (l = 0, bulletptr = bullets; l < maxbullets; l++, bulletptr++)
        if ((*bulletptr).life)
        {
            tempx = (*bulletptr).x >> 3;
            tempy = (*bulletptr).y >> 3;
            if (pixx + PUSEX > (int)lenx3 && tempx < PUSEX)
                tempx += lenx3;
            if (pixy + PUSEY > (int)leny3 && tempy < PUSEY)
                tempy += leny3;
            if (insidepixel(tempx, tempy, pixx, pixy, 4, 4))
                drawsquare(bildx + tempx - pixx, tempy % PBILDY,
                           bulletmap + ((*bulletptr).dir << 4), bulletstorage + (l << 4), 4, 4);
            else
            {
                target = *(bana + (tempx >> 3) % lenx + ((tempy >> 3) % leny) * lenx);
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
                            hit((tempx + 3) % lenx3, (tempy + 3) % leny3, 4, (*bulletptr).owner);
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

    for (l = maxbullets - 1, bulletptr = bullets + maxbullets - 1; l >= 0; l--, bulletptr--)
        if ((*bulletptr).life)
        {
            tempx = (*bulletptr).x >> 3;
            tempy = (*bulletptr).y >> 3;
            if (pixx + PUSEX > (int)lenx3 && tempx < PUSEX)
                tempx += lenx3;
            if (pixy + PUSEY > (int)leny3 && tempy < PUSEY)
                tempy += leny3;
            if (insidepixel(tempx, tempy, pixx, pixy, 4, 4))
            {
                crash =
                    testcrash(bulletmap + ((*bulletptr).dir << 4), bulletstorage + (l << 4), 16, 0);
                if (crash)
                {
                    if (crash >= 4)
                        hit((tempx + 3) % lenx3, (tempy + 3) % leny3, crash, (*bulletptr).owner);
                    (*bulletptr).life = 0;
                }
                undrawsquare(bildx + tempx - pixx, tempy % PBILDY, bulletstorage + (l << 4), 4, 4);
            }
        }
}

void drawfragments(void)
{
    int l;
    fragment* fragmentptr;
    uint32_t tempx, tempy;
    static uint8_t fragmentmap[4] = {12, 12, 12, 12};

    for (l = 0, fragmentptr = fragments; l < maxfragments; l++, fragmentptr++)
        if ((*fragmentptr).life)
        {
            tempx = (*fragmentptr).x >> 3;
            tempy = (*fragmentptr).y >> 3;
            if (pixx + PUSEX > (int)lenx3 && tempx < PUSEX)
                tempx += lenx3;
            if (pixy + PUSEY > (int)leny3 && tempy < PUSEY)
                tempy += leny3;
            if (insidepixel(tempx, tempy, pixx, pixy, 2, 2))
                drawsquare(bildx + tempx - pixx, tempy % PBILDY, fragmentmap,
                           fragmentstorage + (l << 2), 2, 2);
            else if (*(bana + (tempx >> 3) % lenx + ((tempy >> 3) % leny) * lenx) != 32)
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

    for (l = maxfragments - 1, fragmentptr = fragments + maxfragments - 1; l >= 0;
         l--, fragmentptr--)
        if ((*fragmentptr).life)
        {
            tempx = (*fragmentptr).x >> 3;
            tempy = (*fragmentptr).y >> 3;
            if (pixx + PUSEX > (int)lenx3 && tempx < PUSEX)
                tempx += lenx3;
            if (pixy + PUSEY > (int)leny3 && tempy < PUSEY)
                tempy += leny3;
            if (insidepixel(tempx, tempy, pixx, pixy, 2, 2))
            {
                crash = testcrash(fragmentmap, fragmentstorage + (l << 2), 4, 0);
                if (crash)
                {
                    (*fragmentptr).life = 0;
                }
                undrawsquare(bildx + tempx - pixx, tempy % PBILDY, fragmentstorage + (l << 2), 2,
                             2);
            }
        }
}

void drawpowerplantblip(void)
{
    uint32_t tempx, tempy;

    tempx = ppx;
    tempy = ppy;
    if (pblockx + BBILDX > (int)lenx && tempx < BBILDX)
        tempx += lenx;
    if (pblocky + BBILDY > (int)leny && tempy < BBILDY)
        tempy += leny;
    if (insideblock(tempx, tempy, pblockx, pblocky, 0, 0))
        putblock(bblockx - pblockx + tempx, tempy % BBILDY,
                 blocks + ((ppblip ? 32 : 222 - (ppcount & 0xc)) << 6));
}

void drawload(int flag)
{
    uint32_t tempx, tempy;

    tempx = loadbx;
    tempy = loadby;
    if (pblockx + BBILDX > (int)lenx && tempx < BBILDX)
        tempx += lenx;
    if (pblocky + BBILDY > (int)leny && tempy < BBILDY)
        tempy += leny;
    putblock(bblockx - pblockx + tempx, tempy % BBILDY, blocks + ((flag ? 109 : 32) << 6));
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

    if (loaded || loadcontact)
    {
        x1 = bildx + 161 + shipdx;
        y1 = (bildy + 89 + shipdy);
        if (loaded)
        {
            x2 = bildx + 161 - (int)((252 - loadpoint) * cos(alpha) / 7.875);
            y2 = bildy + 89 + (int)((252 - loadpoint) * sin(alpha) / 7.875);
        }
        else
        {
            x2 = (loadbx << 3) + 3;
            if (abs(x2 - x1) > PBILDX / 2)
                x2 += PBILDX;
            y2 = (loadby << 3) + 3;
        }
        lx = abs(x1 - x2) % PBILDX;
        ly = abs(y1 - y2) % PBILDY;
        if (lx > 64)
            lx = abs(lx - PBILDX);
        if (ly > 64)
            ly = abs(ly - PBILDY);
        drawline(x1, y1 % PBILDY, x2, y2 % PBILDY, 11, wirestorage);
        tmp = testcrash(wiremap, wirestorage, max_int(lx, ly) + 1, shield);
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
    drawship(bildx + 154 + shipdx, (bildy + 82 + shipdy) % PBILDY,
             (shield ? shieldship : ship) + (dir << 8), shipstorage);
    tmp = testcrash(ship + (dir << 8), shipstorage, 256, shield);
#ifdef DEBUG2
    if (tmp)
        printf("Crash: Ship destroyed. By %d.\n", tmp);
#endif
    crash = max_int(tmp, crash);
    if (loaded || loadcontact)
    {
        if (loaded)
            drawsquare(x2 - 3, (y2 - 3) % PBILDY, loadmap, loadstorage, 8, 8);
        else if (loadcontact)
        {
            x1 = loadbx << 3;
            y1 = loadby << 3;
            if (pixx + PBILDX > (int)lenx3 && x1 < PBILDX)
                x1 += lenx3;
            if (pixy + PBILDY > (int)leny3 && y1 < PBILDY)
                y1 += leny3;
            drawsquare(bildx - pixx + x1, y1 % PBILDY, loadmap, loadstorage, 8, 8);
        }
        tmp = testcrash(loadmap, loadstorage, 64, shield);
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

    if (loaded)
    {
        x2 = bildx + 161 - (int)((252 - loadpoint) * cos(alpha) / 7.875);
        y2 = bildy + 89 + (int)((252 - loadpoint) * sin(alpha) / 7.875);
        undrawsquare(x2 - 3, (y2 - 3) % PBILDY, loadstorage, 8, 8);
    }
    else if (loadcontact)
    {
        x1 = loadbx << 3;
        y1 = loadby << 3;
        if (pixx + PBILDX > (int)lenx3 && x1 < PBILDX)
            x1 += lenx3;
        if (pixy + PBILDY > (int)leny3 && y1 < PBILDY)
            y1 += leny3;
        undrawsquare(bildx - pixx + x1, y1 % PBILDY, loadstorage, 8, 8);
    }
    undrawship(bildx + 154 + shipdx, (bildy + 82 + shipdy) % PBILDY, shipstorage);
    if (loaded || loadcontact)
    {
        x1 = bildx + 161 + shipdx;
        y1 = (bildy + 89 + shipdy);
        if (loadcontact)
        {
            x2 = (loadbx << 3) + 3;
            if (abs(x2 - x1) > PBILDX / 2)
                x2 += PBILDX;
            y2 = (loadby << 3) + 3;
        }
        undrawline(x1, y1 % PBILDY, x2, y2 % PBILDY, wirestorage);
    }
}

void drawfuellines(void)
{
    drawsquare(bildx + shipdx + 151, (bildy + shipdy + 98) % PBILDY, fuelmap, fuelstorage, 4, 32);
    drawsquare(bildx + shipdx + 168, (bildy + shipdy + 98) % PBILDY, fuelmap + 128,
               fuelstorage + 128, 4, 32);
}

void undrawfuellines(void)
{
    undrawsquare(bildx + shipdx + 151, (bildy + shipdy + 98) % PBILDY, fuelstorage, 4, 32);
    undrawsquare(bildx + shipdx + 168, (bildy + shipdy + 98) % PBILDY, fuelstorage + 128, 4, 32);
}
