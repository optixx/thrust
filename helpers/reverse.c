
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifdef HAVE_CONFIG_H
#include "../src/config.h"
#endif

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
  long j;
  int i,r;
  int ch;
  FILE *si, *so;
  
  if(argc != 2) {
    fprintf(stderr, "%s: Usage %s [number]\n", argv[0], argv[0]);
    exit(1);
  }

  r=atoi(argv[1]);

  si = fdopen(fileno(stdin), "rb");
  so = fdopen(fileno(stdout), "wb");

  if(fseek(si, 0L, SEEK_END))
    perror(argv[0]);
  for(j=ftell(si)/r-1; j>=0; j--) {
    if(fseek(si, j*r, SEEK_SET))
      perror(argv[0]);
    for(i=0; i<r; i++) {
      ch=fgetc(si);
      fputc(ch, so);
    }
  }
  
  return(0);
}
