#ifndef ASSETS_STATE_H
#define ASSETS_STATE_H

#include <stdint.h>

typedef struct
{
    uint8_t* bulletmap;
    uint8_t* blocks;
    uint8_t* ship;
    uint8_t* shieldship;
    uint8_t* shipstorage;
    uint8_t* bulletstorage;
    uint8_t* fragmentstorage;
    uint8_t* fuelmap;
    uint8_t* fuelstorage;
    uint8_t* loadmap;
    uint8_t* loadstorage;
    uint8_t* wirestorage;
} assets_state_t;

assets_state_t* assets_state(void);

#endif /* ASSETS_STATE_H */
