
/* Written by Peter Ekberg, peda@lysator.liu.se */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics.h"
#include "level.h"
#include "things.h"
#include "thrust.h"
#include "thrust_t.h"

static unsigned manhattan_distance(uint32_t ax, uint32_t ay, uint32_t bx, uint32_t by)
{
    unsigned dx = ax > bx ? ax - bx : bx - ax;
    unsigned dy = ay > by ? ay - by : by - ay;
    return dx + dy;
}

int matchsliders(void)
{
    uint32_t i, j;
    uint32_t match;
    int dist;

    for (i = 0; i < nrsliders; i++)
    {
        if (!sliders[i].match && sliders[i].dir)
        {
            match = nrsliders;
            dist = lenx;
            for (j = 0; j < nrsliders; j++)
            {
                if (!sliders[j].match && !sliders[j].dir &&
                    ((sliders[i].type - 1) / 3 == (sliders[j].type - 1) / 3))
                {
                    switch (sliders[i].dir)
                    {
                    case 1:
                        if (sliders[j].type != 12)
                        { /* Horizontal sliders */
                            if ((sliders[i].y1 == sliders[j].y1) &&
                                (sliders[j].x1 > sliders[i].x1) &&
                                (sliders[j].x1 - sliders[i].x1 < dist))
                            {
                                dist = sliders[j].x1 - sliders[i].x1;
                                match = j;
                            }
                        }
                        else /* Vertical sliders */
                            if ((sliders[i].x1 == sliders[j].x1) &&
                                (sliders[j].y1 > sliders[i].y1) &&
                                (sliders[j].y1 - sliders[i].y1 < dist))
                            {
                                dist = sliders[j].y1 - sliders[i].y1;
                                match = j;
                            }
                        break;
                    case 2:
                        if (sliders[j].type != 12)
                        { /* Horizontal sliders */
                            if ((sliders[i].y1 == sliders[j].y1) &&
                                (sliders[i].x1 > sliders[j].x1) &&
                                (sliders[i].x1 - sliders[j].x1 < dist))
                            {
                                dist = sliders[i].x1 - sliders[j].x1;
                                match = j;
                            }
                        }
                        else /* Vertical sliders */
                            if ((sliders[i].x1 == sliders[j].x1) &&
                                (sliders[i].y1 > sliders[j].y1) &&
                                (sliders[i].y1 - sliders[j].y1 < dist))
                            {
                                dist = sliders[i].y1 - sliders[j].y1;
                                match = j;
                            }
                        break;
                    }
                }
            }
            if (match == nrsliders)
            {
                printf("Found slider with no blocker.\n");
                return (0);
            }
            sliders[i].match = 1;
            sliders[match].match = 1;
            sliders[i].x2 = sliders[match].x1;
            sliders[i].y2 = sliders[match].y1;
            j = closestbutton((sliders[i].x1 + sliders[i].x2) << 2, (sliders[i].y1 + sliders[i].y2)
                                                                        << 2);
            if (j < 0)
            {
                printf("Found no botton to connect the slider with.\n");
                return (0);
            }
            j = majorbutton(j);
            sliders[i].next = ((buttondata*)things[j].data)->sliders;
            ((buttondata*)things[j].data)->sliders = &sliders[i];
        }
    }

    return (1);
}

int ismajorbutton(int tag)
{
    uint32_t i;

    for (i = 0; i < nrthings; i++)
        if ((things[i].type == 7 || things[i].type == 8) &&
            (((buttondata*)things[i].data)->tag == tag))
            return (i);
    return (-1);
}

void releasebana(void)
{
    uint32_t i;

    for (i = 0; i < nrthings; i++)
        if (things[i].data)
            free(things[i].data);
    nrthings = 0;
    nrsliders = 0;
    nrbarriers = 0;
    nrrestartpoints = 0;
}

int readbana(char** ptr)
/*
  Reads the level into the variable 'bana'.
  The file starts with eight values, i.e
  123
  69
  8
  10
  16
  R
  G
  B
  These eight numbers can be described as follows:
  The level is 123 blocks wide (lenx) and 69 blocks
  high (leny). It starts with 8 rows of stars which are
  followed by ten rows of empty space.
  The level is ended with 16 rows of solid bedrock.
  RGB is the color of the level.
  Below these 8 numbers is the definition of the level.
  This is what is drawn between the empty space and
  the bedrock.
*/
{
    int stat = 1;
    uint32_t t1, t2, x, y, z;
    uint32_t sx, sy, sz;
    char* temp;
    buttondata* bdata;

    releasebana();

    powerplant = 0;
    lenx = atoi(ptr[0]);
    if (lenx % BBILDX != 0)
        stat = 0;
    lenx3 = lenx << 3;
    leny = atoi(ptr[1]);
    sx = atoi(ptr[2]);
    for (sy = 0; sy < 3; sy++)
        if (sx < 2 * BBILDY)
        {
            sx += BBILDY - (leny % BBILDY);
            leny += BBILDY - (leny % BBILDY);
        }
    leny3 = leny << 3;
    if (lenx * (long)leny > (long)maxlenx * maxleny)
        stat = 0;
    sy = atoi(ptr[3]);
    sz = atoi(ptr[4]);
    x = y = lenx * (sx - 2 * BBILDY);
    colorr = atoi(ptr[5]);
    colorg = atoi(ptr[6]);
    colorb = atoi(ptr[7]);
    if (stat)
    {
        memset(bana, 32, lenx * (sx + sy));
        x >>= 6;
        for (z = 0; z < BBILDY; z++)
        {
            t1 = random() % (BBILDY * lenx);
            t2 = random() % 16;
            *(bana + t1) = t2;
            *(bana + t1 + BBILDY * lenx) = t2;
        }
        for (; x; x--)
            *(bana + 2 * BBILDY * lenx + (random() % y)) = random() % 16;
        for (y = sx + sy; y < leny - sz && stat; y++)
        {
            temp = ptr[y - sx - sy + 8];
            for (x = 0; x < lenx && stat; x++)
            {
                switch (temp[x])
                {
                case '#':
                    if (nrbarriers < maxbarriers)
                    {
                        barriers[nrbarriers].x = x;
                        barriers[nrbarriers++].y = y;
                    }
                    else
                        stat = 0;
                    break;
                case '*':
                    if (nrrestartpoints < maxrestartpoints)
                    {
                        restartpoints[nrrestartpoints].x = x;
                        restartpoints[nrrestartpoints++].y = y;
                    }
                    else
                        stat = 0;
                    break;
                case '@': /* Slide \block to the right */
                case 'A': /* Slide \block to the left */
                case 'B': /* Horizontal \blocker */
                case 'C': /* Slide /block to the right */
                case 'D': /* Slide /block to the left */
                case 'E': /* Horizontal /blocker */
                case 'F': /* Slide |block to the right */
                case 'G': /* Slide |block to the left */
                case 'H': /* Horizontal |blocker */
                case 'I': /* Slide -block downwards */
                case 'J': /* Slide -block upwards */
                case 'K': /* Vertical -blocker */
                    if (nrsliders < maxsliders)
                        newslider(x, y, temp[x] - '@' + 1);
                    else
                        stat = 0;
                    break;
                case 'L': /* Button on left wall */
                    if (nrthings < maxthings)
                    {
                        bdata = malloc(sizeof(buttondata));
                        if (!bdata)
                        {
                            printf("Out of memory.\n");
                            stat = 0;
                        }
                        else
                        {
                            bdata->sliders = NULL;
                            bdata->tag = temp[x - 1];
                            bdata->major = ismajorbutton(bdata->tag);
                            newthing((x << 3) + 9, (y << 3) + 8, x, y, 7, bdata);
                        }
                    }
                    else
                        stat = 0;
                    break;
                case 'N': /* Button on right wall */
                    if (nrthings < maxthings)
                    {
                        bdata = malloc(sizeof(buttondata));
                        if (!bdata)
                        {
                            printf("Out of memory.\n");
                            stat = 0;
                        }
                        else
                        {
                            bdata->sliders = NULL;
                            bdata->tag = temp[x - 1];
                            bdata->major = ismajorbutton(bdata->tag);
                            newthing((x << 3) - 1, (y << 3) + 8, x, y, 8, bdata);
                        }
                    }
                    else
                        stat = 0;
                    break;
                case 'P': /*  |\x Bunker */
                    if (nrthings < maxthings)
                        newthing((x << 3) + 14, (y << 3) + 8, x, y, 3, NULL);
                    else
                        stat = 0;
                    break;
                case 'U': /*  x/| Bunker */
                    if (nrthings < maxthings)
                        newthing((x << 3) + 10, (y << 3) + 8, x, y, 4, NULL);
                    else
                        stat = 0;
                    break;
                case '[': /*  |/x Bunker */
                    if (nrthings < maxthings)
                        newthing((x << 3) - 2, (y << 3) + 8, x - 2, y, 5, NULL);
                    else
                        stat = 0;
                    break;
                case '\\': /*  x\| Bunker */
                    if (nrthings < maxthings)
                        newthing((x << 3) + 10, (y << 3) + 7, x, y, 6, NULL);
                    else
                        stat = 0;
                    break;
                case '`': /* Fuel */
                    if (nrthings < maxthings)
                        newthing((x << 3) + 8, (y << 3) + 8, x, y, 1, NULL);
                    else
                        stat = 0;
                    break;
                case 'd': /* Power Plant */
                    if (nrthings < maxthings && !powerplant)
                    {
                        powerplant = 1;
                        ppblip = 1;
                        ppx = x + 2;
                        ppy = y;
                        newthing((x << 3) + 12, (y << 3) + 9, x, y, 2, NULL);
                    }
                    else
                        stat = 0;
                    break;
                case 'm':
                    loadbx = x;
                    loadby = y;
                    break;
                }
                if (!stat)
                    printf("Unable to create a thing.\n");
            }
            memcpy(bana + (long)y * lenx, temp, lenx);
        }
        memset(bana + (long)y * lenx, 112, lenx * sz);
    }

    if (stat)
    {
        for (x = 0; x < nrthings; x++)
        {
            if (things[x].type == 7)
                *(bana + (long)things[x].py * lenx + things[x].px - 1) = 112;
            if (things[x].type == 8)
                *(bana + (long)things[x].py * lenx + things[x].px - 1) = 32;
        }
        for (x = 0; x < nrsliders; x++)
            *(bana + (long)sliders[x].y1 * lenx + sliders[x].x1) = 112;
        if (!matchsliders())
        {
            printf("Unable to match all sliders.\n");
            stat = 0;
        }
        if (!nrrestartpoints)
        {
            printf("There must be at least one restartpoint.\n");
            stat = 0;
        }
        for (x = 0; x < nrbarriers; x++)
        {
            unsigned best_dist, candidate;

            barriers[x].restart = &restartpoints[0];
            best_dist = manhattan_distance(barriers[x].x, barriers[x].y, barriers[x].restart->x,
                                           barriers[x].restart->y);
            for (y = 1; y < nrrestartpoints; y++)
            {
                candidate = manhattan_distance(barriers[x].x, barriers[x].y, restartpoints[y].x,
                                               restartpoints[y].y);
                if (candidate < best_dist)
                {
                    best_dist = candidate;
                    barriers[x].restart = &restartpoints[y];
                }
            }
        }
    }

    return (stat);
}
