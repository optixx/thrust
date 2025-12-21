#ifndef ASSETS_H
#define ASSETS_H

#include <stdint.h>

#include "assets_state.h"

int assets_allocate(void);
void assets_cleanup(void);

uint8_t* assets_bulletmap(void);
uint8_t* assets_blocks(void);
uint8_t* assets_ship(void);
uint8_t* assets_shieldship(void);
uint8_t* assets_shipstorage(void);
uint8_t* assets_bulletstorage(void);
uint8_t* assets_fragmentstorage(void);
uint8_t* assets_fuelmap(void);
uint8_t* assets_fuelstorage(void);
uint8_t* assets_loadmap(void);
uint8_t* assets_loadstorage(void);
uint8_t* assets_wirestorage(void);

assets_state_t* assets_state(void);

#endif /* ASSETS_H */
