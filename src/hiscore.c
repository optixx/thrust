
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "thrust_t.h"
#include "hiscore.h"

highscoreentry highscorelist[HIGHSCORES];

static const char default_name[] = "John Doe";

static int
write_le32(FILE *fp, int score)
{
  int i;

  for(i = 0; i < 4; i++) {
    if(fputc((score >> (8 * i)) & 0xff, fp) == EOF)
      return 0;
  }
  return 1;
}

static int
read_le32(FILE *fp, int *out)
{
  int i;
  int value = 0;
  int byte;

  for(i = 0; i < 4; i++) {
    byte = fgetc(fp);
    if(byte == EOF)
      return 0;
    value |= byte << (8 * i);
  }

  *out = value;
  return 1;
}

static void
sanitize_name(char *dest, const char *src)
{
  if(src == NULL) {
    dest[0] = '\0';
    return;
  }

  size_t len = strnlen(src, sizeof highscorelist[0].name - 1);
  memcpy(dest, src, len);
  dest[len] = '\0';
}

void
writehighscores(void)
{
  FILE *fp;
  int i;

  fp=fopen(HIGHSCOREFILE, "wb");
  if(fp==NULL)
    return;
  for(i=0; i<HIGHSCORES; i++) {
    if(fwrite(highscorelist[i].name, sizeof highscorelist[i].name, 1, fp) != 1
       || !write_le32(fp, highscorelist[i].score))
      break;
  }
  fclose(fp);
}

int
readhighscores(void)
{
  FILE *fp;
  int res;
  int i;
  fp=fopen(HIGHSCOREFILE, "rb");
  if(fp==NULL)
    return(0);

  for(i=0; i<HIGHSCORES; i++) {
    res = fread(highscorelist[i].name, sizeof highscorelist[i].name, 1, fp);
    if(res != 1 || !read_le32(fp, &highscorelist[i].score)) {
      fclose(fp);
      return(0);
    }
  }

  fclose(fp);
  return(1);
}

char *
standardname(void)
{
  char *tmp;
  static char name[40];

  tmp=getenv("USER");
  if(tmp==NULL)
    tmp=getenv("LOGNAME");
  if(tmp==NULL)
    name[0]=0;
  else {
    sanitize_name(name, tmp);
    name[0]=toupper((unsigned char)name[0]);
  }

  return(name);
}

int
inithighscorelist(void)
{
  int i;
  
  if(!readhighscores()) {
    for(i=0; i<HIGHSCORES; i++) {
      sanitize_name(highscorelist[i].name, default_name);
      highscorelist[i].score=(5-i)*1000;
    }
  }
  
  return(0);
}

int
ahighscore(int score)
{
  return(score>highscorelist[HIGHSCORES-1].score);
}

void
inserthighscore(char *name, int score)
{
  int i;

  for(i=HIGHSCORES; i>0 && score>highscorelist[i-1].score; i--) {
    if(i<HIGHSCORES) {
      highscorelist[i] = highscorelist[i-1];
    }
  }
  if(i<HIGHSCORES) {
    sanitize_name(highscorelist[i].name, name);
    highscorelist[i].score = score;
  }
}
