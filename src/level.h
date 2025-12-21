
/* Written by Peter Ekberg, peda@lysator.liu.se */

#ifndef LEVEL_H
#define LEVEL_H

int matchsliders(void);
int ismajorbutton(int tag);
void releasebana(void);
int readbana(char** ptr);
uint8_t* level_buffer(void);
void level_set_buffer(uint8_t* buffer);
void level_release_buffer(void);

#endif /* LEVEL_H */
