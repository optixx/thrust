#ifndef HUD_H
#define HUD_H

#include <stdint.h>

uint8_t nextmove(int reset);
void gamestatusframe(void);
void gamestatus(int lives, int fuel, int score);
void pause_message(void);
void escape_message(void);

#endif /* HUD_H */
