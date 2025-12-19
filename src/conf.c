
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "thrust_t.h"
#include "keyboard.h"
#include "conf.h"
#include "font5x5.h"
#include "thrust.h"
#include "graphics.h"

int
getscancode(int old, int x, int y)
{
  int result;
  static char str[40];
  int oldcolor;

  oldcolor=chcolor;
  sprintf(str, "%s", keystring(old));
  printgs(x, y, str);

  displayscreen();
  result = getonemultiplekey();
  if(!strcasecmp("Escape", keystring(result)))
    result = old;
  chcolor=0;
  printgs(x, y, str);
  chcolor=oldcolor;
  sprintf(str, "%s", keystring(result));
  printgs(x, y, str);

  return(result);
}

#define DESCRIPTIONS (5)

void
conf(void)
{
  static char *descriptions[DESCRIPTIONS] = {
    "Turn left",
    "Turn right",
    "Thrust",
    "Fire",
    "Pickup & Shield"
  };
  int i, end;
  char *keyname;

  chcolor = HIGHLIGHT;
  gcenter(45, "Configuration");
  chcolor = TEXTCOLOR;
  for(i=0; i<DESCRIPTIONS; i++) {
    printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);
    printgs(161, 65+i*8, ":");
    printgs(167, 65+i*8, keystring(scancode[i]));
  }
  gcenter(125, "Press Enter to change a key");
  gcenter(132, "(don't use Q or P, these are taken)");
  gcenter(139, "Use Cursor Up/Down to move around");
  gcenter(148, "Press Escape for the main menu");

  i=0;
  end=0;
  chcolor = HIGHLIGHT;
  printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);
  chcolor = TEXTCOLOR;

  fade_in();

  do {
    keyname = keystring(getonemultiplekey());

    if(!strcasecmp("Cursor Up", keyname)
       || !strcasecmp("Keypad Up", keyname)
       || !strcasecmp("Up", keyname)
       || !strcasecmp("KP Up", keyname)) {
      printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);
      i=(i+DESCRIPTIONS-1)%DESCRIPTIONS;
    }

    if(!strcasecmp("Cursor Down", keyname)
       || !strcasecmp("Keypad Down", keyname)
       || !strcasecmp("Down", keyname)
       || !strcasecmp("KP Down", keyname)) {
      printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);
      i=(i+1)%DESCRIPTIONS;
    }

    if(!strcasecmp("Enter", keyname)
       || !strcasecmp("Keypad Enter", keyname)
       || !strcasecmp("KP Enter", keyname)
       || !strcasecmp("Return", keyname)) {
      printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);

      chcolor = HIGHLIGHT;
      scancode[i]=getscancode(scancode[i], 167, 65+i*8);
      printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);
      chcolor = TEXTCOLOR;

      printgs(167, 65+i*8, keystring(scancode[i]));
      displayscreen();
    }

    if(!strcasecmp("Escape", keyname)
       || !strcasecmp("Q", keyname)) {
      end=1;
    }

    chcolor = HIGHLIGHT;
    printgs(160-gstrlen(descriptions[i]), 65+i*8, descriptions[i]);
    chcolor = TEXTCOLOR;

    displayscreen();
  } while(!end);
  
  chcolor = TEXTCOLOR;
  
  fade_out();
}

void initkeys(void)
{
  char *home;
  char *thrustrc;
  char *keyboarddriver;
  FILE *f;
  int rows=0;
  int res;
  char row[256], field[256], value[256], driver[256];

  home=getenv("HOME");
  if(home==NULL)
    home = "";

  thrustrc = malloc(strlen(home) + 11);
  if(thrustrc == NULL) {
    printf("Out of memory when trying to read .thrustrc.\n");
    return;
  }

  strcpy(thrustrc, home);
#if defined(__DJGPP__) || defined(_WIN32)
  if(thrustrc[0]) {
    unsigned int i;

    for(i=0; i<strlen(thrustrc); i++)
      thrustrc[i] = thrustrc[i]=='/' ? '\\' : thrustrc[i];
    if(thrustrc[strlen(thrustrc)-1]!='\\')
      strcat(thrustrc, "\\");
  }
  strcat(thrustrc, "thrustrc");
#else
  if(thrustrc[0])
    if(thrustrc[strlen(thrustrc)-1]!='/')
      strcat(thrustrc, "/");
  strcat(thrustrc, ".thrustrc");
#endif


  f = fopen(thrustrc, "r");
  if(f == NULL) {
#if defined(_WIN32)
    strcpy(thurstrc, "thrustrc");
    f = fopen(thrustrc, "r");
    if(f == NULL)
      return;
#else
    return;
#endif
  }

  keyboarddriver = keyname();

  while(!feof(f)) {
    rows++;
    if(fgets(row, 255, f) == NULL) {
      if(ferror(f)) {
	perror("Error while parsing row %d of \"%s\"\n");
	break;
      }
      else
	row[0] = '\0';
    }
    if(!feof(f)) {
      if(row[strlen(row)-1] != '\n') {
	printf("Row %d of \"%s\" is too long.\n", rows, thrustrc);
	break;
      }
      else
	row[strlen(row)-1] = '\0';
    }
    if(row[0] != '#') {
      res = sscanf(row, "%[^-]-%s %s", driver, field, value);
      if(res==2)
	printf("Syntax error in row %d of \"%s\".\n", rows, thrustrc);
      else if(res>=3) {
	if(!strcasecmp(driver, "SVGA")
	   || !strcasecmp(driver, "X11")
	   || !strcasecmp(driver, "Win32")) {
	  if(!strcasecmp(driver, keyboarddriver)) {
	    if(!strcasecmp(field, "counterclockwise")) {
	      scancode[0] = keycode(value);
	    }
	    else if(!strcasecmp(field, "clockwise")) {
	      scancode[1] = keycode(value);
	    }
	    else if(!strcasecmp(field, "thrust")) {
	      scancode[2] = keycode(value);
	    }
	    else if(!strcasecmp(field, "fire")) {
	      scancode[3] = keycode(value);
	    }
	    else if(!strcasecmp(field, "pickup")) {
	      scancode[4] = keycode(value);
	    }
	    else {
	      printf("Illegal keyboard field \"%s\" specified in row %d.\n",
		     field, rows);
	    }
	  }
	}
	else {
	  printf("Illegal keyboard driver \"%s\" specified in row %d.\n",
		 driver, rows);
	}
      }
    }
  }

  fclose(f);
}
