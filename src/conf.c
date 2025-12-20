
/* Written by Peter Ekberg, peda@lysator.liu.se */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "conf.h"
#include "font5x5.h"
#include "graphics.h"
#include "keyboard.h"
#include "thrust.h"
#include "helpers.h"

int getscancode(int old, int x, int y)
{
    int result;
    static char str[40];
    int oldcolor;

    oldcolor = chcolor;
    sprintf(str, "%s", keystring(old));
    printgs(x, y, str);

    displayscreen();
    result = wait_for_key();
    if (!strcasecmp("Escape", keystring(result)))
        result = old;
    chcolor = 0;
    printgs(x, y, str);
    chcolor = oldcolor;
    sprintf(str, "%s", keystring(result));
    printgs(x, y, str);

    return (result);
}

#define DESCRIPTIONS (5)

void conf(void)
{
    static char* descriptions[DESCRIPTIONS] = {"Turn left", "Turn right", "Thrust", "Fire",
                                               "Pickup & Shield"};
    int i, end;
    char* keyname;

    chcolor = HIGHLIGHT;
    gcenter(45, "Configuration");
    chcolor = TEXTCOLOR;
    for (i = 0; i < DESCRIPTIONS; i++)
    {
        printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);
        printgs(161, 65 + i * 8, ":");
        printgs(167, 65 + i * 8, keystring(scancode[i]));
    }
    gcenter(125, "Press Enter to change a key");
    gcenter(132, "(don't use Q or P, these are taken)");
    gcenter(139, "Use Cursor Up/Down to move around");
    gcenter(148, "Press Escape for the main menu");

    i = 0;
    end = 0;
    chcolor = HIGHLIGHT;
    printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);
    chcolor = TEXTCOLOR;

    fade_in();

    do
    {
        keyname = keystring(wait_for_key());

        if (!strcasecmp("Cursor Up", keyname) || !strcasecmp("Keypad Up", keyname) ||
            !strcasecmp("Up", keyname) || !strcasecmp("KP Up", keyname))
        {
            printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);
            i = (i + DESCRIPTIONS - 1) % DESCRIPTIONS;
        }

        if (!strcasecmp("Cursor Down", keyname) || !strcasecmp("Keypad Down", keyname) ||
            !strcasecmp("Down", keyname) || !strcasecmp("KP Down", keyname))
        {
            printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);
            i = (i + 1) % DESCRIPTIONS;
        }

        if (!strcasecmp("Enter", keyname) || !strcasecmp("Keypad Enter", keyname) ||
            !strcasecmp("KP Enter", keyname) || !strcasecmp("Return", keyname))
        {
            printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);

            chcolor = HIGHLIGHT;
            scancode[i] = getscancode(scancode[i], 167, 65 + i * 8);
            printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);
            chcolor = TEXTCOLOR;

            printgs(167, 65 + i * 8, keystring(scancode[i]));
            displayscreen();
        }

        if (!strcasecmp("Escape", keyname) || !strcasecmp("Q", keyname))
        {
            end = 1;
        }

        chcolor = HIGHLIGHT;
        printgs(160 - gstrlen(descriptions[i]), 65 + i * 8, descriptions[i]);
        chcolor = TEXTCOLOR;

        displayscreen();
    } while (!end);

    chcolor = TEXTCOLOR;

    fade_out();
}

void initkeys(void)
{
    char path[PATH_MAX];
    char* keyboarddriver;
    FILE* f;
    int rows = 0;
    int res;
    char row[256], field[256], value[256], driver[256];

    snprintf(path, sizeof path, "thrust.rc");

    f = fopen(path, "r");
    if (f == NULL)
    {
        perror("Opening configuration file");
        return;
    }

    keyboarddriver = keyname();
    printf("Loading configuration from %s (keyboard %s)\n", path, keyboarddriver);

    while (fgets(row, sizeof row, f))
    {
        char* line = row;
        size_t len = strlen(line);

        rows++;
        if (len && line[len - 1] == '\n')
            line[--len] = '\0';

        while (*line && isspace((unsigned char)*line))
            line++;
        if (*line == '\0' || *line == '#')
            continue;

        res = sscanf(line, "%63[^-]-%63s %63s", driver, field, value);
        if (res < 3)
        {
            printf("Syntax error in row %d of \"%s\".\n", rows, path);
            continue;
        }

        if (strcasecmp(driver, keyboarddriver) && strcasecmp(driver, "SDL"))
            continue;

        int idx = -1;
        if (!strcasecmp(field, "counterclockwise"))
        {
            scancode[0] = keycode(value);
            idx = 0;
        }
        else if (!strcasecmp(field, "clockwise"))
        {
            scancode[1] = keycode(value);
            idx = 1;
        }
        else if (!strcasecmp(field, "thrust"))
        {
            scancode[2] = keycode(value);
            idx = 2;
        }
        else if (!strcasecmp(field, "fire"))
        {
            scancode[3] = keycode(value);
            idx = 3;
        }
        else if (!strcasecmp(field, "pickup"))
        {
            scancode[4] = keycode(value);
            idx = 4;
        }
        else
        {
            printf("Illegal keyboard field \"%s\" specified in row %d.\n", field, rows);
        }
        if (idx >= 0)
            printf("  %s -> %s\n", field, keystring(scancode[idx]));
    }

    fclose(f);
}
